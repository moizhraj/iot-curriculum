
var map, datasource

var lastLayer = null;

var mapSubscriptionKey = 'O_azZVgz2yPHC_nLbTntZzbcpZF7LYx7kO87PfcOv70';

// The arrow icon faces left, however the direction data takes North (Top) as Zero degree
// So we move the arrow 90 degrees initially to make it point North 
var rotationOffset = 90; 

function GetMap()
{
    //Initialize a map instance.
    map = new atlas.Map('myMap', {
        center: [-122.18822, 47.63208],
        zoom: 10,
        view: "Auto",
        style: "road",
        authOptions: {
            authType: 'subscriptionKey',
            subscriptionKey: mapSubscriptionKey
        }
    });

    //Wait until the map resources are ready.
    map.events.add('ready', function ()
    {
        //Add the zoom control to the map.
        map.controls.add(new atlas.control.ZoomControl(), {
            position: 'top-right'
        });

        // Add the images that will be used to build the symbol layer icons (Arrow and small circle)
        map.imageSprite.add('arrow-icon', 'https://upload.wikimedia.org/wikipedia/commons/thumb/0/0d/Red_Arrow_Left.svg/64px-Red_Arrow_Left.svg.png');
        map.imageSprite.add('point-icon', 'https://upload.wikimedia.org/wikipedia/commons/0/0e/Basic_red_dot.png');
    
        console.log("Socket Started -- ");

        //connect to the socket server.
        var socket = io.connect('http://' + document.domain + ':' + location.port + '/get_data');
    
        //receive details from server
        socket.on('mapdata', function (msg)
        {
            console.log("Data received");
            showDeviceData(msg.data);
        });
    
    });
}

function showDeviceData(data)
{
    // Test data
    //data = data || '{ "temperature": 25.86, "humidity": 64.32, "pressure": 1006.84, "accelX": -3, "accelY": -2, "accelZ": 1003, "gyroX": 840.00, "gyroY": -2730.00, "gyroZ": 910.00, "magX": -94, "magY": 221, "magZ": -703, "cmagX": 204, "cmagY": 50, "cmagZ": -520, "heading": "45", "location": "N 47.63, W 122.1882" }';
    data = JSON.parse(data);
    var location = data.location.split(",");
    
    // The data comes from IoT hub as N S and E W. However, Azure Maps accepts them as + or - values. 
    // The following code converts S and W as '-' ve
    var lat = location[0].replace('N', '').replace('S ', '-').trim();
    var long = location[1].replace('E', '').replace('W ', '-').trim();

    // Show the data coming from IoT hub inside a div
    $("#deviceData ul").empty();
    $("#deviceData ul").append("<li> <b>Temperature:</b> " + data.temperature + " &deg;C </li>");
    $("#deviceData ul").append("<li> <b>Humidity:</b> " + data.humidity + "% </li>");
    $("#deviceData ul").append("<li> <b>Pressure:</b> " + data.pressure + " Millibar </li>");
    $("#deviceData ul").append("<li> <b>Location:</b> " + data.location + " </li>");

    var dataSource = new atlas.source.DataSource();
    var point = new atlas.Shape(new atlas.data.Point([long, lat ]));
    dataSource.add([point]);
    map.sources.add(dataSource);

    // Add a symbol in the GPS position received from IoT hub
    // Make the arrow direction as coming in the 'heading' value
    var ic = new atlas.layer.SymbolLayer(dataSource, null, {
        iconOptions: {
            image: 'arrow-icon',
            allowOverlap: true,
            anchor: 'center',
            size: 0.3,
            rotation: rotationOffset + parseFloat(data.heading) 
        }
    });

    if (lastLayer)
    {
        // Change the last symbol layer from arrow to a point
        map.layers.remove(lastLayer);
        var pointLayer = new atlas.layer.SymbolLayer(lastLayer.getSource(), null, {
            iconOptions: {
                image: 'point-icon',
                allowOverlap: true,
                anchor: 'center',
                size: 0.02,
                rotation: rotationOffset + parseFloat(data.heading)
            }
        });

        map.layers.add(pointLayer);
    }
    
    map.layers.add(ic);

    // Save the last symbol layer
    lastLayer = ic;
}