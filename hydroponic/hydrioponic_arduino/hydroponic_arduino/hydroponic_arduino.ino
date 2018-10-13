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
const int DHT11_PIN = 2;       //Đọc dữ liệu từ DHT11 ở chân 2 trên mạch Arduino
const int DHT11_TYPE = DHT11;  //Khai báo loại cảm biến:DHT11, DHT21 or AM2301, DHT22, 
DHT controllerSensor(DHT11_PIN, DHT11_TYPE);

const int DHT21_PIN = 4;       //Đọc dữ liệu từ DHT11 ở chân 2 trên mạch Arduino
const int DHT21_TYPE = DHT21;
DHT environmentSensor(DHT21_PIN, DHT21_TYPE);

/*
 * DS18b20 sensor
 */
// Chân nối với Arduino
#define ONE_WIRE_BUS 7
//Thiết đặt thư viện onewire
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature waterSensor(&oneWire);

/*
 * Ultrasonic sensor
 */
const int TRIG_PIN = 12;     // chân trig của HC-SR04
const int ECHO_PIN = 13;     // chân echo của HC-SR04
unsigned long duration; // biến đo thời gian
int distance;           // biến lưu khoảng cách
char distanceChar[8];

/*
 * Init json serialize
 */
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

String float2String(float f)
{
  char buffer[6];
  dtostrf(f, 2, 1, buffer);
  return String(buffer);
}

void initLCD()
{
  lcd.init();       // initialize the lcd 
  lcd.backlight();
}

void initControllerSensor(void)
{
  controllerSensor.begin();         // Khởi động cảm biến
}

void initEnvironmentSensor(void)
{
  environmentSensor.begin();         // Khởi động cảm biến
}

void initWaterTemperatureSensor(void)
{
  waterSensor.begin();
}

void initWaterTankLevel()
{
  pinMode(TRIG_PIN, OUTPUT);   // chân trig sẽ phát tín hiệu
  pinMode(ECHO_PIN, INPUT);    // chân echo sẽ nhận tín hiệu
}

//DS18b20 sensor
float readWaterTemperature()
{
  waterSensor.requestTemperatures();
  float v = waterSensor.getTempCByIndex(0);

  char buffer[4];
  dtostrf(v, 2, 1, buffer);
  sprintf(buffer,"%s", buffer);
  lcd.setCursor(0, 1);
  lcd.print("wa:");
  lcd.print(buffer);
  
  return v;
}

//DHT11
float readControllerTemperature()
{
  float v = controllerSensor.readTemperature();

  char buffer[4];
  dtostrf(v, 2, 1, buffer);
  sprintf(buffer, "%s", buffer);
  lcd.setCursor(0, 0);
  lcd.print("ct:");
  lcd.print(buffer);
  
  return v;
}

//DHT11
float readControllerHumidity()
{
  float v = controllerSensor.readHumidity();

  //char buffer[4];
  //dtostrf(v, 1, 1, buffer);
  //sprintf(buffer, "%s", buffer);
  //lcd.setCursor(4, 0);
  //lcd.print(buffer);

  return v;
}

//DHT21
float readEnvironmentTemperature()
{
  float v = environmentSensor.readTemperature();

  char buffer[4];
  dtostrf(v, 2, 1, buffer);
  sprintf(buffer, "%s", buffer);
  lcd.setCursor(8, 0);
  lcd.print("en:");
  lcd.print(buffer);

  return v;
}

//DHT21
float readEnvironmentHumidity()
{
  float v = environmentSensor.readHumidity();

  //char buffer[4];
  //dtostrf(v, 1, 1, buffer);
  //sprintf(buffer, "%s", buffer);
  //lcd.setCursor(12, 0);
  //lcd.print(buffer);

  return v;
}

int readDistance()
{
  /* Phát xung từ chân trig */
  digitalWrite(TRIG_PIN, LOW);  // tắt chân trig
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH); // phát xung từ chân trig
  delayMicroseconds(10);    // xung có độ dài 5 microSeconds
  digitalWrite(TRIG_PIN, LOW);  // tắt chân trig
  
  /* Tính toán thời gian */
  // Đo độ rộng xung HIGH ở chân echo. 
  duration = pulseIn(ECHO_PIN, HIGH);  
  // Tính khoảng cách đến vật
  int v = int(duration*0.343/2);  // in mm

  char buffer [8];
  sprintf(buffer, "%3d mm", v);
  lcd.setCursor(8, 1);
  lcd.print(buffer);

  return v;
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
  initControllerSensor();
  initEnvironmentSensor();
  initWaterTemperatureSensor();
  initWaterTankLevel();
  initLCD();
}

void loop() {
  root["water.temp"] = readWaterTemperature();
  root["ctrl.temp"] = readControllerTemperature();
  root["ctrl.humid"] = readControllerHumidity();
  root["env.temp"] = readEnvironmentTemperature();
  root["env.humid"] = readEnvironmentHumidity();
  root["water.level"] = readDistance();
  root.printTo(Serial);
  //Serial.print('\n');

  //char buffer [50];
  //sprintf(buffer, "%3d mm", readDistance());
  //lcd.setCursor(0, 1);
  //lcd.print(buffer);

  //executeCommands();

  delay(500);
}
