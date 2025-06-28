#include <ESP8266mDNS.h>
#include "WebHtml.h"
#include "DhtApp.h"
#include "GetTime.h"
#include "NodeMcu_Mixed_Defines.h"

ESP8266WebServer WebHtml_Server(80);

const char INDEX_HTML[] = R"__--__(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Arduino LED & Sensor Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            padding: 20px;
            background-color: #f5f5f5;
        }

        h1 {
            color: #333;
        }

        .container {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 20px;
        }

        .column {
            flex: 1;
            min-width: 300px;
            max-width: 400px;
            padding: 10px;
            background: #fff;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            border-radius: 5px;
        }

        h2 {
            color: #007BFF;
        }

        button {
            width: 100px;
            height: 40px;
            font-size: 16px;
            margin: 10px 5px;
            background-color: #007BFF;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
        }

        button:hover {
            background-color: #0056b3;
        }

        .status {
            font-weight: bold;
            color: #555;
            margin-top: 10px;
        }
    </style>
</head>

<body>
    <h1>Control Your Arduino</h1>
    <div class="container">
        <!-- LED Controls Column -->
        <div class="column">
            <h2>LED Controls</h2>
            <h3>Built-in LED</h3>
            <button onclick="sendData('ON', 'ledBuiltInState')">ON</button>
            <button onclick="sendData('OFF', 'ledBuiltInState')">OFF</button>
            <p id="ledBuiltInState" class="status">Built-in LED state: {{__builtin_state}}</p>
            <h3>External LED</h3>
            <button onclick="sendData('ON', 'ledState')">ON</button>
            <button onclick="sendData('OFF', 'ledState')">OFF</button>
            <p id="ledState" class="status">External LED state: {{__state}}</p>
        </div>

        <!-- Sensor Data Column -->
        <div class="column">
            <h2>Sensor Data</h2>
            <p>Temperature: <span id="temperature">{{__temperature}}</span> &deg;C</p>
            <p>Heat Index: <span id="heatid">{{__heatid}}</span> &deg;C</p>
            <p>Humidity: <span id="humidity">{{__humidity}}</span> %</p>
        </div>
        <div class="column">
            <h2>Timezone</h2>
            <select id="timezoneSelect" onchange="setTimezone()">
                <option value="Etc/GMT+12">(GMT-12:00) Baker Island</option>
                <option value="Etc/GMT+11">(GMT-11:00) American Samoa</option>
                <option value="Pacific/Honolulu">(GMT-10:00) Hawaii</option>
                <option value="America/Anchorage">(GMT-9:00) Alaska</option>
                <option value="America/Los_Angeles">(GMT-8:00) Los Angeles</option>
                <option value="America/Denver">(GMT-7:00) Denver</option>
                <option value="America/Chicago">(GMT-6:00) Chicago</option>
                <option value="America/New_York">(GMT-5:00) New York</option>
                <option value="America/Caracas">(GMT-4:00) Caracas</option>
                <option value="America/Halifax">(GMT-3:00) Halifax</option>
                <option value="America/Noronha">(GMT-2:00) Noronha</option>
                <option value="Atlantic/Azores">(GMT-1:00) Azores</option>
                <option value="UTC">(GMT±0:00) UTC</option>
                <option value="Europe/Berlin">(GMT+1:00) Berlin</option>
                <option value="Europe/Athens">(GMT+2:00) Athens</option>
                <option value="Asia/Baghdad">(GMT+3:00) Baghdad</option>
                <option value="Asia/Dubai">(GMT+4:00) Dubai</option>
                <option value="Asia/Karachi">(GMT+5:00) Karachi</option>
                <option value="Asia/Dhaka">(GMT+6:00) Dhaka</option>
                <option value="Asia/Hanoi" selected>(GMT+7:00) Hanoi</option>
                <option value="Asia/Shanghai">(GMT+8:00) Beijing</option>
                <option value="Asia/Tokyo">(GMT+9:00) Tokyo</option>
                <option value="Australia/Sydney">(GMT+10:00) Sydney</option>
                <option value="Pacific/Noumea">(GMT+11:00) New Caledonia</option>
                <option value="Pacific/Auckland">(GMT+12:00) Auckland</option>
            </select>
        </div>
    </div>

    <footer>
        <p>&copy; 2025 Arduino Controller</p>
    </footer>

    <script>
        function setTimezone() {
            const tz = document.getElementById("timezoneSelect").value;

            // Send selected timezone to the ESP8266 server
            const xhr = new XMLHttpRequest();
            xhr.open("GET", `/setTimezone?tz=${encodeURIComponent(tz)}`, true);
            xhr.send();
        }

        function sendData(state, id) {
            const xhr = new XMLHttpRequest();
            xhr.open('GET', `/${id}?state=${state}`, true);
            xhr.onreadystatechange = function () {
                if (xhr.readyState === 4 && xhr.status === 200) {
                    if (id === 'ledBuiltInState') {
                        document.getElementById(id).innerText = `Built-in LED state: ${state}`;
                    } else if (id === 'ledState') {
                        document.getElementById(id).innerText = `External LED state: ${state}`;
                    }
                }
            };
            xhr.send();
        }

        function updateSensorData() {
            const xhr = new XMLHttpRequest();
            xhr.open('GET', '/sensor', true);
            xhr.onreadystatechange = function () {
                if (xhr.readyState === 4 && xhr.status === 200) {
                    const sensorData = JSON.parse(xhr.responseText);
                    document.getElementById('temperature').innerText = sensorData.temperature;
                    document.getElementById('heatid').innerText = sensorData.heatid;
                    document.getElementById('humidity').innerText = sensorData.humidity;
                }
            };
            xhr.send();
        }

        setInterval(updateSensorData, 5000);
    </script>
