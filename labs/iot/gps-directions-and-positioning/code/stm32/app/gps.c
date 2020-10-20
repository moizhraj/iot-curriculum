#include <stdio.h>
#include <stdbool.h>
#include "gps.h"

#include "stm32l4xx_hal.h"
#include "tx_api.h"

UART_HandleTypeDef huart4;
DMA_HandleTypeDef hdma_uart4_rx;
DMA_HandleTypeDef hdma_uart4_tx;
uint8_t flag = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart4);
int nmea0183_checksum(char *msg);

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

/**
  * Enable DMA controller clock
  */
void MX_DMA_Init(void) {
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel3_IRQn);
  /* DMA2_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel5_IRQn);
}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
void MX_UART4_Init(void) {
    huart4.Instance = UART4;
    huart4.Init.BaudRate = 9600;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;
    // huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    // huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_TXINVERT_INIT|UART_ADVFEATURE_RXINVERT_INIT;
    // huart4.AdvancedInit.TxPinLevelInvert = UART_ADVFEATURE_TXINV_ENABLE;
    // huart4.AdvancedInit.RxPinLevelInvert = UART_ADVFEATURE_RXINV_ENABLE;
    if (HAL_UART_Init(&huart4) != HAL_OK)
    {
        // Error_Handler();
    }
}

/**
  * @brief Get Geo location
  * @param huart4
  * @retval None
  */
