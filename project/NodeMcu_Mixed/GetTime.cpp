
#include "GetTime.h"

long utcOffsetInSeconds = 25200; /* 7*60*60 */

const char* timezoneNames[] = {
    "Etc/GMT+12", "Etc/GMT+11", "Pacific/Honolulu", "America/Anchorage",
    "America/Los_Angeles", "America/Denver", "America/Chicago", "America/New_York",
    "America/Caracas", "America/Halifax", "America/Noronha", "Atlantic/Azores",
    "UTC", "Europe/Berlin", "Europe/Athens", "Asia/Baghdad",
    "Asia/Dubai", "Asia/Karachi", "Asia/Dhaka", "Asia/Hanoi", // <- updated label
    "Asia/Shanghai", "Asia/Tokyo", "Australia/Sydney", "Pacific/Noumea", "Pacific/Auckland"
};


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
String currentTimezone = "Asia/Hanoi";

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

long GetTime_GetOffsetForTimezone(const String& tz)
{
    for (int i = 0; i < sizeof(timezoneNames) / sizeof(timezoneNames[0]); ++i) {
        if (tz == timezoneNames[i]) {
            return (i - 12) * 3600; // Index 12 is UTC
        }
    }
    return 0; // fallback to UTC
}
