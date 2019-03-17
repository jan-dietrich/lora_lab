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
u1_t NSK[16] = { 0xA7, 0x60, 0xE8, 0xCE, 0x11, 0xE3, 0x22, 0x5C, 0x2B, 0x22, 0xB5, 0x7F, 0x06, 0x3B, 0x6F, 0x1B }; //network session key
u1_t ASK[16] = { 0xA7, 0x59, 0xE8, 0xDE, 0x11, 0xE3, 0x22, 0x6C, 0x2B, 0x22, 0xB5, 0x9F, 0x06, 0x3B, 0xFF, 0x1B }; //application session key
u4_t DEVADDR = 0; //device addresss

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

void lora_setabpkeys(u1_t* web_NSK, u1_t* web_ASK, u4_t* web_DEVADDR){
    #if LOG_LEVEL > 3
        Serial.printf("%s:web_NSK before memcpy set to: ", TAG);
        for (int i=0;i < 16; i++){
            Serial.printf("%u ", web_NSK[i]);
        }
        Serial.printf("\n");
        Serial.printf("%s:web_ASK before memcpy set to: ", TAG);
        for (int i=0;i < 16; i++){
            Serial.printf("%u ", web_ASK[i]);
        }
        Serial.printf("\n");
    #endif
    
    for (int i=0;i <16; i++){
        NSK[i]=web_NSK[i];
    }
    for (int i=0;i <16; i++){
        ASK[i]=web_ASK[i];
    }
    DEVADDR = *web_DEVADDR;
    #if LOG_LEVEL > 2
        Serial.printf("%s:NSK after memcpy set to: ", TAG);
        for (int i=0;i < 16; i++){
            Serial.printf("%u ", NSK[i]);
        }
        Serial.printf("\n");
        Serial.printf("%s:ASK after memcpy set to: ", TAG);
        for (int i=0;i < 16; i++){
            Serial.printf("%u ", ASK[i]);
        }
        Serial.printf("\n");
        Serial.printf("%s:Device Address set to %u\n", TAG, DEVADDR);
    #endif
    wifi_setlog("ABP Keys gesetzt");
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
        Serial.printf("%S:Network Session Key is %u\n", TAG, NSK);
        Serial.printf("%S:Application Session Key is %u\n", TAG, ASK);
        Serial.printf("%S:Device Address is %u\n", TAG, DEVADDR);

    #endif
    } else {
        #if LOG_LEVEL > 0
            Serial.printf("%s:Could not set LMIC session parameters\n",TAG);
            Serial.printf("%S:Network Session Key is %u\n", TAG, NSK);
            Serial.printf("%S:Application Session Key is %u\n", TAG, ASK);
            Serial.printf("%S:Device Address is %u\n", TAG, DEVADDR);
        #endif
    }
    //LMIC_setSession (0x1, DEVADDR1, NWKSKEY, APPSKEY);

    // define additional channels
    
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    
    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // disable automatic datarate
    //LMIC_setAdrMode (0);

    // TTN uses SF9 for its RX2 window. Is defined by TTN and must not be changed
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);
    updateWebpage("data_state_sf;SF7");
    updateWebpage("data_state_bw;125kHz");
    updateWebpage("data_state_cr;4/5");

    //call lora_send once to enable scheduled data transfer
    lora_send(&sendjob);
    wifi_setlog("LMIC Bibliothek initialisiert");
    updateWebpage("data_state_lmic;Initialisiert");

    //determin how long the process shall be blocked every loop cycle
    const TickType_t xDelay = 1 / portTICK_PERIOD_MS;

    for (;;) {
        os_runloop_once();
        vTaskDelay(xDelay);
    }
    vTaskDelete(NULL); // shoud never be reached
}

void lora_send(osjob_t *job) {
  MessageBuffer_t SendBuffer;
  // Check if there is a pending TX/RX job running, if yes don't eat data
  // since it cannot be sent right now
  if ((LMIC.opmode & (OP_JOINING | OP_REJOIN | OP_TXDATA | OP_POLL)) != 0) {
    // waiting for LoRa getting ready
    #if LOG_LEVEL > 1
        Serial.printf("%s:OP_TXRXPEND, could not send data to LoRa Gateway. LMIC is busy\n", TAG);
        Serial.printf("%s:Opcode is: %x\n", TAG, LMIC.opmode);
    #endif
    wifi_setlog("Daten konnten nicht gesendet werden, da LMIC noch arbeitet");
  } else {
    #if LOG_LEVEL > 2
    Serial.printf("%s:Sendjob startet: Checking que\n", TAG);
    #endif
    if (xQueueReceive(LoraSendQueue, &SendBuffer, (TickType_t)0) == pdTRUE) {
      // SendBuffer now filled with next payload from queue
      LMIC_setTxData2(SendBuffer.MessagePort, SendBuffer.Message, SendBuffer.MessageSize, 0);
      wifi_setlog("Warteschlange an TTN gesendet");
        #if LOG_LEVEL > 2
        Serial.printf("%s:%d byte(s) sent to LoRa\n",TAG, SendBuffer.MessageSize);
        Serial.printf("%s:Transmitted message: %s\n",TAG,SendBuffer.Message);
        #endif
    }
    else {
        #if LOG_LEVEL > 1
            Serial.printf("%s:Warning! No entries in que\n",TAG);
        #endif
        wifi_setlog("Keine Daten in der Warteschlange");
    }
  }
  // reschedule job
  os_setTimedCallback(job, os_getTime()+sec2osticks(TX_INTERVAL) ,lora_send);
}

