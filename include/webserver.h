#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>


//set up for the webserver and access point
void wifi_setup();

//shall run all the time to react to new clients
void wifi_polling();

#endif