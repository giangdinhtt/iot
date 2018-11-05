#ifndef HYDROPONIC_ARDUINO_H
#define HYDROPONIC_ARDUINO_H

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Serial baud rate
#define BAUD_RATE 9600
// LCD address
#define LCD_ADDRESS 0x3f
// set the LCD address to 0x3f for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);  

// Determine usage of LCD
#define USE_DIPLAY true

#endif
