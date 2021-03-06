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

//for wifi log
String slog;

static osjob_t sendjob;
QueueHandle_t LoraSendQueue;
MessageBuffer_t SendBuffer; // contains MessageSize, MessagePort, Message[]

// Keys for abp
u1_t NSK[16] = { }; //network session key
u1_t ASK[16] = {  }; //application session key
u4_t DEVADDR = 0; //device addresss

//keys for otaa
u1_t APPEUI[8]={  };  //application eui
u1_t DEVEUI[8]={  };  //device eui
u1_t APPKEY[16] = {  };   //application key
// must be defined even if abp is used
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

//default mode is abp
int lmic_mode = 0;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = LORA_CS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LORA_RST,
    .dio = {LORA_DIO0, LORA_DIO1, LORA_DIO2},
};

void lora_setabpkeys(u1_t* web_NSK, u1_t* web_ASK, u4_t* web_DEVADDR){
    #if LOG_LEVEL > 3
        Serial.printf("%s:NSK before memcpy set to: ", TAG);
        for (int i=0;i < 16; i++){
            Serial.printf("%u ", NSK[i]);
        }
        Serial.printf("\n");
        Serial.printf("%s:ASK before memcpy set to: ", TAG);
        for (int i=0;i < 16; i++){
            Serial.printf("%u ", ASK[i]);
        }
        Serial.printf("\n");
        Serial.printf("%s:DEVADDR before memcpy set to: %u", TAG, DEVADDR);
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

void lora_setotaakeys(u1_t* web_APPEUI, u1_t* web_DEVEUI, u1_t* web_APPKEY){
    #if LOG_LEVEL > 3
        Serial.printf("%s:APPEUI before memcpy set to: ", TAG);
        for (int i=0;i < 8; i++){
            Serial.printf("%u ", APPEUI[i]);
        }
        Serial.printf("\n");
        Serial.printf("%s:DEVEUI before memcpy set to: ", TAG);
        for (int i=0;i < 8; i++){
            Serial.printf("%u ", DEVEUI[i]);
        }
        Serial.printf("\n");
        Serial.printf("%s:APPKEY before memcpy set to: ", TAG);
        for (int i=0;i < 16; i++){
            Serial.printf("%u ", APPKEY[i]);
        }
        Serial.printf("\n");
    #endif

    for (int i=0;i <8; i++){
        APPEUI[i]=web_APPEUI[i];
    }
    for (int i=0;i <8; i++){
        DEVEUI[i]=web_DEVEUI[i];
    }
    for (int i=0;i <16; i++){
        APPKEY[i]=web_APPKEY[i];
    }
    #if LOG_LEVEL > 2
        Serial.printf("%s:APPEUI after memcpy set to: ", TAG);
        for (int i=0;i < 8; i++){
            Serial.printf("%u ", APPEUI[i]);
        }
        Serial.printf("\n");
        Serial.printf("%s:DEVEUI after memcpy set to: ", TAG);
        for (int i=0;i < 8; i++){
            Serial.printf("%u ", DEVEUI[i]);
        }
        Serial.printf("\n");
        Serial.printf("%s:APPKEY after memcpy set to: ", TAG);
        for (int i=0;i < 16; i++){
            Serial.printf("%u ", APPKEY[i]);
        }
        Serial.printf("\n");
    #endif
    wifi_setlog("OTAA Keys gesetzt");
}

void lora_setmode(int mode) {lmic_mode = mode;}

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

    if (lmic_mode == 0){
    updateWebpage("data_state_lmicmode;ABP Modus");
    //Set session
    if ((NSK != 0 ) & (ASK != 0) & (DEVADDR != 0)){
    LMIC_setSession (0x1, DEVADDR, NSK, ASK);
    #if LOG_LEVEL > 2 
        Serial.printf("%s:Set LMIC session parameters successfully\n",TAG);
        Serial.printf("%S:Network Session Key is %u\n", TAG, NSK);
        Serial.printf("%S:Application Session Key is %u\n", TAG, ASK);
        Serial.printf("%S:Device Address is %u\n", TAG, DEVADDR);

    #endif
    }
    }
    
    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // disable automatic datarate
    LMIC_setAdrMode (0);

    // TTN uses SF9 for its RX2 window. Is defined by TTN and must not be changed
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);
    updateWebpage("data_state_sf;SF7");
    updateWebpage("data_state_bw;125kHz");
    updateWebpage("data_state_cr;4/5");

    if (lmic_mode == 1){
        updateWebpage("data_state_lmicmode;OTAA Modus");
        if (!LMIC_startJoining()) { // start joining
        #if LOG_LEVEL > 0
            Serial.printf("%s:already joined\n",TAG);
        #endif
        wifi_setlog("Bereits gejoined");
        }
    }

    updateWebpage("data_state_lmic;Initialisiert");
    setLEDs(3,false);
    //call lora_send once to enable scheduled data transfer
    lora_send(&sendjob);
    wifi_setlog("LMIC Bibliothek initialisiert");

    //determin how long the process shall be blocked every loop cycle
    const TickType_t xDelay = 2 / portTICK_PERIOD_MS;

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
  if (LMIC.opmode & OP_TXRXPEND) {
    // waiting for LoRa getting ready
    #if LOG_LEVEL > 1
        Serial.printf("%s: OP_TXRXPEND, could not send data to LoRa Gateway. LMIC is busy\n", TAG);
        Serial.printf("%s: Opcode is: %x\n", TAG, LMIC.opmode);
    #endif
    wifi_setlog("Daten konnten nicht gesendet werden, da LMIC noch arbeitet");
    display_update(2,(char*)"TXRXPEND -> RST");
  } else {
    #if LOG_LEVEL > 2
    Serial.printf("%s:Sendjob startet: Checking que\n", TAG);
    #endif
    if (xQueueReceive(LoraSendQueue, &SendBuffer, (TickType_t)0) == pdTRUE) {
      // SendBuffer now filled with next payload from queue
      LMIC_clrTxData(); //clear data that is still in buffer
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
            slog = (String)LMIC.dataLen + " Bytes empfangen";
            wifi_setlog(slog);
            slog = "RSSI: " + (String)LMIC.rssi + " SNR: " + (String)LMIC.snr;
            wifi_setlog(slog);
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_TXCOMPLETE (includes waiting for RX windows)\n", TAG);
                if (LMIC.txrxFlags & TXRX_ACK)
                    Serial.printf("%s:Received ack\n",TAG);
                Serial.printf("%s:Received ",TAG);
                Serial.printf("%d",LMIC.dataLen);
                Serial.printf(" byte(s) of payload, RSSI %d SNR %d\n", LMIC.rssi, LMIC.snr);
                if (LMIC.dataLen) { //if valid data is received
                    decode_message(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
                }
            #endif
            wifi_setlog("Event empfangen: TX abgeschlossen");
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
        case EV_TXSTART:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is EV_TXSTART\n", TAG);
            #endif
            wifi_setlog("Event empfangen: TX begonnen");
            display_update(2,(char*)"EV_TXSTART");
            break; 
         default:
            #if LOG_LEVEL > 2
                Serial.printf("%s:Event is unknown: ", TAG);
                Serial.println((unsigned) ev);
            #endif
            wifi_setlog("Unbekanntes Event empfangen");
            display_update(2,(char*)"UNKNOWN");
            break;
    }
}

void lora_enqueuedata(MessageBuffer_t *message, int isr) {
  // enqueue message in LORA send queue
  BaseType_t ret;
    ret = xQueueSendToBack(LoraSendQueue, (void *)message, (TickType_t)0);

  if (ret == pdTRUE) {
      #if LOG_LEVEL > 2
      Serial.printf("%s: %d bytes enqueued for LORA interface\n", TAG, message->MessageSize);
      #endif
      if (isr == 0){
      wifi_setlog("Daten zur Warteschlange hinzugef&uumlgt");
      }
  } else {
      #if LOG_LEVEL > 1
      Serial.printf("%s:nLoRa sendque is full\n", TAG);
      #endif
  }
}

// helper function to assign LoRa datarates to numeric spreadfactor values
void switch_sf(uint8_t sf) {
uint8_t tx = 14;  
#if LOG_LEVEL > 2
    Serial.printf("%s: Switched SF to ", TAG);
    Serial.println(sf);
#endif
slog = "Speizfaktor gesetzt: " + (String)sf;
wifi_setlog(slog);
slog = "data_state_sf;SF" + sf;
updateWebpage(slog);

switch (sf) {
    case 7:
        LMIC_setDrTxpow(DR_SF7, tx);
    break;
    case 8:
        LMIC_setDrTxpow(DR_SF8, tx);
    break;
    case 9:
        LMIC_setDrTxpow(DR_SF9, tx);
    break;
    case 10:
        LMIC_setDrTxpow(DR_SF10, tx);
    break;
    case 11:
        #if defined(CFG_us915)
        LMIC_setDrTxpow(DR_SF11CR, tx);
    break;
        #else
        LMIC_setDrTxpow(DR_SF11, tx);
    break;
        #endif
    case 12:
        #if defined(CFG_us915)
        LMIC_setDrTxpow(DR_SF12CR, tx);
    break;
        #else
        LMIC_setDrTxpow(DR_SF12, tx);
    break;
        #endif
    default:
        #if LOG_LEVEL > 0
            Serial.printf("%s: Invalid SF chosen!",TAG);
        #endif
    break;
  }
}