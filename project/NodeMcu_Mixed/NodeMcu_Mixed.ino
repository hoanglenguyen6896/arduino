#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "DhtApp.h"
#include "WebHtml.h"
#include "Led74Hc.h"
#include "HomeWifi.h"
#include "GetTime.h"

unsigned long previousMillis = 0;
unsigned long previousMillis_2 = 0;

// Counter variable to track the count
uint32_t counter = 0;
TimeStruct_Type currentTime;

void setup()
{
    unsigned status;

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(TEST_LED, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(TEST_LED, LOW);

    Serial.begin(9600);

    DhtApp_Init();
    HomeWifi_Init();
    WebHtml_Init();
    GetTime_Init();
    // GetTime_Update();
    GetTime_GetCurrentTime(&currentTime.h, &currentTime.m, &currentTime.s);
    Led74Hc_Type _Config = {
        .SHCP = D6,
        .STCP = D7,
        .DS = D5,
    };
    Led74Hc_Init(_Config);
}

void loop()
{
    unsigned long currentMillis = millis();

    WebHtml_Server.handleClient();
    MDNS.update(); // Keep mDNS responder active

    if (NTP_INTERVAL >= counter)
    {
        if (currentMillis - previousMillis_2 >= 1000)
        {
            previousMillis_2 = currentMillis;
            counter++;
            currentTime.s++;
            if (60 <= currentTime.s)
            {
                currentTime.s = 0;
                currentTime.m++;
                if (60 <= currentTime.m)
                {
                    currentTime.m = 0;
                    currentTime.h++;
                    if (24 <= currentTime.h)
                    {
                        currentTime.h = 0;
                    }
                }
            }
        }
    }
    else
    {
        counter = 0;
        GetTime_GetCurrentTime(&currentTime.h, &currentTime.m, &currentTime.s);
    }
    Led74Hc_ShowValue(currentTime.h*100 + currentTime.m, 4, 0b00000100);

    if (currentMillis - previousMillis >= SERIAL_INTERVAL)
    {
        previousMillis = currentMillis;

        int status = WiFi.status();
        Serial.print("WiFi status: ");
        Serial.print(status);
        Serial.print(" - ");
        Serial.println(HomeWifi_GetWiFiStatusMeaning(status));

        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        float t = dht.readTemperature();
        float h = dht.readHumidity();
        float heatIndex = dht.computeHeatIndex(t, h, false);

        Serial.print(F("Humidity: "));
        Serial.print(h);
        Serial.print(F("%  Temperature: "));
        Serial.print(t);
        Serial.print(F("°C  Heat index: "));
        Serial.print(heatIndex);
        Serial.print(F("°C "));
    }
    // else if (currentMillis - previousMillis_2 >= 1000)
    // {
    //     previousMillis_2 = currentMillis;
    //     Serial.print(currentTime.h);
    //     Serial.print(" ");
    //     Serial.print(currentTime.m);
    //     Serial.print(" ");
    //     Serial.print(currentTime.s);
    //     Serial.print("\n");
    // }
}
