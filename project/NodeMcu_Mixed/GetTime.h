#ifndef GETTIME_H
#define GETTIME_H

#include <Arduino.h>

void GetTime_Init(void);

void GetTime_Update(void);

int GetTime_GetSec(void);

int GetTime_GetMin(void);

int GetTime_GetHour(void);

void GetTime_GetCurrentTime(uint8_t *h, uint8_t *m, uint8_t *s);
#endif /* GETTIME_H */