void onEvent(ev_t ev) {
    #if LOG_LEVEL > 2
    Serial.printf("%s:Event received @:%d\n",TAG,os_getTime());
    #endif
    
    // depending on event
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_SCAN_TIMEOUT\n", TAG);
            #endif
            wifi_setlog("Event empfangen: Timeout");
            display_update(2,(char*)"EV_SCAN_TIMEOUT");
            break;
        case EV_BEACON_FOUND:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_BEACON_FOUND\n", TAG);
            #endif
            wifi_setlog("Event empfangen: Beacon gefunden");
            display_update(2,(char*)"EV_BEACON_FOUND");
            break;
        case EV_BEACON_MISSED:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_BEACON_MISSED\n", TAG);
            #endif
            wifi_setlog("Event empfangen: Beacon verpasst");
            display_update(2,(char*)"EV_BEACON_MISSED");
            break;
        case EV_BEACON_TRACKED:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_BEACON_TRACKED\n", TAG);
            #endif
            wifi_setlog("Event empfangen: Beacon getracked");
            display_update(2,(char*)"EV_BEACON_TRACKED");
            break;
        case EV_JOINING:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_JOINING\n", TAG);
            #endif
            wifi_setlog("Event empfangen: Joining");
            display_update(2,(char*)"EV_JOINING");
            break;
        case EV_JOINED:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_JOINED\n", TAG);
            #endif
            wifi_setlog("Event empfangen: Joined");
            display_update(2,(char*)"EV_JOINED");
            break;
        case EV_RFU1:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_RFU1\n", TAG);
            #endif
            wifi_setlog("Event empfangen: RFU1");
            display_update(2,(char*)"EV_RFU1");
            break;
        case EV_JOIN_FAILED:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_JOIN_FAILED\n", TAG);
            #endif
            wifi_setlog("Event empfangen: Join fehlgeschlagen");
            display_update(2,(char*)"EV_JOIN_FAILED");
            break;
        case EV_REJOIN_FAILED:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_REJOIN_FAILED\n", TAG);
            #endif
            wifi_setlog("Event empfangen: Rejoin fehlgeschlagen");
            display_update(2,(char*)"EV_REJOIN_FAILED");
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
            wifi_setlog("Event empfangen: TX abgeschlossen (einschließlich warten auf RX Fenster)");
            decode_message(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
            display_update(2,(char*)"EV_TXCOMPLETE");
            break;
        case EV_LOST_TSYNC: 
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_LOST_TSYNC\n", TAG);
            #endif
            wifi_setlog("Event empfangen: TSYNC verloren");
            display_update(2,(char*)"EV_LOST_TSYNC");
            break;
        case EV_RESET:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_RESET\n", TAG);
            #endif
            wifi_setlog("Event empfangen: Reset");
            display_update(2,(char*)"EV_RESET");
            break;
        case EV_RXCOMPLETE:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_RXCOMPLETE\n", TAG);
            #endif
            wifi_setlog("Event empfangen: RX abgeschlossen");
            display_update(2,(char*)"EV_RXCOMPLETE");
            break;
        case EV_LINK_DEAD:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_LINK_DEAD\n", TAG);
            #endif
            wifi_setlog("Event empfangen: Link tot");
            display_update(2,(char*)"EV_LINK_DEAD");
            break;
        case EV_LINK_ALIVE:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_LINK_ALIVE\n", TAG);
            #endif
            wifi_setlog("Event empfangen: Link am Leben");
            display_update(2,(char*)"EV_LINK_ALIVE");
            break;
         default:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is unknown\n", TAG);
            #endif
            wifi_setlog("Unbekanntes Event empfangen");
            display_update(2,(char*)"UNKNOWN");
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
      wifi_setlog("Daten zur Warteschlange hinzugef&uumlgt");
  } else {
      #if LOG_LEVEL > 1
      Serial.printf("%s:LoRa sendque is full\n", TAG);
      #endif
  }
}