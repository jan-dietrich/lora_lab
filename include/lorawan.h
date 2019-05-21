#ifndef _LORAWAN_H
#define _LORAWAN_H

// needed for the LMIC lib
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

extern QueueHandle_t LoraSendQueue;

// Struct holding payload for data send queue
typedef struct {
  uint8_t MessageSize;
  uint8_t MessagePort;
  uint8_t Message[SEND_BUFFER_SIZE];
} MessageBuffer_t;

//defines the mode lmic is started with
//default is ABP
void lora_setmode(int mode);

// initializes the lora module and sets the correct channels
void lora_initialize(void * parameter);

// is used to activate abp mode and set the correct keys
void lora_setabpkeys(u1_t* web_NSK, u1_t* web_ASK, u4_t* web_DEVADDR);

// is used to activate otaa mode and set the correct keys
void lora_setotaakeys(u1_t* web_APPEUI, u1_t* web_DEVEUI, u1_t* web_APPKEY);

// gets called on event from LMIC
// name can not be changed as LMIC is not working otherwise
void onEvent(ev_t ev);

// starts uplink to network server. Should not be called. Instead use lora_enquedata
void lora_send(osjob_t* j);

//enque data into uplink que
void lora_enqueuedata(MessageBuffer_t *message, int isr =0);

//change spreading factor
void switch_sf(uint8_t sf);

#endif