</body>

</html>
)__--__";

static void WebHtml_HandleLedState(void);
static void WebHtml_HandleBuiltInLedState(void);
static void WebHtml_HandleSensor(void);
static void WebHtml_HandleRoot(void);
static void WebHtml_HandleTimezone(void);

void WebHtml_Init(void)
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(TEST_LED, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(TEST_LED, LOW);

    if (MDNS.begin("espserver")) {
        Serial.println("MDNS responder started");
    } else {
        Serial.println("Error setting up MDNS responder!");
    }

    WebHtml_Server.on("/", WebHtml_HandleRoot);
    WebHtml_Server.on("/ledState", WebHtml_HandleLedState);
    WebHtml_Server.on("/ledBuiltInState", WebHtml_HandleBuiltInLedState);
    WebHtml_Server.on("/sensor", WebHtml_HandleSensor);
    WebHtml_Server.on("/setTimezone", WebHtml_HandleTimezone);
    WebHtml_Server.begin();
    Serial.println("HTTP WebHtml_Server started");
}

static void WebHtml_HandleBuiltInLedState(void)
{
    String state = WebHtml_Server.arg("state");
    if (state == "ON")
    {
        digitalWrite(LED_BUILTIN, LOW); // Turn the LED on
    }
    else if (state == "OFF")
    {
        digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off
    }
    WebHtml_Server.send(200, "text/plain", "OK");
}

static void WebHtml_HandleLedState(void)
{
    String state = WebHtml_Server.arg("state");
    if (state == "ON")
    {
        digitalWrite(TEST_LED, HIGH); // Turn the LED on
    }
    else if (state == "OFF")
    {
        digitalWrite(TEST_LED, LOW); // Turn the LED off
    }
    WebHtml_Server.send(200, "text/plain", "OK");
}

static void WebHtml_HandleSensor(void)
{
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    String sensorData = "{";
    sensorData += "\"temperature\": \"" + String(t) + "\",";
    sensorData += "\"heatid\": \"" + String(dht.computeHeatIndex(t, h, false)) + "\",";
    sensorData += "\"humidity\": \"" + String(h) + "\"}";
    WebHtml_Server.send(200, "application/json", sensorData);
}

// Assume timezoneNames[] and getOffsetForTimezone() are defined elsewhere
static void WebHtml_HandleTimezone(void) {
    if (WebHtml_Server.hasArg("tz")) {
        String tz = WebHtml_Server.arg("tz");

        long offset = GetTime_GetOffsetForTimezone(tz);
        if (tz != currentTimezone) {
            currentTimezone = tz;
            utcOffsetInSeconds = offset;

            // Apply new offset to NTPClient if running
            timeClient.setTimeOffset(utcOffsetInSeconds);
            GetTime_GetCurrentTime(&currentTime.h, &currentTime.m, &currentTime.s);

            Serial.println("Timezone updated to: " + currentTimezone);
            Serial.print("UTC offset (seconds): ");
            Serial.println(utcOffsetInSeconds);
        }
    } else {
        Serial.println("No timezone parameter received.");
    }

    WebHtml_Server.send(200, "text/plain", "Timezone set to: " + currentTimezone);
}

static void WebHtml_HandleRoot(void)
{
    // Read the sensor values
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    float heatIndex = dht.computeHeatIndex(t, h, false);

    Serial.println("Client connected.");
    // Determine the LED states
    String ledBuiltInState = digitalRead(LED_BUILTIN) == LOW ? "ON" : "OFF";
    String ledState = digitalRead(TEST_LED) == HIGH ? "ON" : "OFF";

    // Replace placeholders in the HTML
    String html = INDEX_HTML;
    html.replace("{{__temperature}}", String(t));
    html.replace("{{__heatid}}", String(heatIndex));
    html.replace("{{__humidity}}", String(h));
    html.replace("{{__builtin_state}}", ledBuiltInState);
    html.replace("{{__state}}", ledState);
    html.replace("<option value=" + currentTimezone + ">", "<option value=" + currentTimezone + " selected>");

    // Send the updated HTML to the client
    WebHtml_Server.send(200, "text/html", html);
}
