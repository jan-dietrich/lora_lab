#include "main.h"

// Local logging Tag
static const char TAG[] = "wifi";

// Set these to your desired credentials.
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWD;

AsyncWebServer server(80);

//data that is send to lora
uint8_t mydata[] = "Hello, world!";

//handle for RTOS LMIC task
TaskHandle_t pvLMICTask = NULL;

//open up websocket to handle JavaScript requests from buttons
AsyncWebSocket ws("/socket");
AsyncWebSocketClient * globalClient = NULL;

String webpage_log[MAX_LOG_NUMBER + 1]; //stores the data that is displayed on the webpage
String webpage_log_out;
int log_counter = 0;

void wifi_initialize(){
  //disable the core 0 watchdog
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;

  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  #if LOG_LEVEL > 2
  Serial.printf("%s:AP IP address: ",TAG);
  Serial.println(myIP);
  #endif

  //register handler
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.begin();

  #if LOG_LEVEL > 2
  Serial.printf("%s:Server started\n",TAG);
  #endif
  
  //initialize SPIFFS
  if(!SPIFFS.begin(true)){
  #if LOG_LEVEL > 0
  Serial.printf("%s:An Error has occurred while mounting SPIFFS",TAG);
  #endif
  return;
  }

  //define get requests for webserver
  //main webpage
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/index.html", "text/html");
  });
  //other pages
  server.on("/pages/data.html", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/pages/data.html", "text/html");
  });
  server.on("/pages/home.html", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/pages/home.html", "text/html");
  });
  server.on("/pages/keys.html", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/pages/keys.html", "text/html");
  });
  server.on("/pages/lmic.html", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/pages/lmic.html", "text/html");
  });
  server.on("/pages/parameter.html", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/pages/parameter.html", "text/html");
  });
  //jquery
  server.on("/jquery-3.3.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/jquery-3.3.1.min.js", "text/javascript");
  });
  //css stylesheet
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  //dhbw logo
  server.on("/img/dhbw_logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/img/dhbw_logo.png", "image/png");
  });
  //favicon
  server.on("/img/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/img/favicon.ico", "image/vnd.microsoft.icon");
  });

  //after all requests finally start webserver
  server.begin(); 
  wifi_setlog("Webserver gestartet");

  //prepare data for LoRa
  SendBuffer.MessageSize = sizeof(mydata)-1;
  SendBuffer.MessagePort = 1;
  memcpy(SendBuffer.Message, mydata, SendBuffer.MessageSize);

}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
 
  if(type == WS_EVT_CONNECT){
    Serial.printf("%s:JavaScript Event: WS_EVT_CONNECT\n",TAG);
    globalClient = client;
    display_update(0,(char*)"");
  }
  else if (type == WS_EVT_DATA){
    char webdata[WEB_BUFFER] = "";
    for (int i = 0; i < len;i++){
      webdata[i] = (char) data[i];
    }
    #if LOG_LEVEL > 2
    Serial.printf("%s:JavaScript Event: WS_EVT_DATA\n",TAG);
    #endif
    decode_webcmd(webdata);
  }
}

void wifi_setlog(String log){
String log_entry = String(log_counter) + ": " + log + "</br>";
for (int i = (MAX_LOG_NUMBER -1); i != 0; i--){
    webpage_log[i] = webpage_log[i-1];
}
webpage_log[0] = log_entry;

log_counter ++;
webpage_log_out = "data_log;";
for (int i = 0; i < MAX_LOG_NUMBER; i++){
        webpage_log_out += webpage_log[i];
}
updateWebpage(webpage_log_out);
}

void updateWebpage(String data){
if(globalClient != NULL && globalClient->status() == WS_CONNECTED){
      globalClient->text(data);
   }
}

