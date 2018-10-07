/**
 * Arduino part for Hydroponic automation system
 */
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <DHT.h>

LiquidCrystal_I2C lcd(0x3f,16,2);  // set the LCD address to 0x3f for a 16 chars and 2 line display

/*
 * DHT sensor
 */
const int DHTPIN = 2;       //Đọc dữ liệu từ DHT11 ở chân 2 trên mạch Arduino
const int DHTTYPE = DHT11;  //Khai báo loại cảm biến, có 2 loại là DHT11 và DHT22
DHT dht(DHTPIN, DHTTYPE);

/*
 * DS18b20 sensor
 */
// Chân nối với Arduino
#define ONE_WIRE_BUS 7
//Thiết đặt thư viện onewire
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature oneWireSensor(&oneWire);

/*
 * Ultrasonic sensor
 */
const int trig = 12;     // chân trig của HC-SR04
const int echo = 13;     // chân echo của HC-SR04
unsigned long duration; // biến đo thời gian
int distance;           // biến lưu khoảng cách
char distanceChar[8];

/*
 * Init json serialize
 */
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

void initLCD()
{
  lcd.init();       // initialize the lcd 
  lcd.backlight();
}

void initDHT(void)
{
  dht.begin();         // Khởi động cảm biến
}

void initDS12B20(void)
{
  oneWireSensor.begin();
}

void initUltrasonicSensor()
{
  pinMode(trig,OUTPUT);   // chân trig sẽ phát tín hiệu
  pinMode(echo,INPUT);    // chân echo sẽ nhận tín hiệu
}

float readDS18B20Temperature()
{
  oneWireSensor.requestTemperatures();
  return oneWireSensor.getTempCByIndex(0);
}

float readDHTTemperature()
{
  return dht.readTemperature();
}

float readDHTHumidity()
{
  return dht.readHumidity();
}

int readDistance()
{
  /* Phát xung từ chân trig */
  digitalWrite(trig, LOW);  // tắt chân trig
  delayMicroseconds(5);
  digitalWrite(trig, HIGH); // phát xung từ chân trig
  delayMicroseconds(10);    // xung có độ dài 5 microSeconds
  digitalWrite(trig, LOW);  // tắt chân trig
  
  /* Tính toán thời gian */
  // Đo độ rộng xung HIGH ở chân echo. 
  duration = pulseIn(echo, HIGH);  
  // Tính khoảng cách đến vật
  return int(duration*0.343/2);  // in mm
}

void executeCommands()
{
  if (Serial.available() > 0) {
    // read the incoming byte:
    String incomingByte = Serial.readString();
    Serial.println(incomingByte);
    
    // say what you got:
    lcd.setCursor(0, 0);
    lcd.print(incomingByte);
  }
}

void setup() {
  Serial.begin(9600);
  initDHT();
  initDS12B20();
  initUltrasonicSensor();
  initLCD();
}

void loop() {
  root["DS12B20.temp"] = readDS18B20Temperature();
  root["DHT.temp"] = readDHTTemperature();
  root["DHT.humid"] = readDHTHumidity();
  root["water.level"] = readDistance();
  root.printTo(Serial);

  char buffer [50];
  sprintf(buffer, "%3d mm", readDistance());
  lcd.setCursor(0, 1);
  lcd.print(buffer);

  executeCommands();

  delay(500);
}
