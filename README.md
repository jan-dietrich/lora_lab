# lora_lab
A simple LoRaWAN lab application that shall teach students to understand the importance of low power wide area radio networks. The board is based on a Heltec LoRa32 V2 and provides a webinterface where it can be controlled. 

The aim is to send and receive data via TTN and to interact with various sensors and actors placed on the lab board.

## Getting Started
These instructions will help you to set the project up and flash it on the used Heltec WiFi LoRa32 chip. For more information regarding the board see http://www.heltec.cn/project/wifi-lora-32/?lang=en.

### Prerequisites
What things you need to install the software and how to install them
- It is recommended to use the PlatformIO framework. There are multiple IDEs to use it with. A detailed description and instructions can be found here: https://platformio.org/platformio-ide
- It is also possible to use the Arduino IDE but is't not ideal for more complex projects. If you are using Arduino you have to install the used libraries by yourself. Also there will be no documentation.

### Used libraries
- MCCI LoRaWAN LMIC library in a version >=2.3.2 for the LoRa implementation
- U8g2 in a version >=2.25.7 to control the display
- ESP Async WebServer in a version >=1.2.0 for webserver implementation

### Installing
This will only be valid if using PlatformIO
- Clone this Git repository 
- Add the project folder to the workspace
- Open the conf.h file
- Change the settings you're not happy with. In most cases this should be only the name or the WiFi settings
- Plug in your Heltec WiFi LoRa32
- The webpage has to be flashed in the SPIFFS of the ESP. Therefore open the PlatformIO menu (the alien on the left) and press "upload file system"
- Compile and flash the sourcecode by pressing the arrow in the blue menu on the bottom. If necessary add the board to the PlatformIO boards by pressing the house in the same menu -> boards -> Heltec WiFi LoRa32

### Usage
This chapter gives an overview about how to use the lora_lab board
- tbc
