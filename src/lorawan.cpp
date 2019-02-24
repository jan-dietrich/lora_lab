/* ----------------- LoRaWAN Component -----------------

    Copyright 2019 Jan Dietrich

    This component is responsible for the lora communication
    with the TTN. It implements functions for sending & receiving 
    data as well as setting the session parameters.

    The LMIC library is used for LoRa communication

    Changelog:
    V1.0    Initial release

*/
#include "main.h"

// Local logging Tag
static const char TAG[] = "lora";

static osjob_t sendjob;
QueueHandle_t LoraSendQueue;
MessageBuffer_t SendBuffer; // contains MessageSize, MessagePort, Message[]

// Keys for abp
u1_t NSK[16] = "0"; //network session key
u1_t ASK[16] = "0"; //application session key
u4_t DEVADDR = 0; //device addresss

u1_t NWKSKEY[16] = { 0xA7, 0x59, 0xE8, 0xCE, 0x11, 0xE3, 0x22, 0x6C, 0x2B, 0x22, 0xB5, 0x7F, 0x06, 0x3B, 0xFF, 0x1B };
u1_t APPSKEY[16] = { 0xF7, 0x63, 0xDE, 0x9B, 0xD8, 0xBF, 0x2E, 0x25, 0xE1, 0xEE, 0xA4, 0xE1, 0x90, 0x2D, 0x04, 0x6F };
u4_t DEVADDR1 = 0x26011C08 ;

// Keys for otaa
// must be defined empty even if abp is used
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = LORA_CS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LORA_RST,
    .dio = {LORA_DIO0, LORA_DIO1, LORA_DIO2},
};

void lora_setabpkeys(u1_t NSK[16], u1_t ASK[16], u4_t DEVADDR){
    NSK = NSK;
    ASK = ASK;
    DEVADDR = DEVADDR;

    #if LOG_LEVEL > 3
        Serial.printf("%S:Network Session Key set to %u\n", TAG, NSK);
        Serial.printf("%S:Application Session Key set to %u\n", TAG, ASK);
        Serial.printf("%S:Device Address set to %u\n", TAG, DEVADDR);
    #endif
}

// initializes the lora module and sets the correct channels
void lora_initialize(void * parameter){
    LoraSendQueue = xQueueCreate(SEND_QUEUE_SIZE, sizeof(MessageBuffer_t));
    if (LoraSendQueue == 0) {
        #if LOG_LEVEL > 0 
            Serial.printf("%s:Could not create que. Aborting ...\n",TAG);
        #endif
    }
    #if LOG_LEVEL > 2
        Serial.printf("%s:LORA send queue created, size %d Bytes\n",TAG,SEND_QUEUE_SIZE * PAYLOAD_BUFFER_SIZE);
        Serial.printf("%s:Initializing LMIC instance\n", TAG);
    #endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100); // may be used if no downlink is received. Helps to correct bad clock

    //Set session
    if ((NSK != 0 ) & (ASK != 0) & (DEVADDR != 0)){
    LMIC_setSession (0x1, DEVADDR, NSK, ASK);
    #if LOG_LEVEL > 2 
        Serial.printf("%s:Set LMIC session parameters successfully\n",TAG);
    #endif
    } else {
        #if LOG_LEVEL > 0
            Serial.printf("%s:Could not set LMIC session parameters\n",TAG);
            Serial.printf("%S:Network Session Key is %u\n", TAG, NSK);
            Serial.printf("%S:Application Session Key is %u\n", TAG, ASK);
            Serial.printf("%S:Device Address is %u\n", TAG, DEVADDR);
        #endif
    }
    LMIC_setSession (0x1, DEVADDR1, NWKSKEY, APPSKEY);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window. Is defined by TTN and must not be changed
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    //call lora_send once to enable scheduled data transfer
    lora_send(&sendjob);

    for (;;) {
        os_runloop_once();
        vTaskDelay(1);
    }
}

void lora_send(osjob_t *job) {
  MessageBuffer_t SendBuffer;
  // Check if there is a pending TX/RX job running, if yes don't eat data
  // since it cannot be sent right now
  if (LMIC.opmode & OP_TXRXPEND) {
    // waiting for LoRa getting ready
    #if LOG_LEVEL > 1
        Serial.printf("%s:OP_TXRXPEND, could not send data to LoRa Gateway. LMIC is busy\n", TAG);
        Serial.printf("%s:Opcode is: %x\n", TAG, LMIC.opmode);
    #endif
  } else {
    #if LOG_LEVEL > 2
    Serial.printf("%s:Sendjob startet: Checking que\n", TAG);
    #endif
    if (xQueueReceive(LoraSendQueue, &SendBuffer, (TickType_t)0) == pdTRUE) {
      // SendBuffer now filled with next payload from queue
      LMIC_setTxData2(SendBuffer.MessagePort, SendBuffer.Message, SendBuffer.MessageSize, 0);
        #if LOG_LEVEL > 2
        Serial.printf("%s:%d byte(s) sent to LoRa\n",TAG, SendBuffer.MessageSize);
        Serial.printf("%s:Transmitted message: %s\n",TAG,SendBuffer.Message);
        #endif
    }
    else {
        #if LOG_LEVEL > 1
            Serial.printf("%s:Warning! No entries in que\n",TAG);
        #endif
    }
  }
  // reschedule job every 0,5 - 1 sec. including a bit of random to prevent
  // systematic collisions
  os_setTimedCallback(job, os_getTime()+sec2osticks(TX_INTERVAL) ,lora_send);
}

void onEvent (ev_t ev) {
    #if LOG_LEVEL > 2
    Serial.printf("%s:Event received @:%s",TAG,os_getTime());
    #endif

    // depending on event
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            break;
        case EV_LOST_TSYNC: 
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void lora_enqueuedata(MessageBuffer_t *message) {
  // enqueue message in LORA send queue
  BaseType_t ret;
    ret = xQueueSendToBack(LoraSendQueue, (void *)message, (TickType_t)0);

  if (ret == pdTRUE) {
      #if LOG_LEVEL > 2
      Serial.printf("%s:%d bytes enqueued for LORA interface\n", TAG, message->MessageSize);
      #endif
  } else {
      #if LOG_LEVEL > 1
      Serial.printf("%s:LoRa sendque is full\n", TAG);
      #endif
  }
}