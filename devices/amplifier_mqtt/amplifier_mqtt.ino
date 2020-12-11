#include <EEPROM.h>

#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>  

#include <PubSubClient.h>

#include "ACS712.h"

#define MQTT_CLIENT_NAME    "cambridge"
#define DEBUG               true

const char* fwVersion = "v.0.1";

const char* commandTopic = "edwin/cambridge/command";
const char* stateTopic = "edwin/cambridge/state";
const char* infoTopic = "edwin/cambridge/info";
const char* availabilityTopic = "edwin/cambridge/availability";

const int relayPin = D1;
const int switchPin = D5;

String mqttServer = "";
String mqttUser = "";
String mqttPassword = "";

bool shouldSaveConfig = false;

int lastSwitchState = 0;

ACS712 sensor(ACS712_30A, A0);

WiFiClient espClient;
PubSubClient client(espClient);

int status = WL_IDLE_STATUS;

void wait_for_wifi() {
  delay(10);
  if (DEBUG) {
    Serial.println();
    Serial.print("Checking WiFi connection");
    Serial.println(".");
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (DEBUG) Serial.print(".");
  }
  if (DEBUG) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (DEBUG) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  }

  if ((char)payload[0] == '1') {
    digitalWrite(relayPin, HIGH);
    delay(500);
    digitalWrite(relayPin, LOW);
  } else if ((char)payload[0] == '0') {
    digitalWrite(relayPin, HIGH);
    delay(500);
    digitalWrite(relayPin, LOW);
  }

  delay(500);
  sendStatus();
}


void reconnect() {
  while (!client.connected()) {
    if (DEBUG) {
      Serial.print("Attempting MQTT connection to ");
      Serial.print(mqttServer);
      Serial.print(" ");
      Serial.print(mqttUser);
      Serial.print(" ");
      Serial.print(mqttPassword);
      Serial.print(" ... ");
    }

    if (client.connect(
          MQTT_CLIENT_NAME,
          mqttUser.c_str(),
          mqttPassword.c_str(),
          availabilityTopic,
          0,
          1,
          "offline"
          )
        ) {
      if (DEBUG) Serial.println("connected");
      delay(200);
      sendStatus();
      client.subscribe(commandTopic);
      if (DEBUG) {
       Serial.print("subscribed to : ");
       Serial.println(commandTopic);
      }
    } else {
      if (DEBUG) {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.print(" wifi=");
        Serial.print(WiFi.status());
        Serial.println(" try again in 5 seconds");
      }
      delay(5000);
    }
  }
}

void saveConfigCallback() {
  if (DEBUG) Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(switchPin, INPUT); 
  if (DEBUG) {
    Serial.begin(115200);
    Serial.println("Reading settings from EEPROM...");
  }
  EEPROM.begin(512);
  int addr = 0;
  for (int l = 0; l < 40; ++l) {
    if (char(EEPROM.read(l)) != '|') {
      mqttServer += char(EEPROM.read(l));
    } else {
      break;
    }
  }
  addr = 40;
  for (int l = 0; l < 20; ++l) {
    if (char(EEPROM.read(addr+l)) != '|') {
      mqttUser += char(EEPROM.read(addr+l));
    } else {
      break;
    }
  }
  addr = 60;
  for (int l = 0; l < 20; ++l) {
    if (char(EEPROM.read(addr+l)) != '|') {
      mqttPassword += char(EEPROM.read(addr+l));
    } else {
      break;
    }
  }
  EEPROM.end();
  
  WiFiManager wifiManager;
  WiFiManagerParameter custom_mqtt_server("server", "MQTT server", mqttServer.c_str(), 40);
  WiFiManagerParameter custom_mqtt_user("user", "MQTT user", mqttUser.c_str(), 20);
  WiFiManagerParameter custom_mqtt_password("password", "MQTT password", mqttPassword.c_str(), 20);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.autoConnect("Cambridge-AXA25", "00000008");
  wait_for_wifi();
  mqttServer = String(custom_mqtt_server.getValue());
  mqttUser = String(custom_mqtt_user.getValue());
  mqttPassword = String(custom_mqtt_password.getValue());
  
  if (shouldSaveConfig) {
    if (DEBUG) {
      Serial.print("Saving settings to EEPROM...");
    }
    EEPROM.begin(512);
    int addr = 0;
    for (int j = 0; j < mqttServer.length(); j++) { 
      EEPROM.write(addr+j, mqttServer[j]);  
    }
    addr = mqttServer.length();
    EEPROM.write(addr, '|');
    addr = 40;
    for (int j = 0; j < mqttUser.length(); j++) { 
      EEPROM.write(addr+j, mqttUser[j]);  
    }
    addr = 40 + mqttUser.length();
    EEPROM.write(addr, '|');
    addr = 60;
    for (int j = 0; j < mqttPassword.length(); j++) { 
      EEPROM.write(addr+j, mqttPassword[j]);  
    }
    addr = 60 + mqttPassword.length();
    EEPROM.write(addr, '|');
    EEPROM.commit();
    EEPROM.end();
    if (DEBUG) {
      Serial.println("Done.");
    }
  }
  client.setServer(mqttServer.c_str(), 1883);
  client.setCallback(callback);
}

void sendStatus() {
  if (client.connected()) {
    client.publish( availabilityTopic , "online", true );
    String switchStatePayload;
    if (digitalRead(switchPin) == HIGH) {
      switchStatePayload = "on";
    } else {
      switchStatePayload = "off";
    }
    String infoPayload = "{\"switchState\": \""
      + switchStatePayload
      + "\", \"version\": \""
      + fwVersion
      + "\", \"ipAddress\": \""
      + WiFi.localIP()[0] + "." + WiFi.localIP()[1]
      + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3] +"\"}";
    
    if (DEBUG) {
      Serial.print("Publish topic: ");
      Serial.println(stateTopic);
      Serial.print("Publish message: ");
      Serial.println(switchStatePayload);
    }
    client.publish( stateTopic , switchStatePayload.c_str(), true );
    client.publish( infoTopic , infoPayload.c_str(), true );
  }  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  float I = sensor.getCurrentDC();
  if (DEBUG) Serial.println(String("I = ") + I + " A");
  delay(1000); 
  int currentSwitchState = digitalRead(switchPin);
  if (lastSwitchState != currentSwitchState) {
    lastSwitchState = currentSwitchState;
    if (DEBUG) {
      Serial.print("Switch state changed to ");
      Serial.println(lastSwitchState);
    }
    sendStatus();
  }
  client.loop();
}
