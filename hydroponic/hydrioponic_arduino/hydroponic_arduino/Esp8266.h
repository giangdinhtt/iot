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
 * Configurations via Wifi
 */
#define AP_NAME "HydroponicAP" // Assigns your Access Point name
#define AP_PASSWORD "12345678" // Assigns your Access Point name
#define MQTT_SERVER "mqtt.giang.xyz"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "iot.hydroponic.esp8266"

// Must modify this configuration in PubSubClient.h (in libraries folder to make it work)
#define MQTT_MAX_PACKET_SIZE 256  // https://pubsubclient.knolleary.net/api.html

//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40];
int mqtt_port;

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void configModeCallback(WiFiManager *myWiFiManager) {
  if (USE_DIPLAY)
  {
    lcd.setCursor(0, 1);
    lcd.print("Entered AP mode");
  }
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setupAccessPoint()
{
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
        Serial.println("");
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          mqtt_port = json["mqtt_port"];
          tank_height = json["tank_height"];

        } else {
          Serial.println("failed to load json config, using defaults");
          strcpy(mqtt_server, MQTT_SERVER);
          mqtt_port = MQTT_PORT;
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  /**
   * Press RST button 2 times cause access point reset
   */
  if (hard_reset_required)
  {
      Serial.println("Double reset boot, resetting AP");
      //clean FS, for testing
      SPIFFS.format();
      wifiManager.resetSettings();  //reset settings - for testing
  }

  //set config save notify callback
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  char mqtt_port_str[6];
  itoa(mqtt_port, mqtt_port_str, 10);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port_str, 6);
  char tank_height_str[3];
  itoa(tank_height, tank_height_str, 10);
  WiFiManagerParameter custom_tank_height("tank_height", "Chiều cao bể nước", tank_height_str, 3);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_tank_height);

  //set minimum quality of signal so it ignores AP's under that quality. Defaults to 8%
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
  mqtt_port = atoi(custom_mqtt_port.getValue());
  tank_height = atoi(custom_tank_height.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["tank_height"] = tank_height;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    Serial.println("");
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  if (USE_DIPLAY)
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
  }
}

#endif
