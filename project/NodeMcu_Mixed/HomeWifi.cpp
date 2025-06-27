#include "HomeWifi.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

const int32_t ssidAddress = 0;
const int32_t passAddress = 100;

char ssid[32];
char password[32];


static void HomeWifi_ReadCredentials();

/* Inialize wifi */
void HomeWifi_Init(void)
{
    int32_t status;

    HomeWifi_ReadCredentials();

    WiFi.begin(ssid, password);


    while (WiFi.status() != WL_CONNECTED)
    {
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);

        Serial.print("WiFi status: ");
        status = WiFi.status();
        Serial.print(status);
        Serial.print(" - ");
        Serial.println(HomeWifi_GetWiFiStatusMeaning(status));
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

/* Print wifi meaning */
String HomeWifi_GetWiFiStatusMeaning(int32_t status)
{
    switch (status)
    {
    case WL_IDLE_STATUS:
        return "Idle";
    case WL_NO_SSID_AVAIL:
        return "No SSID Available";
    case WL_SCAN_COMPLETED:
        return "Scan Completed";
    case WL_CONNECTED:
        return "Connected";
    case WL_CONNECT_FAILED:
        return "Connect Failed";
    case WL_CONNECTION_LOST:
        return "Connection Lost";
    case WL_DISCONNECTED:
        return "Disconnected";
    default:
        return "Unknown";
    }
}

/* Read credential from EEPROM at 0 and 100 */
static void HomeWifi_ReadCredentials()
{
    int32_t i;

    EEPROM.begin(512);
    for (i = 0; i < 32; ++i)
    {
        ssid[i] = EEPROM.read(ssidAddress + i);
        password[i] = EEPROM.read(passAddress + i);
    }
    ssid[31] = '\0';
    password[31] = '\0';
    EEPROM.end();
}
