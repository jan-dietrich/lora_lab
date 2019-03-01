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
    LMIC_setClockError(MAX_CLOCK_ERROR * MAX_CLOCK_ERROR_PERCENTAGE / 100); // may be used if no downlink is received. Helps to correct bad clock

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
    wifi_setlog("LMIC Bibliothek initialisiert</br>");

    for (;;) {
        os_runloop_once();
        vTaskDelay(1);
    }
    vTaskDelete(NULL); // shoud never be reached
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
    wifi_setlog("Daten konnten nicht gesendet werden, da kein Downlink empfangen wurde</br>");
  } else {
    #if LOG_LEVEL > 2
    Serial.printf("%s:Sendjob startet: Checking que\n", TAG);
    #endif
    if (xQueueReceive(LoraSendQueue, &SendBuffer, (TickType_t)0) == pdTRUE) {
      // SendBuffer now filled with next payload from queue
      LMIC_setTxData2(SendBuffer.MessagePort, SendBuffer.Message, SendBuffer.MessageSize, 0);
      wifi_setlog("Warteschlange an TTN gesendet</br>");
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
  // reschedule job
  os_setTimedCallback(job, os_getTime()+sec2osticks(TX_INTERVAL) ,lora_send);
}

void onEvent(ev_t ev) {
    #if LOG_LEVEL > 2
    Serial.printf("%s:Event received @:%d\n",TAG,os_getTime());
    #endif
    wifi_setlog("Event empfangen</br>");

    // depending on event
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_SCAN_TIMEOUT\n", TAG);
            #endif
            break;
        case EV_BEACON_FOUND:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_BEACON_FOUND\n", TAG);
            #endif
            break;
        case EV_BEACON_MISSED:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_BEACON_MISSED\n", TAG);
            #endif
            break;
        case EV_BEACON_TRACKED:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_BEACON_TRACKED\n", TAG);
            #endif
            break;
        case EV_JOINING:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_JOINING\n", TAG);
            #endif
            break;
        case EV_JOINED:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_JOINED\n", TAG);
            #endif
            break;
        case EV_RFU1:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_RFU1\n", TAG);
            #endif
            break;
        case EV_JOIN_FAILED:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_JOIN_FAILED\n", TAG);
            #endif
            break;
        case EV_REJOIN_FAILED:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_REJOIN_FAILED\n", TAG);
            #endif
            break;
        case EV_TXCOMPLETE: 
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_TXCOMPLETE (includes waiting for RX windows)\n", TAG);
                if (LMIC.txrxFlags & TXRX_ACK)
                    Serial.printf("%s:Received ack\n",TAG);
                if (LMIC.dataLen) { //if valid data is received
                    Serial.printf("%s:Received ",TAG);
                    Serial.printf("%d",LMIC.dataLen);
                    Serial.printf(" byte(s) of payload, RSSI %d SNR %d\n", LMIC.rssi, LMIC.snr);
                }
            #endif
            decode_message(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
            break;
        case EV_LOST_TSYNC: 
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_LOST_TSYNC\n", TAG);
            #endif
            break;
        case EV_RESET:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_RESET\n", TAG);
            #endif
            break;
        case EV_RXCOMPLETE:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_RXCOMPLETE\n", TAG);
            #endif
            break;
        case EV_LINK_DEAD:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_LINK_DEAD\n", TAG);
            #endif
            break;
        case EV_LINK_ALIVE:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_LINK_ALIVE\n", TAG);
            #endif
            break;
         default:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is unknown\n", TAG);
            #endif
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
      wifi_setlog("Daten zur Warteschlange hinzugef&uumlgt</br>");
  } else {
      #if LOG_LEVEL > 1
      Serial.printf("%s:LoRa sendque is full\n", TAG);
      #endif
  }
}