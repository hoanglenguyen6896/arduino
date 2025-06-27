#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "DHT.h"

#include "WebHtml.h"

#define TEST_LED        (D0)

#define DHTPIN          (D3)
#define DHTTYPE         (DHT22)

ESP8266WebServer server(80);

DHT dht(DHTPIN, DHTTYPE);

const int ssidAddress = 0;
const int passAddress = 100;
char ssid[32];
char password[32];

unsigned long previousMillis = 0;
const long interval = 60000; // 60 seconds

void readCredentials() {
    EEPROM.begin(512);
    for (int i = 0; i < 32; ++i) {
        ssid[i] = EEPROM.read(ssidAddress + i);
        password[i] = EEPROM.read(passAddress + i);
    }
    ssid[31] = '\0';
    password[31] = '\0';
    EEPROM.end();
}

String getWiFiStatusMeaning(int status) {
    switch (status) {
        case WL_IDLE_STATUS: return "Idle";
        case WL_NO_SSID_AVAIL: return "No SSID Available";
        case WL_SCAN_COMPLETED: return "Scan Completed";
        case WL_CONNECTED: return "Connected";
        case WL_CONNECT_FAILED: return "Connect Failed";
        case WL_CONNECTION_LOST: return "Connection Lost";
        case WL_DISCONNECTED: return "Disconnected";
        default: return "Unknown";
    }
}

void handleLedState() {
    String state = server.arg("state");
    if (state == "ON") {
        digitalWrite(TEST_LED, HIGH); // Turn the LED on
    } else if (state == "OFF") {
        digitalWrite(TEST_LED, LOW); // Turn the LED off
    }
    server.send(200, "text/plain", "OK");
}

void handleLedBuiltInState() {
    String state = server.arg("state");
    if (state == "ON") {
        digitalWrite(LED_BUILTIN, LOW); // Turn the LED on
    } else if (state == "OFF") {
        digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off
    }
    server.send(200, "text/plain", "OK");
}

void handleSensor() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    String sensorData = "{";
    sensorData += "\"temperature\": \"" + String(t) + "\",";
    sensorData += "\"heatid\": \"" + String(dht.computeHeatIndex(t, h, false)) + "\",";
    sensorData += "\"humidity\": \"" + String(h) + "\"}";
    server.send(200, "application/json", sensorData);
}

void handleRoot() {
    // Read the sensor values
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    float heatIndex = dht.computeHeatIndex(t, h, false);

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

    // Send the updated HTML to the client
    server.send(200, "text/html", html);
}

void setup() {
    unsigned status;

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(TEST_LED, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(TEST_LED, LOW);

    Serial.begin(9600);
    dht.begin();

    readCredentials();
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);

        Serial.print("WiFi status: ");
        int status = WiFi.status();
        Serial.print(status);
        Serial.print(" - ");
        Serial.println(getWiFiStatusMeaning(status));
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("espserver")) {
        Serial.println("MDNS responder started");
    } else {
        Serial.println("Error setting up MDNS responder!");
    }

    server.on("/", handleRoot);
    server.on("/ledState", handleLedState);
    server.on("/ledBuiltInState", handleLedBuiltInState);
    server.on("/sensor", handleSensor);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
    MDNS.update(); // Keep mDNS responder active

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        int status = WiFi.status();
        Serial.print("WiFi status: ");
        Serial.print(status);
        Serial.print(" - ");
        Serial.println(getWiFiStatusMeaning(status));

        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
}
