//Edwin MQTT IR remote
//v.3.0
#include <base64.hpp>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include <ESP8266WiFi.h>
//MQTT_MAX_PACKET_SIZE in PubSubClient should be changed to 256
#include <PubSubClient.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>  
#include <EEPROM.h>

#define DEBUG               false
#define IR_LED D3

#define BYTE_T uint8_t
#define TWOBYTE_T uint16_t
#define LOWBYTE(x)          ((BYTE_T)x)
#define HIGHBYTE(x)         ((TWOBYTE_T)x >> 0x8)
#define BYTE_COMBINE(h, l)  (((BYTE_T)h << 0x8) + (BYTE_T)l)

const char* mqttClientName = "edwinir_tv1";
const char* subscriptionIRCommands = "com/edwinir_tv1/ir";

String mqttServer = "";
String mqttUser = "";
String mqttPassword = "";

bool shouldSaveConfig = false;

IRsend irsend(IR_LED);

WiFiClient espClient;
PubSubClient client(espClient);

int status = WL_IDLE_STATUS;

void wait_for_wifi() {
  delay(10);
  if (DEBUG) {
    Serial.println();
    Serial.print("Checking WiFi connection");
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
    Serial.print("There is a message on topic: ");
    Serial.println(topic);
  }
  if (strcmp(topic, subscriptionIRCommands) == 0) {
    if (DEBUG) {
      Serial.println("Processing IR data...");
      Serial.println("Payload:");
    }
    unsigned char base64Data[length];
    for (int i = 0; i < length; i++) {
      base64Data[i] = (char)payload[i];
      if (DEBUG) Serial.print((char)payload[i]);
    }
    if (DEBUG) Serial.println();
    unsigned char decoded8BitArray[700]; //Max 16 bit array langth will be 350
    if (DEBUG) Serial.println("Decoding base64 data to 8 bit array...");
    unsigned int decodedDataLength = decode_base64(base64Data, decoded8BitArray);
    if (DEBUG) {
      Serial.print("8 bit array length: ");
      Serial.println(decodedDataLength);
      Serial.print("Restoring 16 bit array");
    }
    uint16_t rawIRData[decodedDataLength / 2];
    int rest_count = 0;
    for (int i=0; i < decodedDataLength; i+=2) {
        rawIRData[rest_count] = BYTE_COMBINE(decoded8BitArray[i], decoded8BitArray[i+1]);
        rest_count++;
        if (DEBUG) Serial.print(".");
    }
    if (DEBUG) {
      Serial.println("done. Result:");
      Serial.print("{");
      for (int i = 0; i < (decodedDataLength / 2); i++) {
        Serial.print(rawIRData[i]);
        Serial.print(",");
      }
      Serial.println("}");
    }
  
    irsend.sendRaw(rawIRData, sizeof(rawIRData) / 2, 38);
  }
 
}

void reconnect() {
  while (!client.connected()) {
    if (DEBUG) Serial.print("Attempting MQTT connection...");

    if (client.connect(mqttClientName,mqttUser.c_str(),mqttPassword.c_str())) {
      if (DEBUG) Serial.println("connected");
      delay(200);
      client.subscribe(subscriptionIRCommands);
      if (DEBUG) {
       Serial.println("Subscribed to:");
       Serial.println(subscriptionIRCommands);
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
  irsend.begin();
  if (DEBUG) Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
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
  
  wifiManager.autoConnect("Edwin-IR-01", "00000008");
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
  client.setBufferSize(256);
  client.setServer(mqttServer.c_str(), 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
}
