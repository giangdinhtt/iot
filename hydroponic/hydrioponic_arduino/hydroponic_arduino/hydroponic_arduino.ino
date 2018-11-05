/**
 * Arduino part for Hydroponic automation system
 */
#include "hydroponic_arduino.h"
#include "Esp8266.h"
#include <DoubleResetDetect.h>

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

/*
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32) //ESP8266 or ESP32
  #include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
  #include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
  #include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
  #include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#endif
*/
//LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);  // set the LCD address to 0x3f for a 16 chars and 2 line display

/**
 * Wifi configurations
 */
//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40];
char mqtt_port[6] = "8080";
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

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
  #if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32) //ESP8266 or ESP32
    // D9/GPIO1 (TX) swap the pin to a GPIO.
    //pinMode(D9, FUNCTION_3);
    // D10/GPIO3 (RX) swap the pin to a GPIO.
    //pinMode(D10, FUNCTION_3);
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
  pinMode(SWITCH_1, OUTPUT);
  pinMode(SWITCH_2, OUTPUT);
  //pinMode(SWITCH_3, OUTPUT);
  //pinMode(SWITCH_4, OUTPUT);

  // Relay triggered as LOW, set HIGH level to turn them OFF
  digitalWrite(SWITCH_1, HIGH);
  digitalWrite(SWITCH_2, HIGH);
  //digitalWrite(SWITCH_3, HIGH);
  //digitalWrite(SWITCH_4, HIGH);
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void initAccessPoint()
{
  /*
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
  */

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);

  /**
   * Press RST button 2 times to reset access point
   */
  if (drd.detect())
  {
      Serial.println("Double reset boot, resetting AP");
      wifiManager.resetSettings();  //reset settings - for testing
  }

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality(10);
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  //first parameter is name of access point, second is the password
  if (!wifiManager.autoConnect(AP_NAME, AP_PASSWORD)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
/*
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
*/
    json.printTo(Serial);
//    json.printTo(configFile);
//    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
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
    sprintf(buffer, "%2dcm", v);
    lcd.setCursor(0, 1);
    lcd.print("lv:");
    lcd.print(buffer);
  }
  return v;
}

void showSwitchesStatus()
{
  if (USE_DIPLAY)
  {
    lcd.setCursor(8, 1);
    lcd.print("        ");
    lcd.setCursor(8, 1);
    lcd.print("sw: ");
    if (!digitalRead(SWITCH_1))
    {
      lcd.print("1");
    }
    if (!digitalRead(SWITCH_2))
    {
      lcd.print("2");
    }
    if (!digitalRead(SWITCH_3))
    {
      lcd.print("3");
    }
    if (!digitalRead(SWITCH_4))
    {
      lcd.print("4");
    }
  }
}

void waitForCommands()
{
  char json[] = "{\"switch-1\":1,\"switch-2\":0,\"switch-3\":0,\"switch-4\":0}";
  StaticJsonBuffer<200> commandJsonBuffer;
  JsonObject& commandRoot = commandJsonBuffer.parseObject(json);
  if (!commandRoot.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  String output;
  commandRoot.printTo(output);
  Serial.println(output);

  if (commandRoot.containsKey("switch-1"))
  {
    digitalWrite(SWITCH_1, !commandRoot["switch-1"]);
  }
  if (commandRoot.containsKey("switch-2"))
  {
    digitalWrite(SWITCH_2, !commandRoot["switch-2"]);
  }
  if (commandRoot.containsKey("switch-3"))
  {
    digitalWrite(SWITCH_3, !commandRoot["switch-3"]);
  }
  if (commandRoot.containsKey("switch-4"))
  {
    digitalWrite(SWITCH_4, !commandRoot["switch-4"]);
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  setupPins();
  if (USE_DIPLAY)
  {
    initLCD();
    lcd.setCursor(0, 1);
    lcd.print("BaudRate: ");
    lcd.print(BAUD_RATE);
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

  root["switch-1"] = (int) !digitalRead(SWITCH_1);
  root["switch-2"] = (int) !digitalRead(SWITCH_2);
  root["switch-3"] = (int) !digitalRead(SWITCH_3);
  root["switch-4"] = (int) !digitalRead(SWITCH_4);

  showSwitchesStatus();

  String output;
  root.printTo(output);
  
  Serial.println(output);
  //root.printTo(Serial);

  waitForCommands();

  delay(500);
}
