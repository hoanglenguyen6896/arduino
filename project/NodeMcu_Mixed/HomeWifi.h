#ifndef HOMEWIFI_H
#define HOMEWIFI_H

#include <Arduino.h>

extern const char INDEX_HTML[];

/* Inialize wifi */
void HomeWifi_Init(void);

/* Print wifi meaning */
String HomeWifi_GetWiFiStatusMeaning(int32_t status);

#endif /* HOMEWIFI_H */
