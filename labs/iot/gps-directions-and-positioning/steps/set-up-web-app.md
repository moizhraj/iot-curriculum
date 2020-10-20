# Run the Web Appliction
In the [previous step](./set-up-azure-maps.md) you set up the Azure Maps account.

In this step you will run the web application to display the device data on Azure Maps.

## Steps

1. Open Visual Studio Code
1. Click Open Folder and select the [code](../code/client) folder
1. Set up the python virtual environment by following steps in [Setting Up Python with Visual Studio Code](https://code.visualstudio.com/docs/python/tutorial-flask) till **'Create and run a minimal Flask app'**
1.  Create a new file called .env inside code folder.
1. Open the .env file and add
```python
CONNECTION_STR = '<Iot_Hub_Connection_String>'
CONSUMER_GROUP_NAME = '<Consumer_Group_Name>'
```
1. Update The .env file.

Replace
    
```python
    <Iot_Hub_Connection_String>
```
with 'Event Hub Compatible Endppoint' in [Set up IoT Hub](./set-up-iot-hub-and-device.md)
and
```python
    <CONSUMER_GROUP_NAME>
```
with 'Consumer Group Name' in [Set up IoT Hub](./set-up-iot-hub-and-device.md)

1. Replace <Iot_Hub_Connection_String> from [Set up IoT Hub](./set-up-iot-hub-and-device.md)
1. Open a terminal in Visual Studio Code and make sure the current directory is [code/client](../code/client) folder
1. If not then go to File -> 'Add Folder to Workspace' and select the 'Client' folder.
1. All the installables are listed in [requirements.txt](../code/client/requirements.txt). Install the libraries by running the following commands in command prompt

    ```sh
    pip3 install -r requirements.txt
    ```
5. In the [Application.js](../code/client/static/js/application.js) file, change the **<Azure_Maps_Subscription_Key>** with the primary key that you got in [Set up Azure Maps](./set-up-azure-maps.md)
    ```js
    var mapSubscriptionKey = '<Azure_Maps_Subscription_Key>';
    ```
## Run the Python Code
1. Run the web application with the following command in the terminal
    ```sh
        python -m flask run
    ```
1. Note down the port. Normally it should be 5000. If not use that port number in next step.
![Web Server Running](../images/web-server-running.png)
1. Open a browser and open http://localhost:5000
1. Make sure that your device is connected and sending data to the IoT hub
1. You should see an Azure Maps.
1. You should see the data coming from the device in the left side of the map.
1. You should also see a red arrow in the map. If you do not see the arrow, please zoom out the map and go near the location of your GPS device.
1. You should continue seeing more device data every 5 seconds. If your device is moving, then you should see a plot like follows
![Azure Maps](../images/map-view.png)


## Troubleshoot
1. If you do not see any map loading please check the **Azure Maps subscription key** is correct.
1. If you do not see the arrow in the map check that the .env file has right values
1. Open F12 and go to console to check if you see any error messages.
4. ![Messages](../images/map-view-2.png)

## Next steps

In this step you have run the web application and displayed the position received from the GPS receiver in the map.

In the [next step](./clean-up.md) you will clean up your resources.

