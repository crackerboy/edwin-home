//WiFi enabled MQTT hunidifier based on Wemos D1 mini and relay module.
//v.2.0
#include <EEPROM.h>

#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>  

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_CLIENT_NAME    "humidifier"
#define DEBUG               false

const char* powerTopic = "edwin/humidifier/power";
const char* stateTopic = "edwin/humidifier/state";

const int relayPin = D1;
const int switchPin = D5;

String mqttServer = "";
String mqttUser = "";
String mqttPassword = "";

bool shouldSaveConfig = false;

int lastSwitchState = 0;

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

  if ((char)payload[0] == '1' && digitalRead(switchPin) == HIGH) {
    digitalWrite(relayPin, HIGH);
  } else if ((char)payload[0] == '0') {
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

    if (client.connect(MQTT_CLIENT_NAME,mqttUser.c_str(),mqttPassword.c_str())) {
      if (DEBUG) Serial.println("connected");
      delay(200);
      sendStatus();
      client.subscribe(powerTopic);
      if (DEBUG) {
       Serial.print("subscribed to : ");
       Serial.println(powerTopic);
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
  
  wifiManager.autoConnect("Edwin-Humidifier", "00000008");
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
    String payload = "";
    if (digitalRead(relayPin) == HIGH) {
      payload = "on";
    } else {
      payload = "off";
    }
    
    if (DEBUG) {
      Serial.print("Publish topic: ");
      Serial.println(stateTopic);
      Serial.print("Publish message: ");
      Serial.println(payload);
    }
    client.publish( stateTopic , (char*) payload.c_str(), true );
  }  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  int currentSwitchState = digitalRead(switchPin);
  if (lastSwitchState != currentSwitchState) {
    lastSwitchState = currentSwitchState;
    if (DEBUG) {
      Serial.print("Changing relay state according to switch: ");
      Serial.println(lastSwitchState);
    }
    digitalWrite(relayPin, lastSwitchState);
    sendStatus();
  }
  client.loop();
}
