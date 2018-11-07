/**
 * Arduino part for Hydroponic automation system
 */
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include "hydroponic_arduino.h"
#include "Esp8266.h"
#include <PubSubClient.h>

/****************************************
 * Define Constants
 ****************************************/
/*
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32) //ESP8266 or ESP32
  #include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
  #include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
  #include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
  #include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#endif
*/
//LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);  // set the LCD address to 0x3f for a 16 chars and 2 line display
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

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

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
  delayMicroseconds(100);
  digitalWrite(TRIG_PIN, HIGH); // phát xung từ chân trig
  delayMicroseconds(10);    // xung có độ dài 5 microSeconds
  digitalWrite(TRIG_PIN, LOW);  // tắt chân trig
  
  /* Tính toán thời gian */
  // Đo độ rộng xung HIGH ở chân echo. 
  duration = pulseIn(ECHO_PIN, HIGH);  
  // Distance from water tank roof to water surface
  int s = int(duration*0.343/2/10);  // in cm
  // Water level
  int lv = tank_height - s;

  if (USE_DIPLAY)
  {
    char buffer [8];
    sprintf(buffer, "%2dcm", lv);
    lcd.setCursor(0, 1);
    lcd.print("lv:");
    lcd.print(buffer);
  }
  return lv;
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

void performCommand(char* json)
{
  //char json[] = "{\"switch-1\":1,\"switch-2\":0,\"switch-3\":0,\"switch-4\":0}";
  StaticJsonBuffer<100> commandJsonBuffer;
  JsonObject& commandRoot = commandJsonBuffer.parseObject(json);
  if (!commandRoot.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  if (commandRoot.containsKey("sw-1"))
  {
    digitalWrite(SWITCH_1, !commandRoot["sw-1"]);
  }
  if (commandRoot.containsKey("sw-2"))
  {
    digitalWrite(SWITCH_2, !commandRoot["sw-2"]);
  }
  if (commandRoot.containsKey("sw-3"))
  {
    digitalWrite(SWITCH_3, !commandRoot["sw-3"]);
  }
  if (commandRoot.containsKey("sw-4"))
  {
    digitalWrite(SWITCH_4, !commandRoot["sw-4"]);
  }
}

/*
 * Init json serialize
 */
StaticJsonBuffer<100> sensorJsonBuffer;
JsonObject& sensor = sensorJsonBuffer.createObject();

StaticJsonBuffer<100> relayJsonBuffer;
JsonObject& relay = relayJsonBuffer.createObject();

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() {
  Serial.begin(BAUD_RATE);
  if (drd.detect())
  {
      Serial.println("** Double reset boot **");
      hard_reset_required = true;
  }
  else
  {
      Serial.println("** Normal boot **");
  }
  setupPins();
  if (USE_DIPLAY)
  {
    initLCD();
    lcd.setCursor(0, 0);
    lcd.print("BaudRate: ");
    lcd.print(BAUD_RATE);
  }
  #if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32) //ESP8266 or ESP32
    setupAccessPoint();
  #endif
  initControllerSensor();
  initEnvironmentSensor();
  initWaterTemperatureSensor();
  initWaterTankLevel();
  initRelays();
  delay(1000);
  if (USE_DIPLAY)
  {
    lcd.clear();
  }
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);
}

float lastWaterTemp;
float lastEnvTemp;
float lastEnvHumid;
int lastWaterLevel;

void loop() {
  bool sensorChanged = false;
  float waterTemp = readWaterTemperature();
  if (!isnan(waterTemp) && waterTemp > 0 && waterTemp != lastWaterTemp)
  {
    sensor["water.temp"] = waterTemp;
    lastWaterTemp = waterTemp;
    sensorChanged = true;
  }

  float envTemp = readEnvironmentTemperature();
  if (!isnan(envTemp) && envTemp > 0 && envTemp != lastEnvTemp)
  {
    sensor["env.temp"] = envTemp;
    lastEnvTemp = envTemp;
    sensorChanged = true;
  }

  float envHumid = readEnvironmentHumidity();
  if (!isnan(envHumid) && envHumid > 0 && envHumid != lastEnvHumid)
  {
    sensor["env.humid"] = envHumid;
    lastEnvHumid = envHumid;
    sensorChanged = true;
  }


  int waterLevel = readDistance();
  if (!isnan(waterLevel) && waterLevel != tank_height && waterLevel != lastWaterLevel)
  {
    sensor["water.level"] = waterLevel;
    lastWaterLevel = waterLevel;
    sensorChanged = true;
  }

  //sensor.printTo(Serial);
  //Serial.println();
  
  //sensor["ctrl.temp"] = readControllerTemperature();
  //sensor["ctrl.humid"] = readControllerHumidity();
  //sensor["env.temp"] = readEnvironmentTemperature();
  //sensor["env.humid"] = readEnvironmentHumidity();
  //sensor["water.level"] = readDistance();

  relay["sw-1"] = (int) !digitalRead(SWITCH_1);
  relay["sw-2"] = (int) !digitalRead(SWITCH_2);
  relay["sw-3"] = (int) !digitalRead(SWITCH_3);
  relay["sw-4"] = (int) !digitalRead(SWITCH_4);

  char relayOutput[100];
  relay.printTo(relayOutput);
  //Serial.println(relayOutput);
  showSwitchesStatus();

  if (!mqttClient.connected()) {
    reconnect();
  }
  if(!mqttClient.loop()) {
    mqttClient.connect(MQTT_CLIENT_ID);
  }

  mqttClient.publish("iot/hydroponic/relay", relayOutput);

  now = millis();
  // Publishes sensor data intervally
  if (sensorChanged || now - lastMeasure > READ_INTERVAL) {
    lastMeasure = now;
    char sensorOutput[100];
    sensor.printTo(sensorOutput);
    mqttClient.publish("iot/hydroponic/sensor", sensorOutput);
  }

  delay(500);
}

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Allocate the correct amount of memory for the payload copy
  char payload[length];
  for (int i = 0; i < length; i++) {
    payload[i] = (char) message[i];
  }
  /*
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  */
  Serial.println(payload);

  if (topic=="iot/hydroponic/controller"){
      performCommand(payload);
  } else if (topic=="iot/hydroponic/reset"){
    #if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32) //ESP8266 or ESP32
      ESP.reset();
      delay(1000);
    #endif
    Serial.println("Arduino reset");
    resetFunc();
  } 
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    /*
     YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
     To change the ESP device ID, you will have to give a new name to the ESP8266.
     Here's how it looks:
       if (mqttClient.connect("ESP8266Client")) {
     You can do it like this:
       if (mqttClient.connect("ESP1_Office")) {
     Then, for the other ESP:
       if (mqttClient.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      mqttClient.subscribe("iot/hydroponic/controller");
      mqttClient.subscribe("iot/hydroponic/reset");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
