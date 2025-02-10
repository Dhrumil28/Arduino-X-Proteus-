#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>

// Wi-Fi credentials
const char* ssid = "Dedsec";
const char* password = "28082001";

// N2YO API URL (or another provider's URL)
const char* apiKey = "C9MC3Y-BFAQEM-VNJ2CF-5EX3";  
const char* latitude = "20.5937";  // India's Latitude  
const char* longitude = "78.9629"; // India's Longitude  
const char* altitude = "0";        // Altitude in km  

String spaceURL = "https://api.n2yo.com/rest/v1/satellite/above/" + 
                  String(latitude) + "/" + String(longitude) + "/" + 
                  String(altitude) + "/90/0/&apiKey=" + String(apiKey);

// Initialize WebServer
WebServer server(80);

// HTML Content to serve
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 Spacecraft Tracker</title>
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.7.1/dist/leaflet.css" />
    <script src="https://unpkg.com/leaflet@1.7.1/dist/leaflet.js"></script>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #2c3e50;
            color: white;
            margin: 0;
            padding: 0;
        }
        h1 {
            text-align: center;
            color: #1abc9c;
            margin-top: 20px;
        }
        #map {
            width: 100%;
            height: 400px;
        }
        .container {
            padding: 20px;
            max-width: 900px;
            margin: 20px auto;
            background-color: rgba(0, 0, 0, 0.6);
            border-radius: 10px;
        }
        .satellite-list {
            margin-top: 20px;
            padding: 10px;
            background-color: rgba(0, 0, 0, 0.3);
            border-radius: 5px;
        }
        .satellite-item {
            padding: 10px;
            border-bottom: 1px solid #444;
        }
        .satellite-item:last-child {
            border-bottom: none;
        }
        .satellite-name {
            font-weight: bold;
            color: #1abc9c;
        }
        .satellite-info {
            font-size: 0.9em;
        }
        .satellite-count {
            font-size: 1.5em;
            text-align: center;
            color: #1abc9c;
            margin: 10px 0;
        }
        @keyframes twinkle {
            0% { opacity: 0.3; }
            50% { opacity: 1; }
            100% { opacity: 0.3; }
        }
        .star {
            position: fixed;
            width: 2px;
            height: 2px;
            background: white;
            animation: twinkle 1s infinite;
        }
    </style>
</head>
<body>
    <h1>🚀 Spacecraft Tracker Over India</h1>
    <div id="map"></div>
    <div class="container">
        <div class="satellite-count">
            <b>Satellites Visible Over India:</b> <span id="satelliteCount">0</span>
        </div>
        <div class="satellite-list" id="satelliteList">
            <!-- Satellite List Will Be Populated Here -->
        </div>
    </div>

    <script>
        // Initialize map
        var map = L.map('map').setView([20.5937, 78.9629], 4); // India Latitude, Longitude

        // Add OpenStreetMap tiles to the map
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
        }).addTo(map);

        var satelliteCount = 0;

        // Create twinkling stars in the background
        for (let i = 0; i < 50; i++) {
            const star = document.createElement('div');
            star.className = 'star';
            star.style.left = Math.random() * 100 + 'vw';
            star.style.top = Math.random() * 100 + 'vh';
            star.style.animationDelay = Math.random() * 3 + 's';
            document.body.appendChild(star);
        }

        // Fetch satellites and update map/list
        function fetchSatellites() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    satelliteCount = 0;
                    let satelliteListHTML = "";
                    data.satellites.forEach(sat => {
                        satelliteCount++;

                        // Add satellite marker on the map
                        var marker = L.marker([sat.latitude, sat.longitude]).addTo(map);
                        marker.bindPopup("<b>" + sat.name + "</b><br>Altitude: " + sat.altitude + " km<br>Speed: " + sat.speed + " km/h");

                        // Add satellite to the list
                        satelliteListHTML += `
                            <div class="satellite-item">
                                <div class="satellite-name">${sat.name}</div>
                                <div class="satellite-info">
                                    Altitude: ${sat.altitude} km | Speed: ${sat.speed} km/h | Location: ${sat.latitude.toFixed(2)}, ${sat.longitude.toFixed(2)}
                                </div>
                            </div>
                        `;
                    });

                    // Update satellite count and list
                    document.getElementById("satelliteCount").innerText = satelliteCount;
                    document.getElementById("satelliteList").innerHTML = satelliteListHTML;
                })
                .catch(error => console.error('Error fetching satellites:', error));
        }

        // Call the fetch function periodically
        setInterval(fetchSatellites, 10000); // Refresh every 10 seconds
        fetchSatellites(); // Initial call to display satellites
    </script>
</body>
</html>
)rawliteral";

// HTTP Request Handlers
void handleRoot() {
    server.send(200, "text/html", index_html);
}

void handleData() {
    HTTPClient http;
    http.begin(spaceURL);
    int httpCode = http.GET();
    String payload;

    if (httpCode == 200) {
        payload = http.getString();
        DynamicJsonDocument doc(4096);
        deserializeJson(doc, payload);

        JsonArray sats = doc["above"];
        DynamicJsonDocument response(4096);
        JsonArray satArray = response.createNestedArray("satellites");

        for (JsonObject sat : sats) {
            JsonObject satObj = satArray.createNestedObject();
            satObj["name"] = sat["satname"].as<String>();
            satObj["latitude"] = sat["satlat"].as<float>();
            satObj["longitude"] = sat["satlng"].as<float>();
            satObj["altitude"] = sat["satalt"].as<float>();
            satObj["speed"] = sat["satvelocity"].as<float>();
        }

        String jsonResponse;
        serializeJson(response, jsonResponse);
        server.send(200, "application/json", jsonResponse);
    } else {
        server.send(500, "application/json", "{\"error\":\"Failed to fetch data\"}");
    }
    http.end();
}

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    Serial.println(WiFi.localIP());

    // Start Web Server
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
}

void loop() {
    server.handleClient();
}
