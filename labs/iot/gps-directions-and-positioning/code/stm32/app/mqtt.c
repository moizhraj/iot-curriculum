/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "mqtt.h"
#include "math.h"

#include <stdio.h>

#include "stm32l475e_iot01.h"
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_magneto.h"

#include "azure_iot_mqtt.h"
#include "json_utils.h"
#include "sntp_client.h"

#include "azure_config.h"
#include "gps.h"

#define IOT_MODEL_ID "dtmi:com:examples:gsg;1"

#define TELEMETRY_INTERVAL_PROPERTY "telemetryInterval"
#define LED_STATE_PROPERTY          "ledState"

#define M_PI 3.14159265358979323846

#define TELEMETRY_INTERVAL_EVENT 10

static AZURE_IOT_MQTT azure_iot_mqtt;
static TX_EVENT_FLAGS_GROUP azure_iot_flags;

static INT telemetry_interval = 1;

static void set_led_state(bool level)
{
    if (level)
    {
        printf("LED is turned ON\r\n");
        BSP_LED_On(LED_GREEN);
    }
    else
    {
        printf("LED is turned OFF\r\n");
        BSP_LED_Off(LED_GREEN);
    }
}

static void mqtt_direct_method(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* direct_method_name, CHAR* message)
{
    if (strcmp(direct_method_name, "setLedState") == 0)
    {
        printf("Direct method=%s invoked\r\n", direct_method_name);

        // 'false' - turn LED off
        // 'true'  - turn LED on
        bool arg = (strcmp(message, "true") == 0);

        set_led_state(arg);

        // Return success
        azure_iot_mqtt_respond_direct_method(azure_iot_mqtt, 200);

        // Update device twin property
        azure_iot_mqtt_publish_bool_property(azure_iot_mqtt, LED_STATE_PROPERTY, arg);
    }
    else
    {
        printf("Received direct method=%s is unknown\r\n", direct_method_name);
        azure_iot_mqtt_respond_direct_method(azure_iot_mqtt, 501);
    }
}

static void mqtt_c2d_message(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* properties, CHAR* message)
{
    printf("Received C2D message, properties='%s', message='%s'\r\n", properties, message);
}

static void mqtt_device_twin_desired_prop(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* message)
{
    printf("Device Twin Desired Message: %s\n", message);
    jsmn_parser parser;
    jsmntok_t tokens[64];
    INT token_count;

    jsmn_init(&parser);
    token_count = jsmn_parse(&parser, message, strlen(message), tokens, 64);

    if (findJsonInt(message, tokens, token_count, TELEMETRY_INTERVAL_PROPERTY, &telemetry_interval))
    {
        // Set a telemetry event so we pick up the change immediately
        tx_event_flags_set(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR);

        // Confirm reception back to hub
        azure_iot_mqtt_respond_int_writeable_property(
            azure_iot_mqtt, TELEMETRY_INTERVAL_PROPERTY, telemetry_interval, 200);
    }
}

static void mqtt_device_twin_prop(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* message)
{
    printf("Device Twin Message: %s\n", message);
    jsmn_parser parser;
    jsmntok_t tokens[64];
    INT token_count;

    jsmn_init(&parser);
    token_count = jsmn_parse(&parser, message, strlen(message), tokens, 64);

    if (findJsonInt(message, tokens, token_count, TELEMETRY_INTERVAL_PROPERTY, &telemetry_interval))
    {
        // Set a telemetry event so we pick up the change immediately
        tx_event_flags_set(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR);
    }

    // Report writeable properties to the Hub
    azure_iot_mqtt_publish_int_writeable_property(azure_iot_mqtt, TELEMETRY_INTERVAL_PROPERTY, telemetry_interval);
}

static void calibrate_magnetometer(int16_t *offsetX, int16_t *offsetY, int16_t *offsetZ) {

    int16_t magnetoXYZ[3] = {0};
    
    BSP_MAGNETO_GetXYZ(magnetoXYZ);
    int16_t minMagX = magnetoXYZ[0], maxMagX = magnetoXYZ[0], minMagY = magnetoXYZ[1], maxMagY = magnetoXYZ[1], minMagZ = magnetoXYZ[2], maxMagZ = magnetoXYZ[2];
    int counter = 0;

    printf("Starting magnetometer calibration, start rotating device 360% in all dimensions (3D). \n");
    HAL_Delay(2000);
    while(counter++ < 3000) {
        BSP_MAGNETO_GetXYZ(magnetoXYZ);
        if(magnetoXYZ[0] < minMagX) minMagX = magnetoXYZ[0];
        if(magnetoXYZ[0] > maxMagX) maxMagX = magnetoXYZ[0];
        if(magnetoXYZ[1] < minMagY) minMagY = magnetoXYZ[1];
        if(magnetoXYZ[1] > maxMagY) maxMagY = magnetoXYZ[1];
        if(magnetoXYZ[2] < minMagZ) minMagZ = magnetoXYZ[2];
        if(magnetoXYZ[2] > maxMagZ) maxMagZ = magnetoXYZ[2];
        if(counter % 100 == 0) {
            printf("Magnetometer Calibrated minX: %d, maxX: %d, minY: %d, maxY: %d, minZ: %d, maxZ: %d\n", minMagX, maxMagX, minMagY, maxMagY, minMagZ, maxMagZ);
        }
        HAL_Delay(10);
    }
    printf("Completed calibration of magnetometer.\n");

    HAL_Delay(2000);
    *offsetX = (minMagX + maxMagX)/2;
    *offsetY = (minMagY + maxMagY)/2;
    *offsetZ = (minMagZ + maxMagZ)/2;
    printf("Magnetometer Calibration: Offset x: %d, y: %d, z: %d\n", *offsetX, *offsetY, *offsetZ);
}

