#ifndef _WEBPAGE_H
#define _WEBPAGE_H

#include "main.h"
//for automatic refresh
//<meta http-equiv="refresh" content="5; url=http://192.168.4.1">

//shall be displayed as header of the webpage
const char header[] PROGMEM = R"=====(
<HTML>
	<HEAD>
			<TITLE>DHBW LoRa</TITLE>
            
	</HEAD>
<BODY>
<h1>DHBW LoRa Versuchsboard</h1>
<h2>Schritt 1: Anmeldedaten</h2>
Die Zugangsdaten m&uumlssen der TTN Console entnommen werden. Wichtig ist, dass die richtigen Zugangsdaten f&uumlr OTAA oder ABP gew&aumlhlt werden!</br>
<table>
<tr>
<td>
ABP
</td>
<td>
OTAA
</td>
</tr>
<tr>
<td>
MSB Sortierung! </br>
<input type="text" style="width:800px" name="NSK" maxlength="40" value="{ 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C }"></br>
MSB Sortierung! </br>
<input type="text" style="width:800px" name="ASK" maxlength="40" value="{ 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C }"></br>
MSB Sortierung! </br>
<input type="text" style="width:800px" name="DEVADDR" maxlength="20" value="03FF0001">
</td>
<td>
LSB Sortierung! </br>
<input type="text" style="width:800px" name="DEVEUI" maxlength="40" value="{ 0xF0, 0x57, 0xED, 0x3F, 0xBE, 0x42, 0xFD, 0x00 }"></br>
LSB Sortierung! </br>
<input type="text" style="width:800px" name="AEUI" maxlength="40" value="{ 0x91, 0x30, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 }"></br>
MSB Sortierung! </br>
<input type="text" style="width:800px" name="APPKEY" maxlength="20" value="{ 0x4A, 0x55, 0x77, 0x69, 0xE8, 0xDD, 0xB0, 0x34, 0x41, 0xAA, 0xCC, 0x66, 0xB0, 0x3B, 0xA5, 0x94 }">
</td>
</tr>
<tr>
<td>
<a href=/lora/setkeys/abp><button class=\"button\">ABP setzen</button></a>
</td>
<td>
<a href=/lora/setkeys/otaa><button class=\"button\">OTAA setzen</button></a>
</td>
</tr>
</table>
<h2>Schritt 2: LMIC Stack initialisieren</h2>
<a href=/lora/lmic/abp><button class=\"button\">LMIC ABP Modus</button></a><a href=/lora/lmic/otaa><button class=\"button\">LMIC OTAA Modus</button></a>
<h2>Schritt 3: Parameter f&uumlr die &Uumlbertragung</h2>
<h2>Schritt 4: Daten in die Warteschlange einreihen</h2>
<a href=/lora/enquedata><button class=\"button\">Daten senden</button></a>
<a href=/lora/reset><button class=\"button\">LMIC Reset</button></a>
</br>
</br>
<h2>Log</h2>
)=====";

//must be displayed at the end of the webpage
const char footer[] PROGMEM = R"=====(
</BODY>
</HTML>
)=====";

#endif