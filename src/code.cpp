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

void set_decode_mode(int decode){
    decodeMode = decode;
    wifi_setlog("Dekodierung geändert");
    #if LOG_LEVEL > 2
        Serial.printf("%s:decode mode set to: %i\n",TAG,decodeMode);
    #endif
}

void decode_message(uint8_t data[], uint8_t datalength) {
    String wifi_output = "Empfangene Daten: ";
switch (decodeMode)
{
    case 0:
    #if LOG_LEVEL > 2
    Serial.printf("%s:Data: ",TAG);
    for(int icoursor = 0; icoursor < datalength; icoursor ++){
        Serial.printf("%02X ",data[icoursor]); 
    }
    Serial.printf("\n");
    #endif
    for(int icoursor = 0; icoursor < datalength; icoursor ++){
        wifi_output += String(data[icoursor],HEX) + " "; 
    }
    break;

    case 1:
    #if LOG_LEVEL > 2
    Serial.printf("%s:Data: ",TAG);
    for(int icoursor = 0; icoursor < datalength; icoursor ++){
        Serial.printf("%c",data[icoursor]); 
    }
    Serial.printf("\n");
    #endif
    for(int icoursor = 0; icoursor < datalength; icoursor ++){
        wifi_output += char(data[icoursor]); 
    }

    case 2: 
    switch (data[0]){
        case 0: 
            setLEDs(1,false);
            setLEDs(2,false);
            setLEDs(3,false);
        case 1: 
            setLEDs(1,true);
            setLEDs(2,false);
            setLEDs(3,false);
        case 2: 
            setLEDs(1,false);
            setLEDs(2,true);
            setLEDs(3,false);
        case 3: 
            setLEDs(1,false);
            setLEDs(2,false);
            setLEDs(3,true);
    }
    break;

    default:
        break;
}    
    wifi_setlog(wifi_output);
}