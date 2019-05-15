#ifndef _DATA_H
#define _DATA_H

//Pin definitions for the hardware
#define PIN_LED_R       17
#define PIN_LED_G       12
#define PIN_LED_Y       13

#define PIN_BTN_1       39
#define PIN_BTN_2       38
#define PIN_BTN_3       37

#define PIN_TEMP_IN     36

#define PIN_I2C_SDA     21 
#define PIN_I2C_SCL     22

//include other header files

//temp sensor lib
#include <OneWire.h>
#include <DallasTemperature.h>

//prepares the hardware to be userd
void hardware_init();

//method to set the boar leds
void setLEDs(int i, bool va);

//interrupt routines
void isr_btn_1();
void isr_btn_2();
void isr_btn_3();

//read temperature
int read_temperature();

//enable and disable btn interrupts
void enable_btns(bool state);

//sends btn data via lora 
void btn_pressed(int i);
#endif