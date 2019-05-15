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

//data for btns
uint8_t btn_data1[] = "BTN 1 pressed!";
uint8_t btn_data2[] = "BTN 2 pressed!";
uint8_t btn_data3[] = "BTN 3 pressed!";

//set up one wire for temp sensor
OneWire temp_oneWire(PIN_TEMP_IN);
DallasTemperature temp_sensor(&temp_oneWire);

void hardware_init(){
    //prepare the led pins
    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);
    pinMode(PIN_LED_Y, OUTPUT);

    //prepare the button pins
    pinMode(PIN_BTN_1, INPUT);
    pinMode(PIN_BTN_2, INPUT);
    pinMode(PIN_BTN_3, INPUT);
}

void setLEDs(int i, bool val){
    #if LOG_LEVEL > 2
        Serial.printf("%s: LEDs changed status\n",TAG);
    #endif
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

//interrupt routines
void isr_btn_1(){
    static unsigned long last_interrupt_time1 = 0;
    unsigned long interrupt_time1 = millis();
    //if interrupt comes faster than 200ms assume it's a bounce and ignore
    if (interrupt_time1 - last_interrupt_time1 > 200){
    btn_pressed(1);
    }
    Serial.printf("%s: Reseting interrupt time\n",TAG);
    last_interrupt_time1 = interrupt_time1;
}
void isr_btn_2(){
    static unsigned long last_interrupt_time2 = 0;
    unsigned long interrupt_time2 = millis();
    //if interrupt comes faster than 200ms assume it's a bounce and ignore
    if (interrupt_time2 - last_interrupt_time2 > 200){
    btn_pressed(2);
    }
    Serial.printf("%s: Reseting interrupt time\n",TAG);
    last_interrupt_time2 = interrupt_time2;
}
void isr_btn_3(){
    static unsigned long last_interrupt_time3 = 0;
    unsigned long interrupt_time3 = millis();
    //if interrupt comes faster than 200ms assume it's a bounce and ignore
    if (interrupt_time3 - last_interrupt_time3 > 200){
    btn_pressed(3);
    }
    Serial.printf("%s: Reseting interrupt time\n",TAG);
    last_interrupt_time3 = interrupt_time3;
}

//read temperature
int read_temperature(){
    int ireturn = 0;
    temp_sensor.requestTemperatures();
    #if LOG_LEVEL > 2
        Serial.printf("%s: Temp data requested from sensor\n",TAG);
    #endif
    ireturn = (int)temp_sensor.getTempCByIndex(0); //index 0 because there is only 1 sensor
    if (ireturn == 0){
        #if LOG_LEVEL > 0
            Serial.printf("%s: Reading temp data failed!\n", TAG);
        #endif
    }
    else {
    #if LOG_LEVEL > 2
        Serial.printf("%s: Measured temp is %i\n",TAG,ireturn);
    #endif
    }
    return ireturn;
}

//enable and disable btn interrupts
void enable_btns(bool state){
if (state) {
    //attach interrupts to buttons
    wifi_setlog("BTN Interrupts aktiviert");
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_1), isr_btn_1, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_2), isr_btn_2, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_3), isr_btn_3, FALLING);
    #if LOG_LEVEL > 2
        Serial.printf("%s: Btn interrupts enabled\n",TAG);
    #endif
}
else{
    wifi_setlog("BTN Interrupts deaktivert");
    detachInterrupt(digitalPinToInterrupt(PIN_BTN_1));
    detachInterrupt(digitalPinToInterrupt(PIN_BTN_2));
    detachInterrupt(digitalPinToInterrupt(PIN_BTN_3));
    #if LOG_LEVEL > 2
        Serial.printf("%s: Btn interrupts disabled\n",TAG);
    #endif
}
}

void btn_pressed(int i){
    #if LOG_LEVEL > 2
        Serial.printf("%s: Btn%i ISR started\n",TAG,i);
    #endif

    switch(i){
        case 1: 
            SendBuffer.MessageSize = sizeof(btn_data1) - 1;
	        SendBuffer.MessagePort = 1;
	        memcpy(SendBuffer.Message, &btn_data1, SendBuffer.MessageSize);
        break;
        case 2: 
            SendBuffer.MessageSize = sizeof(btn_data2) - 1;
	        SendBuffer.MessagePort = 1;
	        memcpy(SendBuffer.Message, &btn_data2, SendBuffer.MessageSize);
        break;
        case 3: 
            SendBuffer.MessageSize = sizeof(btn_data3) - 1;
	        SendBuffer.MessagePort = 1;
	        memcpy(SendBuffer.Message, &btn_data3, SendBuffer.MessageSize);
        break;
        default: 
        #if LOG_LEVEL > 0
            Serial.printf("%s: Unknown BTN event\n",TAG);
        #endif
        break;

    }

    #if LOG_LEVEL > 2 
        Serial.printf("%s: Enqueue BTN info\n",TAG);
    #endif
    lora_enqueuedata(&SendBuffer, 1);
}