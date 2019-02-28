/* ----------------- Code Component -----------------

    Copyright 2019 Jan Dietrich

    This component shall code and decode the messages that
    are send via LoRa

    Changelog:
    V1.0    Initial release

*/
#include "main.h"

// Local logging Tag
static const char TAG[] = "code";

//decode mode. Determins which decoder is used
int decodeMode = 1;

void decode_message(uint8_t cmd[], uint8_t cmdlength) {
switch (decodeMode)
{
    case 0:
    #if LOG_LEVEL > 2
    Serial.printf("%s:Data: ",TAG);
    for(int icoursor = 0; icoursor < LMIC.dataLen; icoursor ++){
        Serial.printf("%02X ",LMIC.frame[LMIC.dataBeg + icoursor]); 
    }
    Serial.printf("\n");
    #endif
    break;

    case 1:
    #if LOG_LEVEL > 2
    Serial.printf("%s:Data: ",TAG);
    for(int icoursor = 0; icoursor < LMIC.dataLen; icoursor ++){
        Serial.printf("%c",LMIC.frame[LMIC.dataBeg + icoursor]); 
    }
    Serial.printf("\n");
    #endif
    break;

    default:
        break;
}    

}