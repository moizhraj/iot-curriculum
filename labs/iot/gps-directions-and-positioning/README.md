# GPS Directions and Positioning Lab

This folder contains a lab with multiple parts showing GPS Direction and Positioning using an [STM32L475E-IOT01A](https://www.st.com/en/evaluation-tools/b-l475e-iot01a.html) and a [Neo 6M GPS](https://www.u-blox.com/en/product/neo-6-series), Azure IoT Services, and Azure Maps. It is designed for beginners who are new to embedded programming, IoT and Azure.

| Author | [Moiz Malegaonwala](https://github.com/moizhraj), [Tanmoy Rajguru](https://github.com/Tanmoy-TCS) |
|:---|:---|
| Team | [Microsoft AI & IoT Insiders Lab](https://microsoftiotinsiderlabs.com/en) |
| Target platform   | [STM32L475E-IOT01A](https://www.st.com/en/evaluation-tools/b-l475e-iot01a.html) |
| Hardware required | If you are running this on a STM32:<ul><li>[STM32L475E-IOT01A](https://www.st.com/en/evaluation-tools/b-l475e-iot01a.html)</li><li>Micro USB cable</li><li>[ublox Neo 6M GPS](https://www.u-blox.com/en/product/neo-6-series) sensor</li><li>Breadboard</li><li>Jumper Wires</li></ul> |
| Software required | <ul><li>[Visual Studio Code](http://code.visualstudio.com?WT.mc_id=iotcurriculum-github-jabenn)</li></ul>*There are other installs for Windows and Linux that you may need to install later to connect to the STM32, depending on which version of the OS you are using.* |
| Azure Services | <ul><li>[Azure IoT Hub](https://azure.microsoft.com/services/iot-hub/?WT.mc_id=iotcurriculum-github-jabenn)</li><li>[Azure Event Hubs](https://azure.microsoft.com/services/event-hubs/?WT.mc_id=iotcurriculum-github-jabenn)</li><li>[Azure Maps](https://azure.microsoft.com/services/azure-maps/?WT.mc_id=iotcurriculum-github-jabenn)</li></ul> |
| Programming Language | <ul><li>C</li><li>Python</li></ul> |
| Prerequisites | You will need to be reasonably proficient at using a computer, including installing software and running commands from a command line or terminal.<br>These labs will use C and Python, but if you are not proficient in these languages you can simply copy and paste the provided code and run it to see the end results, rather than trying to understand the code.<br><br>If you want to learn C and Python, check out these free resources:<br><ul><li>[Python for beginners video series on Channel9](https://channel9.msdn.com/Series/Intro-to-Python-Development?WT.mc_id=iotcurriculum-github-jabenn)</li><li>[Take your first steps with Python learning path on Microsoft Learn](https://docs.microsoft.com/learn/paths/python-first-steps/?WT.mc_id=iotcurriculum-github-jabenn)</li></ul><br>You will also need an [Azure subscription](https://azure.microsoft.com/free/?WT.mc_id=iotcurriculum-github-jabenn)<br>If you are new to Azure, check out these free resources:<ul><li>[Azure Fundamentals learning path on Microsoft Learn](https://docs.microsoft.com/learn/paths/azure-fundamentals/?WT.mc_id=iotcurriculum-github-jabenn)</li></ul> |
| Date | October 2020 |
| Learning Objectives | <ul><li>Set up dev environment</li><li>Set up IoT Hub</li><li>Send data from a device to IoT Hub</li><li>Call a device function from IoT Hub/Azure IoT Explorer</li><li>Send Cloud to Device message from IoT Hub/Azure IoT Explorer</li><li>Consume data from default Event Hub endpoint</li><li>Visualize co-ordinates and direction on Azure Maps</li></ul> |
| Time to complete | 4 hours |


## The lab parts

This lab has the following parts

1. Set up IoT Hub and IoT Deivce
1. Prepare development environment for programming the STM DevKit in C
1. Set up STM32 and Neo 6m GPS or virtual IoT device to send GPS co-ordinates and direction along with other onboard sensor data
1. Call a direct method/function on device using either Azure IoT Hub or [Azure IoT Explorer](https://docs.microsoft.com/azure/iot-pnp/howto-use-iot-explorer)
1. Send Cloud to Device (C2D) messages using Azure IoT Hub or [Azure IoT Explorer](https://docs.microsoft.com/azure/iot-pnp/howto-use-iot-explorer)
1. Write a client to connect to IoT hub to read the device telemetry and display device co-ordinates and direction

These parts will cover in detail what needs to be done at each step where appropriate, or link to official documentation to cover steps - that way the parts will stay more up to date.

## Azure IoT Hub

[Azure IoT Hub](https://azure.microsoft.com/services/iot-hub/?WT.mc_id=iotcurriculum-github-jabenn) provides a cloud-hosted solution back end to connect virtually any device.

Azure IoT Hub has a free tier that allows 8000 messages/day per Iot Hub unit. If you want to send/receive more messages, you will need to select a different pricing tier. You can find pricing details on the [Azure IoT Hub pricing page](https://azure.microsoft.com/pricing/details/iot-hub/?WT.mc_id=iotcurriculum-github-jabenn).

All the documentation for IoT Hub is available in the [Microsoft IoT Hub docs](https://docs.microsoft.com/azure/iot-hub/?WT.mc_id=iotcurriculum-github-jabenn). Refer to these docs for the latest up-to date information on using IoT Hub.

## Azure RTOS

[Azure RTOS](https://azure.microsoft.com/services/rtos/?WT.mc_id=iotcurriculum-github-jabenn) is an embedded development suite including a small but powerful operating system that provides reliable, ultra-fast performance for resource-constrained devices. It’s easy-to-use and market-proven, having been deployed on more than 6.2 billion devices worldwide. Azure RTOS supports the most popular 32-bit microcontrollers and embedded development tools, so you can make the most of your team’s existing skills.

The Azure RTOS makes use of [Azure IoT device SDK for C](https://docs.microsoft.com/azure/iot-hub/iot-hub-device-sdk-c-intro) that allows you to connect your device using WI-FI and send the telemetry to Azure IOT hub over [MQTT](https://mqtt.org). MQTT is a popular standard for communicating with IoT devices, it's a lightweight protocol for publish/subscribe message transport. You can read more on MQTT and the implementation in the [Communicate with your IoT hub using the MQTT protocol documentation](https://docs.microsoft.com/azure/iot-hub/iot-hub-mqtt-support?WT.mc_id=iotcurriculum-github-jabenn). This documentation covers the protocol and how messages are published or subscribed to.

## Azure Maps

Create location-aware web and mobile applications using simple and secure geospatial services, APIs, and SDKs in Azure. Deliver seamless experiences based on geospatial data with built-in location intelligence from world-class mobility technology partners.

See [here](https://azure.microsoft.com/services/azure-maps/) for more details.

## Azure subscription

These labs are designed for courses where Azure resources are provided to students by the institution. To try them out, you can use one of our free subscriptions. Head to the [Azure Subscriptions Guide](../../../azure-subscription.md) for more information on setting up a subscription.

## Labs

These labs all build on one another, so you need to work through them in order. Work through as many labs as you want to, but if you don't complete all the labs, make sure you always do the [last one](./steps/clean-up.md) as that cleans up your Azure resources.

Some labs have two options - select the STM32 option if you have a STM32 and the sensors, otherwise use the virtual device option.

1. [Set up IoT Hub and Register a Device](./steps/set-up-iot-hub-and-device.md)

1. [Set up a STM 32 and Neo 6m GPS sensor to send device telemtry](./steps/set-up-stm32.md)

1. [Trigger a function on the device](./steps/rules-stm-functioncall.md)

1. [Send Cloud to Device (C2D) message](./steps/rules-stm-c2d.md)

1. [Setup Azure Maps](./steps/set-up-azure-maps.md)

1. [Setup Web App](./steps/set-up-web-app.md)

1. [Clean up](./steps/clean-up.md)

## Clean up

Don't forget to clean up your Azure resources when you are finished, to avoid spending money, or using up your credit from your free subscription. All the instructions to clean up are in the [Clean up you Azure resources guide](./steps/clean-up.md).
