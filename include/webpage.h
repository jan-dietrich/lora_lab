#ifndef _WEBPAGE_H
#define _WEBPAGE_H

#include "main.h"

//shall be displayed as header of the webpage
const char header[] PROGMEM = R"=====(
<HTML>
	<HEAD>
			<TITLE>DHBW LoRa</TITLE>
            <meta http-equiv="refresh" content="5; url=http://192.168.4.1">
	</HEAD>
<BODY>
)=====";

//must be displayed at the end of the webpage
const char footer[] PROGMEM = R"=====(
</BODY>
</HTML>
)=====";

#endif