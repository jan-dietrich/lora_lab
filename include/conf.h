// ----------------- DHBW LoRa Config File -----------------
// needs to be changed to match the requirements 

#ifndef _CONF_H
#define _CONF_G

// Name of the ESP
#define ESPNAME                         "DHBW LoRa"

// Version of software
#define PROGVERSION                     "1.0"

// Set the log level of device
// Possible levels:
//  ESP_LOG_ERROR = 1
//  ESP_LOG_WARN = 2
//  ESP_LOG_INFO = 3
//  ESP_LOG_DEBUG = 4
//  ESP_LOG_VERBOSE = 5
#define LOG_LEVEL                       5

// Pins for LoRa Component
// For detailed pin description visit GitHub of Arduino LMIC
#define LORA_CS                         18
#define LORA_RST                        14
#define LORA_DIO0                       26
#define LORA_DIO1                       33
#define LORA_DIO2                       32

// LoRa TX interval in seconds. Needs to be compliant to fair use policy. May become longer due to duty cycle restrictions
#define TX_INTERVAL                     45 

#define PAYLOAD_BUFFER_SIZE             50      // maximum size of payload block per transmit
#define SEND_QUEUE_SIZE                 10       // maximum number of messages in payload send queue [1 = no queue]
#define SEND_BUFFER_SIZE                50

#endif