/**
 * Pins definitions for ESP8266 NodeMCU LoLin
 */
#ifndef ESP8266_H
#define ESP8266_H

#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

// Serial baud rate
#undef BAUD_RATE
#define BAUD_RATE 115200

// LCD address
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

/**
 * 
 */
#define AP_NAME "HydroponicAP" // Assigns your Access Point name
#define AP_PASSWORD "12345678" // Assigns your Access Point name
#define MQTT_SERVER "mqtt.giang.xyz"
#define MQTT_PORT 1884
#define MQTT_CLIENT_ID "iot.hydroponic.esp8266"

#endif