void decode_webcmd(char * data){
char *ptr;

// initialisieren und ersten Abschnitt erstellen
ptr = strtok(data, ";");

#if LOG_LEVEL > 2
  Serial.printf("%s:Decoded webcmd %s\n",TAG, ptr);
#endif

if (0 == strcmp(ptr,"data_keys_abp")){
  char* web_NSK = strtok(NULL, ";");
  char* web_ASK = strtok(NULL, ";");
  char* web_DEVADDR = strtok(NULL, ";");

  //cast char* to unsigned int (aka u4_t)
  u4_t web_DEVADDR_int = (uint32_t)strtol(web_DEVADDR, NULL, 16);

  //convert NSK and ASK
  u1_t web_NSK_char[16];
  u1_t web_ASK_char[16];
  char buffer[2];

  for(int i = 0; i < 16; i++){
    buffer[0] = web_NSK[i*2];
    buffer[1] = web_NSK[i*2+1];
    web_NSK_char[i] = (uint8_t)strtol(buffer, NULL, 16);
    #if LOG_LEVEL > 3
    Serial.printf("%s:interpreted %c%c as %u\n",TAG,buffer[0],buffer[1],web_NSK_char[i]);
    #endif
  }
  for(int i = 0; i < 16; i++){
    buffer[0] = web_ASK[i*2];
    buffer[1] = web_ASK[i*2+1];
    web_ASK_char[i] = (uint8_t)strtol(buffer, NULL, 16);
    #if LOG_LEVEL > 3
    Serial.printf("%s:interpreted %c%c as %u\n",TAG,buffer[0],buffer[1],web_ASK_char[i]);
    #endif
  }

  #if LOG_LEVEL > 2
    Serial.printf("%s:NSK=%s\n",TAG,web_NSK);
    Serial.printf("%s:ASK=%s\n",TAG,web_ASK);
    Serial.printf("%s:DEVADDR=%s\n",TAG,web_DEVADDR);
  #endif
  display_update(1,(char*)"ABP");
  lora_setabpkeys(web_NSK_char, web_ASK_char, &web_DEVADDR_int);
}
else if (0 == strcmp(ptr,"data_keys_otaa")){
  char* web_APPEUI = strtok(NULL, ";");
  char* web_DEVEUI = strtok(NULL, ";");
  char* web_APPKEY = strtok(NULL, ";");

  //convert APPEUI DEVEUI and APPKEY
  u1_t web_APPEUI_char[8];
  u1_t web_DEVEUI_char[8];
  u1_t web_APPKEY_char[16];
  char buffer[2];

  for(int i = 0; i < 8; i++){
    buffer[0] = web_APPEUI[i*2];
    buffer[1] = web_APPEUI[i*2+1];
    web_APPEUI_char[i] = (uint8_t)strtol(buffer, NULL, 16);
    #if LOG_LEVEL > 3
    Serial.printf("%s:interpreted %c%c as %u\n",TAG,buffer[0],buffer[1],web_APPEUI_char[i]);
    #endif
  }
  for(int i = 0; i < 8; i++){
    buffer[0] = web_DEVEUI[i*2];
    buffer[1] = web_DEVEUI[i*2+1];
    web_DEVEUI_char[i] = (uint8_t)strtol(buffer, NULL, 16);
    #if LOG_LEVEL > 3
    Serial.printf("%s:interpreted %c%c as %u\n",TAG,buffer[0],buffer[1],web_DEVEUI_char[i]);
    #endif
  }
  for(int i = 0; i < 16; i++){
    buffer[0] = web_APPKEY[i*2];
    buffer[1] = web_APPKEY[i*2+1];
    web_APPKEY_char[i] = (uint8_t)strtol(buffer, NULL, 16);
    #if LOG_LEVEL > 3
    Serial.printf("%s:interpreted %c%c as %u\n",TAG,buffer[0],buffer[1],web_APPKEY_char[i]);
    #endif
  }

  #if LOG_LEVEL > 2
    Serial.printf("%s:APPEUI=%s\n",TAG,web_APPEUI);
    Serial.printf("%s:DEVEUI=%s\n",TAG,web_DEVEUI);
    Serial.printf("%s:APPKEY=%s\n",TAG,web_APPKEY);
  #endif
  display_update(1,(char*)"OTAA");
  lora_setotaakeys(web_APPEUI_char, web_DEVEUI_char, web_APPKEY_char);
}
else if (0 == strcmp(ptr,"data_lmic_abp")){
  lora_setmode(0);
  display_update(2,(char*)"-");
  xTaskCreatePinnedToCore(lora_initialize, "lora_initialize", 4096, NULL, 5, &pvLMICTask, 1);
}
else if (0 == strcmp(ptr,"data_lmic_otaa")){
  lora_setmode(1);
  display_update(2,(char*)"-");
  xTaskCreatePinnedToCore(lora_initialize, "lora_initialize", 4096, NULL, 5, &pvLMICTask, 1);
}
else if (0 == strcmp(ptr,"data_send")){
  //prepare data for LoRa
	SendBuffer.MessageSize = sizeof(mydata) - 1;
	SendBuffer.MessagePort = 1;
	memcpy(SendBuffer.Message, mydata, SendBuffer.MessageSize);

  lora_enqueuedata(&SendBuffer);
}
else if (0 == strcmp(ptr,"data_lmic_reset")){
  if( pvLMICTask != NULL )
    {
         vTaskDelete(pvLMICTask);
         display_update(1,(char*)"Nach Reset");
         #if LOG_LEVEL > 2
         Serial.printf("%s:LMIC Reset\n",TAG);
         #endif
         wifi_setlog("LMIC zur&uumlckgesetzt!");
    }
  else {
    #if LOG_LEVEL > 0
      Serial.printf("%s:LMIC could not be reset",TAG);
    #endif
    wifi_setlog("LMIC konnte nicht zur&uumlckgesetzt werden!");
  }
}
else if (0 == strcmp(ptr,"data_decode")){
  char* decode = strtok(NULL, ";");
  if (0 == strcmp(decode,"setDecodeRaw")){set_decode_mode(0);}
  else if (0 == strcmp(decode,"setDecodeASCII")){set_decode_mode(1);}
}
}
