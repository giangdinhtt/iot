/**
 * Pins definitions for Arduino Uno R3
 */
#ifndef Nano_R3
#define Nano_R3

// LCD address
#define LCD_ADDRESS 0x3f

// 1-wire bus (for DS18b20)
#define ONE_WIRE_BUS 7

// Environment temperature/humidity sensor
#define DHT21_PIN = 4;
#define DHT21_TYPE = DHT21;

// Controller temperature/humidity sensor
#define DHT11_PIN = 2;
#define DHT11_TYPE = DHT11;  //DHT11, DHT21 or AM2301, DHT22

// Ultrasonic sensor: HC-SR04, JSN-SR04T-2.0
#define TRIG_PIN = 12;
#define ECHO_PIN = 13;

#endif
