
#include <NTPClient.h>
#include <WiFiUdp.h>


const long utcOffsetInSeconds = 25200; /* 7*60*60 */

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void GetTime_Init(void)
{
    timeClient.begin();
}

void GetTime_Update(void)
{
    timeClient.update();
}

void GetTime_GetCurrentTime(uint8_t *h, uint8_t *m, uint8_t *s)
{
    timeClient.update();
    *h = timeClient.getHours();
    *m = timeClient.getMinutes();
    *s = timeClient.getSeconds();
}

int GetTime_GetSec(void)
{
    return timeClient.getSeconds();
}

int GetTime_GetMin(void)
{
    return timeClient.getMinutes();
}

int GetTime_GetHour(void)
{
    return timeClient.getHours();
}