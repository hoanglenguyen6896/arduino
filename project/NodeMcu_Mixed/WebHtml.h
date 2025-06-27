#ifndef WEBHTML_H
#define WEBHTML_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "NodeMcu_Mixed_Defines.h"

extern ESP8266WebServer WebHtml_Server;
extern const char INDEX_HTML[];

void WebHtml_Init(void);

#endif /* WEBHTML_H */
