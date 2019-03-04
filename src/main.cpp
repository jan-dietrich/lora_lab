/* ----------------- DHBW LoRa Module -----------------

    Copyright 2019 Jan Dietrich

    This module shall be used to teach students about the 
    possibilities and limits of the LoRaWAN standard.
    It can be run on an ESP-32 device with additional
    LoRa Chip.

    Changelog:
    V1.0    Initial release

*/

// Basic Config
#include "main.h"

// Tag for debugging output
static const char TAG[] = "main";

void setup() {
// enable serial monitor if log level is high enough
#if LOG_LEVEL > 0
Serial.begin(115200);
Serial.printf("%s:Serial monitor enabled\n", TAG);
Serial.printf("%s:Debugging Level is set to: %i\n",TAG, LOG_LEVEL);
#endif

#if LOG_LEVEL > 2
Serial.printf("%s:Currently running %s in version %s\n",TAG, ESPNAME, PROGVERSION);
#endif

//session parameters are set static at the moment. Shall be changable via WiFi later
u1_t NWKSKEY[16] = { 0xA7, 0x59, 0xE8, 0xCE, 0x11, 0xE3, 0x22, 0x6C, 0x2B, 0x22, 0xB5, 0x7F, 0x06, 0x3B, 0xFF, 0x1B };
u1_t APPSKEY[16] = { 0xF7, 0x63, 0xDE, 0x9B, 0xD8, 0xBF, 0x2E, 0x25, 0xE1, 0xEE, 0xA4, 0xE1, 0x90, 0x2D, 0x04, 0x6F };
u4_t DEVADDR = 0x26011C08 ;

//initialize lora
lora_setabpkeys(NWKSKEY,APPSKEY,DEVADDR);
xTaskCreatePinnedToCore(lora_initialize, "lora_initialize", 2048, NULL, 5, NULL, 1);

//wifi setup
xTaskCreatePinnedToCore(wifi_initialize, "wifi_initialize", 16384, NULL, 4, NULL, 0);
}

void loop() {

}
