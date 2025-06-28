#ifndef GETTIME_H
#define GETTIME_H

#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

extern long utcOffsetInSeconds;
extern NTPClient timeClient;
extern String currentTimezone;

void GetTime_Init(void);

void GetTime_Update(void);

int GetTime_GetSec(void);

int GetTime_GetMin(void);

int GetTime_GetHour(void);

void GetTime_GetCurrentTime(uint8_t *h, uint8_t *m, uint8_t *s);

long GetTime_GetOffsetForTimezone(const String& tz);
#endif /* GETTIME_H */
