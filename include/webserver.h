#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

//data for LoRa
extern MessageBuffer_t SendBuffer;

//set up for the webserver and access point
void wifi_initialize(void * parameter);

//shall run all the time to react to new clients
void wifi_polling();

//sets the website log
void wifi_setlog(String log);

#endif