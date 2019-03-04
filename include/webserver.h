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

//shall run all the time to react to new clients
void wifi_polling();

//sets the website log
void wifi_setlog(String log);

#endif