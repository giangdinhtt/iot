/**
 * Pins definitions for ESP8266 NodeMCU LoLin
 */
#ifndef Esp8266
#define Esp8266

// LCD address
#define LCD_ADDRESS 0x3f
#define SDA_PIN D3  // GPIO0
#define SLK_PIN D4  // GPIO2

// 1-wire bus (for DS18b20)
#define ONE_WIRE_BUS D2

// Environment temperature/humidity sensor
#define DHT21_PIN D5
#define DHT21_TYPE DHT21

// Controller temperature/humidity sensor
#define DHT11_PIN D2  // TODO: choose another pin
#define DHT11_TYPE DHT11  //DHT11, DHT21 or AM2301, DHT22

// Ultrasonic sensor: HC-SR04, JSN-SR04T-2.0
#define TRIG_PIN D0
#define ECHO_PIN D1

#endif
