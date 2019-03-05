#include "main.h"

// Local logging Tag
static const char TAG[] = "wifi";

// Set these to your desired credentials.
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWD;

//define variables for state
String state_lmic = "Nicht gestartet";
String state_lmicmode = "Noch nicht gesetzt";
String state_sf = "SF7";
String state_bw = "125kHz";
String state_cr = "4/5";

AsyncWebServer server(80);

//open up websocket to handle JavaScript requests from buttons
AsyncWebSocket ws("/socket");
AsyncWebSocketClient * globalClient = NULL;

String webpage_log[MAX_LOG_NUMBER + 1]; //stores the data that is displayed on the webpage
String webpage_log_out;
int log_counter = 0;

//Data to send via LoRa
uint8_t mydata[] = "Hello, world!";

String processor(const String& var){
String state;
  if(var == "state_lmic") state = state_lmic;
  else if (var == "state_lmicmode") state = state_lmicmode;
  else if (var == "state_sf") state = state_sf;
  else if (var == "state_bw") state = state_bw;
  else if (var == "state_cr") state = state_cr;
  return state;
}

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
  request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  //css stylesheet
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  //dhbw logo
  server.on("/img/dhbw_logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/img/dhbw_logo.png", "image/png");
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
    Serial.printf("%s:Websocket connected!\n",TAG);
    globalClient = client;
  }
  else if (type == WS_EVT_DATA){
    #if LOG_LEVEL > 2
    Serial.printf("%s:JavaScript Event: WS_EVT_DATA\n",TAG);
    for (int i = 0; i < len;i++){
      Serial.print((char) data[i]);
    }
    Serial.printf("\n");
    #endif
  }
}

void wifi_setlog(String log){
String log_entry = String(log_counter) + ": " + log + "</br>";
for (int i = (MAX_LOG_NUMBER -1); i != 0; i--){
    webpage_log[i] = webpage_log[i-1];
}
webpage_log[0] = log_entry;

log_counter ++;
webpage_log_out = "#log#";
for (int i = 0; i < MAX_LOG_NUMBER; i++){
        webpage_log_out += webpage_log[i];
}
if(globalClient != NULL && globalClient->status() == WS_CONNECTED){
      globalClient->text(webpage_log_out);
   }
}