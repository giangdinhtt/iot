/*****************************************
 * Include Libraries
 ****************************************/
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ArduinoJson.h>
#include <ConfigManager.h>
#include <DoubleResetDetect.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include <ArduinoJson.h>
#include <DHT.h>
/****************************************
 * Define Constants
 ****************************************/
// maximum number of seconds between resets that
// counts as a double reset 
#define DRD_TIMEOUT 2.0

// address to the block in the RTC user memory
// change it if it collides with another usage 
// of the address block
#define DRD_ADDRESS 0x00

DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

#define AP_NAME "Hydroponic_AP" // Assigns your Access Point name
#define MQTT_SERVER "mqtt.giang.xyz"
#define MQTT_PORT 1884
#define MQTT_CLIENT_ID "iot.hydroponic.esp8266"
#define DEVICE_LABEL "my-device" // Assigns your Device Label
#define VARIABLE_LABEL "my-variable" // Assigns your Variable Label
#define BUILTIN_LED D4

// Determine usage of LCD
#define USE_DIPLAY true

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
#define DHT11_PIN D2
#define DHT11_TYPE DHT11  //DHT11, DHT21 or AM2301, DHT22

// Ultrasonic sensor: HC-SR04, JSN-SR04T-2.0
#define TRIG_PIN D0
#define ECHO_PIN D1

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

String float2String(float f)
{
  char buffer[6];
  dtostrf(f, 2, 1, buffer);
  return String(buffer);
}

void setupPins()
{
  //********** FOR ESP8266 NodeMCU: CHANGE PIN FUNCTION  TO GPIO **********
  //#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32) //ESP8266 or ESP32
    //GPIO 1 (TX) swap the pin to a GPIO.
    //pinMode(TRIG_PIN, FUNCTION_3); 
    //GPIO 3 (RX) swap the pin to a GPIO.
    //pinMode(ECHO_PIN, FUNCTION_3);

    // Setup I2C for LCD
    Wire.begin(SDA_PIN, SLK_PIN);
  //#endif
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

  if (USE_DIPLAY)
  {
    char buffer[4];
    dtostrf(v, 2, 1, buffer);
    sprintf(buffer,"%s", buffer);
    lcd.setCursor(0, 1);
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
    lcd.setCursor(8, 1);
    //lcd.print("lv:");
    lcd.print(buffer);  
  }
  return v;
}

/**
 * Setting AP
 */
struct Config {
    char name[20];
    bool enabled;
    int8_t hour;
    char password[20];
    char mqttServer[32];
    int8_t mqttPort;
} config;

struct Metadata {
    int8_t version;
} meta;

ConfigManager configManager;

void createCustomRoute(WebServer *server) {
    server->on("/custom", HTTPMethod::HTTP_GET, [server](){
        server->send(200, "text/plain", "Hello, World!");
    });
} 

void setupAp()
{
    meta.version = 3;
    // Setup config manager
    configManager.setAPName(AP_NAME);
    configManager.setAPFilename("/index.html");
    configManager.addParameter("name", config.name, 20);
    configManager.addParameter("enabled", &config.enabled);
    configManager.addParameter("hour", &config.hour);
    configManager.addParameter("password", config.password, 20, set);
    configManager.addParameter("mqtt_server", config.mqttServer, 32, set);
    configManager.addParameter("mqtt_port", &config.mqttPort);
    configManager.addParameter("version", &meta.version, get);
    configManager.begin(config);
    configManager.save();

    configManager.setAPCallback(createCustomRoute);
    configManager.setAPICallback(createCustomRoute);
}

void setup()
{
    Serial.begin(115200);
    Serial.println();
    /*
    if (drd.detect())
    {
        Serial.println("Double reset boot");
        for (int i = 0 ; i < EEPROM.length() ; i++) {
          EEPROM.write(i, 0);
        }
        delay(1000);
        ESP.reset();
    }
    else
    {
        Serial.println("** Normal boot **");
    }*/
    setupPins();
    setupAp();
    initControllerSensor();
    initEnvironmentSensor();
    initWaterTemperatureSensor();
    initWaterTankLevel();
    if (USE_DIPLAY)
    {
      initLCD();
    }
}

void loop()
{
  configManager.loop();
  /*
  root["water.temp"] = readWaterTemperature();
  root["ctrl.temp"] = readControllerTemperature();
  root["ctrl.humid"] = readControllerHumidity();
  root["env.temp"] = readEnvironmentTemperature();
  root["env.humid"] = readEnvironmentHumidity();
  root["water.level"] = readDistance();
  root.printTo(Serial);
  */
  Serial.print(readWaterTemperature());
  Serial.println(" C");
  Serial.print(readEnvironmentTemperature());
  Serial.println(" C");
  Serial.print(readEnvironmentHumidity());
  Serial.println(" %");
  Serial.print(readDistance());
  Serial.println(" cm");
  Serial.println("==========");

  //char buffer [50];
  //sprintf(buffer, "%3d mm", readDistance());
  //lcd.setCursor(0, 1);
  //lcd.print(buffer);

  //executeCommands();

  delay(2000);
}