//void GET_GEO_LOCATION(char **result, size_t *result_length) {
char * GET_GEO_LOCATION() {
    char *res = malloc(11);
    //char res[13];
    bool location_locked = false;
    INT loopCount = 100;
    uint8_t buff[255];
    char buffStr[255];
    char nmeaSnt[80];

    char *rawSum;
    char smNmbr[3];

    // The Equator has a latitude of 0°,
    //the North Pole has a latitude of 90° North (written 90° N or +90°),
    //and the South Pole has a latitude of 90° South (written 90° S or −90°)
    char *latRaw;
    char latDg[2];
    char latMS[7];
    char *hemNS;

    // longitude in degrees (0° at the Prime Meridian to +180° eastward and −180° westward)
    // that is why 3
    char *lonRaw;
    char lonDg[3];
    char lonMS[7];
    char *hemEW;

    char *utcRaw; // raw UTC time from the NMEA sentence in the hhmmss format
    char strUTC[8]; // UTC time in the readable hh:mm:ss format

    char hH[2]; // hours
    char mM[2]; // minutes
    char sS[2]; // seconds

    uint8_t cnt = 0;

    while(!location_locked || --loopCount > 0) {
        HAL_UART_Receive_DMA(&huart4, buff, 255);
        /*
            $ - Start delimiter
            * - Checksum delimiter
            , - Field delimiter

            1. $GNGLL log header
            2. Latitude (Ddmm.mm) [The Equator has a latitude of 0°, the North Pole has a latitude of 90° North (written 90° N or +90°)]
            3. Latitude direction (N = North, S = South)
            4. Longitude (DDDmm.mm) [0° at the Prime Meridian to +180° eastward and −180° westward]
            5. Longitude direction (E = East, W = West)
            6. UTC time status of position (hours/minutes/seconds/decimal seconds) hhmmss
            7. Data status: A = Data valid, V = Data invalid
            8. Positioning system mode indicator
            9. *xx Checksum
            10. [CR][LF] Sentence terminator. In C \r\n (two characters).
            or \r Carriage return
            or \n Line feed, end delimiter

        */
        if(flag == 1) { // interrupt signals that the buffer buff[255] is full
            flag = 0;
            memset(buffStr, 0, 255);
            sprintf(buffStr, "%s", buff);
            char *token, *string;
            string = strdup(buffStr);
            while ((token = strsep(&string, "\n")) != NULL) {
                memset(nmeaSnt, 0, 80);
                sprintf(nmeaSnt, "%s", token);
                //printf("%s", nmeaSnt);
                // if ((strstr(nmeaSnt, "$GPGLL") != 0)) {
                //     printf("----- 1st: %d. 2nd: %d, 3rd: %d -----\r\n", (strstr(nmeaSnt, "$GPGLL") != 0), (strlen(nmeaSnt) > 25), (strstr(nmeaSnt, "*") != 0));
                // }
                if ((strstr(nmeaSnt, "$GPGLL") != 0) && strlen(nmeaSnt) > 25 && strstr(nmeaSnt, "*") != 0) {
                    rawSum = strstr(nmeaSnt, "*");
                    memcpy(smNmbr, &rawSum[1], 2);
                    smNmbr[2] = '\0';

                    uint8_t intSum = nmea0183_checksum(nmeaSnt);

                    char hex[2];

                    // "%X" unsigned hexadecimal integer (capital letters)
                    sprintf(hex, "%X", intSum);
                    if (strstr(smNmbr, hex) != NULL) {
                        cnt = 0;
                        for (char *pV = strtok(nmeaSnt, ","); pV != NULL; pV = strtok(NULL, ",")) {
                            switch (cnt) {
                            case 1:
                                latRaw = strdup(pV);
                                break;
                            case 2:
                                hemNS = strdup(pV);
                                break;
                            case 3:
                                lonRaw = strdup(pV);
                                break;
                            case 4:
                                hemEW = strdup(pV);
                                break;
                            case 5:
                                utcRaw = strdup(pV);
                                break;
                            }
                            cnt++;
                        }
                        memcpy(latDg, &latRaw[0], 2);
                        latDg[2] = '\0';
                        
                        memcpy(latMS, &latRaw[2], 7);
                        latMS[7] = '\0';

                        memcpy(lonDg, &lonRaw[0], 3);
                        lonDg[3] = '\0';

                        memcpy(lonMS, &lonRaw[3], 7);
                        lonMS[7] = '\0';
                        char strLonMS[7];
                        sprintf(strLonMS, "%s", lonMS);

                        //converting the UTC time in the hh:mm:ss format
                        memcpy(hH, &utcRaw[0], 2);
                        hH[2] = '\0';

                        memcpy(mM, &utcRaw[2], 2);
                        mM[2] = '\0';

                        memcpy(sS, &utcRaw[4], 2);
                        sS[2] = '\0';

                        strcpy(strUTC, hH);
                        strcat(strUTC, ":");
                        strcat(strUTC, mM);
                        strcat(strUTC, ":");
                        strcat(strUTC, sS);
                        strUTC[8] = '\0';

                        strcpy(res, hemNS);
                        strcat(res, " ");
                        strcat(res, latDg);
                        strcat(res, ", ");
                        strcat(res, hemEW);
                        strcat(res, " ");
                        strcat(res, lonDg);
                        //strcat(res, "\r\n");

                        //printf("Result %s", res);
                        //printf("%s %s%c%s\', %s %s%c%s\', UTC: %s\r\n", hemNS, latDg, 241, latMS, hemEW, lonDg, 241, lonMS, strUTC);
                        location_locked = true;
                    }
                }
            }
            memset(buff, 0, sizeof(buff));
        }
    }
    HAL_UART_DMAStop(&huart4);
    return res;
}

/**
  * @brief DMA Interrupt callback
  * @param huart4
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart4) {
    flag = 1;
}

/**
  * @brief Calculate checksum of the GPS NMEA sentence
  * -4, but not -3 because the NMEA sentences are delimited with \r\n, and there also is the invisible \r in the end
  * @param msg
  * @retval int
  */
int nmea0183_checksum(char *msg) {
    int checksum = 0;
    int j = 0;
    // the first $ sign and the last two bytes of original CRC + the * sign
    for (j = 1; j < strlen(msg) - 4; j++) {
        checksum = checksum ^ (unsigned) msg[j];
    }
    return checksum;
}