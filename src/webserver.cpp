#include "main.h"

// Local logging Tag
static const char TAG[] = "wifi";

// Set these to your desired credentials.
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWD;

AsyncWebServer server(80);

String webpage_log[MAX_LOG_NUMBER + 1]; //stores the data that is displayed on the webpage
String webpage_log_out;
int log_counter = 0;

//Data to send via LoRa
uint8_t mydata[] = "Hello, world!";

String processor(const String& var){
  String sreturn = "";
  return sreturn;
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

void wifi_polling() {
 
}

void wifi_setlog(String log){
String log_entry = String(log_counter) + ": " + log + "</br>";
for (int i = (MAX_LOG_NUMBER -1); i != 0; i--){
    webpage_log[i] = webpage_log[i-1];
}
webpage_log[0] = log_entry;

log_counter ++;
webpage_log_out = "";
for (int i = 0; i < MAX_LOG_NUMBER; i++){
        webpage_log_out += webpage_log[i];
    }
}