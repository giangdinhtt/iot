/**
 * Arduino part for Hydroponic automation system
 */
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include "Nano.h"

// Determine usage of LCD
#define USE_DIPLAY true

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);  // set the LCD address to 0x3f for a 16 chars and 2 line display

/*
 * DHT sensor
 */
DHT controllerSensor(DHT11_PIN, DHT11_TYPE);

DHT environmentSensor(DHT21_PIN, DHT21_TYPE);

/*
 * DS18b20 sensor
 */
//Thiết đặt thư viện onewire
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature waterSensor(&oneWire);

/*
 * Ultrasonic sensor
 */
unsigned long duration; // biến đo thời gian
int distance;           // biến lưu khoảng cách
char distanceChar[8];

/*
 * Init json serialize
 */
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();
JsonObject& commandRoot;

String float2String(float f)
{
  char buffer[6];
  dtostrf(f, 2, 1, buffer);
  return String(buffer);
}

void setupPins()
{
  //********** FOR ESP8266 NodeMCU: CHANGE PIN FUNCTION  TO GPIO **********
  #if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32) //ESP8266 or ESP32
    // D9/GPIO1 (TX) swap the pin to a GPIO.
    pinMode(D9, FUNCTION_3); 
    // D10/GPIO3 (RX) swap the pin to a GPIO.
    pinMode(D10, FUNCTION_3);
  #endif

  #if defined(SDA_PIN) && defined(SLK_PIN)
    // Setup I2C for LCD
    Wire.begin(SDA_PIN, SLK_PIN);
  #endif
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

void initRelays(void)
{
  // Relay triggered as LOW, set HIGH level to turn them OFF
  for(int i = 0; i < 4; i++ ) {
    digitalWrite(RELAY_PINS[i], HIGH);
  }
}

void initAccessPoint()
{}

//DS18b20 sensor
float readWaterTemperature()
{
  waterSensor.requestTemperatures();
  float v = waterSensor.getTempCByIndex(0);

  if (USE_DIPLAY)
  {
    char buffer[4];
    dtostrf(v, 2, 1, buffer);
    sprintf(buffer,"%s", buffer);
    lcd.setCursor(0, 0);
    lcd.print("wa:");
    lcd.print(buffer);
  }
  return v;
}

//DHT11
float readControllerTemperature()
{
  float v = controllerSensor.readTemperature();

  if (USE_DIPLAY)
  {
    char buffer[4];
    dtostrf(v, 2, 1, buffer);
    sprintf(buffer, "%s", buffer);
    lcd.setCursor(0, 0);
    lcd.print("ct:");
    lcd.print(buffer);
  }
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

  if (USE_DIPLAY)
  {
    char buffer[4];
    dtostrf(v, 2, 1, buffer);
    sprintf(buffer, "%s", buffer);
    lcd.setCursor(8, 0);
    lcd.print("en:");
    lcd.print(buffer);
  }
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
  int v = int(duration*0.343/2/10);  // in cm

  if (USE_DIPLAY)
  {
    char buffer [8];
    sprintf(buffer, "%2d cm", v);
    lcd.setCursor(0, 1);
    lcd.print("lv:");
    lcd.print(buffer);
  }
  return v;
}

void waitForCommands()
{
  char json[] = "{\"pump.tank\":1,\"pump.sprinkle\":0,\"pump.mixer_1\":1,\"pump.mixer_2\":0}";
  commandRoot = jsonBuffer.parseObject(json);
  if (!commandRoot.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  commandRoot.printTo(Serial);
  lcd.setCursor(8, 1);
  lcd.print("        ");
  lcd.setCursor(8, 1);
  if (commandRoot.containsKey("pump.tank"))
  {
    digitalWrite(PUMP_TANK_PIN, !commandRoot["pump.tank"]);
    lcd.print("s1");
  }
  if (commandRoot.containsKey("pump.sprinkle"))
  {
    digitalWrite(PUMP_SPRINKLE_PIN, !commandRoot["pump.sprinkle"]);
    lcd.print("s2");
  }
  if (commandRoot.containsKey("pump.mixer_1"))
  {
    digitalWrite(PUMP_MIXER_1_PIN, !commandRoot["pump.mixer_1"]);
    lcd.print("s3");
  }
  if (commandRoot.containsKey("pump.mixer_2"))
  {
    digitalWrite(PUMP_MIXER_2_PIN, !commandRoot["pump.mixer_2"]);
    lcd.print("s4");
  }
}

void setup() {
  Serial.begin(9600);
  setupPins();
  if (USE_DIPLAY)
  {
    initLCD();
  }
  initAccessPoint();

  //initControllerSensor();
  initEnvironmentSensor();
  initWaterTemperatureSensor();
  initWaterTankLevel();
  initRelays();
}

void loop() {
  root["water.temp"] = readWaterTemperature();
  //root["ctrl.temp"] = readControllerTemperature();
  //root["ctrl.humid"] = readControllerHumidity();
  root["env.temp"] = readEnvironmentTemperature();
  root["env.humid"] = readEnvironmentHumidity();
  root["water.level"] = readDistance();

  root["pump.tank"] = !digitalRead(PUMP_TANK_PIN);
  root["pump.sprinkle"] = !digitalRead(PUMP_SPRINKLE_PIN);
  root["pump.mixer_1"] = !digitalRead(PUMP_MIXER_1_PIN);
  root["pump.mixer_2"] = !digitalRead(PUMP_MIXER_2_PIN);

  String output;
  root.printTo(output);
  
  Serial.println(output);
  //root.printTo(Serial);

  waitForCommands();

  delay(500);
}
