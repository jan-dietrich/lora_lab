/* ----------------- Display Module -----------------

    Copyright 2019 Jan Dietrich

    Component for controlling the display and showing 
    graphics or information

    Changelog:
    V1.0    Initial release

*/

// Basic Config
#include "main.h"

//definition for oled display
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

// Tag for debugging output
static const char TAG[] = "oled";

void display_initialize(){
    u8x8.begin();
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    char wifi_ssid[15] = "SSID:";
    strcat(wifi_ssid, WIFI_SSID);
    char wifi_passwd[15] = "PASS:";
    strcat(wifi_passwd, WIFI_PASSWD);
    u8x8.drawString(0, 0, (char*)wifi_ssid);
    u8x8.drawString(0, 1, (char*)wifi_passwd);
    u8x8.drawString(0, 3, "IP aufrufen:");
    u8x8.drawString(0, 4, "192.168.4.1");
    #if LOG_LEVEL > 2
        Serial.printf("%s: Display initialized\n",TAG);
    #endif
}

void display_update(int display_mode, char data[15]){
    u8x8.clearDisplay();
    #if LOG_LEVEL > 2
        Serial.printf("%s: Display set to %i\n",TAG,display_mode);
    #endif
    switch (display_mode)
    {
        //user connected
        case 0:
            u8x8.drawString(0, 0, "Nutzer verbunden");
            u8x8.drawString(0, 2, "1. Keys setzen");
            u8x8.drawString(0, 3, "2. LMIC laden");
            u8x8.drawString(0, 4, "3. Daten senden");
            break;
        //keys set
        case 1:
            u8x8.drawString(0, 0, "Modus:");
            u8x8.drawString(0, 1, (char*)data);
            u8x8.drawString(0, 3, "-> LMIC laden");
            break;
        //lmic loaded, ready to send
        case 2:
            u8x8.drawString(0, 0, "LMIC bereit");
            u8x8.drawString(0, 3, "Letztes Event:");
            u8x8.drawString(0, 4, (char*)data);
            break;
        default:
            u8x8.drawString(0, 0, "Fehler");
            break;
    }
    
}