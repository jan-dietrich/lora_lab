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

//initialize lora
xTaskCreatePinnedToCore(lora_initialize, "lora_initialize", 4096, NULL, 5, NULL, 1);

//wifi setup
xTaskCreatePinnedToCore(wifi_initialize, "wifi_initialize", 16384, NULL, 4, NULL, 0);
}

void loop() {

}
