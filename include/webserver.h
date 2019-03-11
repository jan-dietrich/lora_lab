#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

//used to disable the core 0 watchdog
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

//data for LoRa
extern MessageBuffer_t SendBuffer;

//set up for the webserver and access point
void wifi_initialize();

//handle JavaScript requests
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

//sets the website log
void wifi_setlog(String log);

//updates the user webpage when socket ist established
void updateWebpage(String data);

//decodes incoming webcmd from user
void decode_webcmd(char * data);
#endif