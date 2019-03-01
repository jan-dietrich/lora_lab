#include "main.h"

// Local logging Tag
static const char TAG[] = "wifi";

// Set these to your desired credentials.
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWD;

WiFiServer server(80);

void wifi_setup() {

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
  while(1){
  wifi_polling();
  }
}

void wifi_polling() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    #if LOG_LEVEL > 2
    Serial.printf("%s:New Client\n",TAG);           // print a message out the serial port
    #endif
    String currentLine = "";                // make a String to hold incoming data from the client
    String html_header = "";
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        #if LOG_LEVEL > 3
        Serial.write(c);                    // print it out the serial monitor
        #endif
        html_header += c;

        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            //react to actions by user
            if (html_header.indexOf("GET /lora/startsendjob") >= 0) {
                #if LOG_LEVEL > 2
                Serial.printf("%s:Button pressed: /lora/startsendjob\n",TAG);
                #endif
            }

            // the content of the HTTP response follows the header:
            client.print(header);
            client.println("Den Button dr&uumlcken um Daten per LoRa zu senden! </br>");
            client.println("<a href=\"/lora/startsendjob\"><button class=\"button\">Daten senden</button></a>");
            client.print(footer);
            
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    #if LOG_LEVEL > 2
    Serial.printf("%s:Client disconnected\n",TAG);
    #endif
  }
}