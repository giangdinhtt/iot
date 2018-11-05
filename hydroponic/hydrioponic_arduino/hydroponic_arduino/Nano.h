/**
 * Pins definitions for Arduino Nano v3
 */
#ifndef Nano_H
#define Nano_H

// 1-wire bus (for DS18b20)
#define ONE_WIRE_BUS 2

// Environment temperature/humidity sensor
#define DHT21_PIN 3
#define DHT21_TYPE DHT21

// Controller temperature/humidity sensor
#define DHT11_PIN 4
#define DHT11_TYPE DHT11  //DHT11, DHT21 or AM2301, DHT22

// Ultrasonic sensor: HC-SR04, JSN-SR04T-2.0
#define TRIG_PIN 5
#define ECHO_PIN 6

// Relay pins
#define SWITCH_1 D6
#define SWITCH_2 D7
#define SWITCH_3 D8
#define SWITCH_4 D9

#endif
