/* ----------------- Code Component -----------------

    Copyright 2019 Jan Dietrich

    This component shall interact with the board hardware. it 
    shall be possible to read sensor data and set the leds 

    Changelog:
    V1.0    Initial release

*/
#include "main.h"

// Local logging Tag
static const char TAG[] = "data";

void hardware_init(){
    //prepare the led pins
    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);
    pinMode(PIN_LED_Y, OUTPUT);
}

void setLEDs(int i, bool val){
    switch (i)
    {
        case 1:
            if (val) digitalWrite(PIN_LED_R, HIGH);
            else digitalWrite(PIN_LED_R, LOW);
            break;
        case 2:
            if (val) digitalWrite(PIN_LED_Y, HIGH);
            else digitalWrite(PIN_LED_Y, LOW);
            break;
        case 3:
            if (val) digitalWrite(PIN_LED_G, HIGH);
            else digitalWrite(PIN_LED_G, LOW);
            break;
        default:
            break;
    }
}