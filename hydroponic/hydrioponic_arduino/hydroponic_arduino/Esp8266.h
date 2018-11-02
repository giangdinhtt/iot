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

// Relay pins
#define SWITCH_1 D6
#define SWITCH_2 D7
#define SWITCH_3 D9
#define SWITCH_4 D10
#define PUMP_TANK_PIN SWITCH_1
#define PUMP_SPRINKLE_PIN SWITCH_2
#define PUMP_MIXER_1_PIN SWITCH_3
#define PUMP_MIXER_2_PIN SWITCH_4
#define RELAY_PINS {SWITCH_1, SWITCH_2, SWITCH_3, SWITCH_4}

#endif
