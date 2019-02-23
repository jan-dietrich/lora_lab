#ifndef _LORAWAN_H
#define _LORAWAN_H

#include "main.h"

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

// initializes the lora module and sets the correct channels
void lora_initialize(void * parameter);

// is used to activate abp mode and set the correct keys
void lora_setabpkeys(u1_t NSK[16], u1_t ASK[16], u4_t DEVADDR);

// is used to activate otaa mode and set the correct keys
void lora_setotaakeys(u1_t APPEUI[8], u1_t DEVEUI[8], u1_t APPKEY[16]);

// gets called on event from LMIC
// name can not be changed as LMIC is not working otherwise
void onEvent (ev_t ev);

// starts uplink to network server. Should not be called. Instead use lora_enquedata
void lora_send(osjob_t* j);

//enque data into uplink que
void lora_enqueuedata(MessageBuffer_t *message);

#endif