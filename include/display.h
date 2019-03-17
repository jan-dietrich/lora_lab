#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <U8x8lib.h>

//initializes the display on start of ESP
void display_initialize();

//changes between different display modes which determine the displayed information
void display_update(int display_mode, char data[15]);

#endif