UINT azure_iot_mqtt_entry(NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, ULONG (*sntp_time_get)(VOID))
{
    UINT status;
    ULONG events;
    float temperature;
    float humidity;
    float pressure;
    int16_t acceleroXYZ[3] = {0};
    float gyroXYZ[3] = {0};
    int16_t magnetoXYZ[3] = {0};

    if ((status = tx_event_flags_create(&azure_iot_flags, "Azure IoT flags")))
    {
        printf("FAIL: Unable to create nx_client event flags (0x%02x)\r\n", status);
        return status;
    }

#ifdef ENABLE_DPS
    // Create Azure MQTT for Hub via DPS
    status = azure_iot_mqtt_create_with_dps(&azure_iot_mqtt,
        ip_ptr,
        pool_ptr,
        dns_ptr,
        sntp_time_get,
        IOT_DPS_ENDPOINT,
        IOT_DPS_ID_SCOPE,
        IOT_DEVICE_ID,
        IOT_PRIMARY_KEY,
        IOT_MODEL_ID);
#else
    // Create Azure MQTT for Hub
    status = azure_iot_mqtt_create(&azure_iot_mqtt,
        ip_ptr,
        pool_ptr,
        dns_ptr,
        sntp_time_get,
        IOT_HUB_HOSTNAME,
        IOT_DEVICE_ID,
        IOT_PRIMARY_KEY,
        IOT_MODEL_ID);
#endif

    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error: Failed to create Azure IoT MQTT (0x%04x)\r\n", status);
        return status;
    }

    // Register callbacks
    azure_iot_mqtt_register_direct_method_callback(&azure_iot_mqtt, mqtt_direct_method);
    azure_iot_mqtt_register_c2d_message_callback(&azure_iot_mqtt, mqtt_c2d_message);
    azure_iot_mqtt_register_device_twin_desired_prop_callback(&azure_iot_mqtt, mqtt_device_twin_desired_prop);
    azure_iot_mqtt_register_device_twin_prop_callback(&azure_iot_mqtt, mqtt_device_twin_prop);

    // Connect the Azure MQTT client
    status = azure_iot_mqtt_connect(&azure_iot_mqtt);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error: Failed to create Azure MQTT (0x%02x)\r\n", status);
        return status;
    }

    // Update ledState property
    azure_iot_mqtt_publish_bool_property(&azure_iot_mqtt, LED_STATE_PROPERTY, false);

    // Request the device twin
    azure_iot_mqtt_device_twin_request(&azure_iot_mqtt);

    // Initialize RX/TX
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_UART4_Init();
    printf("UART Initialized over DMA\r\n");

    int16_t offsetX, offsetY, offsetZ;
    calibrate_magnetometer(&offsetX, &offsetY, &offsetZ);

    printf("Starting MQTT loop\r\n");
    while (true)
    {
        BSP_LED_On(1);
        temperature = BSP_TSENSOR_ReadTemp();
        humidity = BSP_HSENSOR_ReadHumidity();
        pressure = BSP_PSENSOR_ReadPressure();
        BSP_ACCELERO_AccGetXYZ(acceleroXYZ);
        BSP_GYRO_GetXYZ(gyroXYZ);
        BSP_MAGNETO_GetXYZ(magnetoXYZ);
        BSP_LED_Off(0);
        //char *location = GET_GEO_LOCATION();

        //Calculate calibrated magnatometer x,y,z based on offset 
        int16_t c_MagX, c_MagY, c_MagZ;
        c_MagX = magnetoXYZ[0] - offsetX;
        c_MagY = magnetoXYZ[1] - offsetY;
        c_MagZ = magnetoXYZ[2] - offsetZ;
        float heading = atan2(c_MagY, c_MagX) * 180/M_PI;

        // Send the temperature as a telemetry event
        // azure_iot_mqtt_publish_float_telemetry(&azure_iot_mqtt, "temperature", temperature);

        CHAR mqtt_message[250];
        snprintf(mqtt_message, sizeof(mqtt_message), 
                    "{\"temperature\":%3.2f, \"humidity\":%3.2f, \"pressure\":%3.2f, \"accelX\":%d, \"accelY\":%d, \"accelZ\":%d, \"gyroX\":%3.2f, \"gyroY\":%3.2f, \"gyroZ\":%3.2f, \"magX\":%d, \"magY\":%d, \"magZ\":%d, \"cmagX\":%d, \"cmagY\":%d, \"cmagZ\":%d, \"heading\":\"%3.2f\"}", 
                    temperature, humidity, pressure, 
                    acceleroXYZ[0], acceleroXYZ[1], acceleroXYZ[2], 
                    gyroXYZ[0], gyroXYZ[1], gyroXYZ[2],
                    magnetoXYZ[0], magnetoXYZ[1], magnetoXYZ[2],
                    c_MagX, c_MagY, c_MagZ, heading);
                    //location);
        //free(location);
        printf("%s\n", mqtt_message);
        //azure_iot_mqtt_publish_string_telemetry(&azure_iot_mqtt, "body", mqtt_message);

        // Sleep
        tx_event_flags_get(
            &azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR_CLEAR, &events, telemetry_interval * NX_IP_PERIODIC_RATE);
    }

    return NXD_MQTT_SUCCESS;
}
