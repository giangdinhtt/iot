#ifndef HYDROPONIC_ARDUINO_H
#define HYDROPONIC_ARDUINO_H

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <DoubleResetDetect.h>

// Serial baud rate
#define BAUD_RATE 9600

// LCD address
#define LCD_ADDRESS 0x3f
// set the LCD address to 0x3f for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);  

// Determine usage of LCD
#define USE_DIPLAY true

// Reading sensors interval in milliseconds
#define READ_INTERVAL 10000

/**
* Press RST button 2 times cause access point reset
*/
bool hard_reset_required = false;

// maximum number of seconds between resets that counts as a double reset 
#define DRD_TIMEOUT 2.0

// address to the block in the RTC user memory
// change it if it collides with another usage 
// of the address block
#define DRD_ADDRESS 0x00

DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);
   
int tank_height = 50;  // water tank's height (in cm)

#endif
