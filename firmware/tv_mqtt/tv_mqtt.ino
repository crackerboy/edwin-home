//WiFi enabled MQTT IR controler for an old Samsung TV
//v.1.0
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "samsung_ir_codes.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_CLIENT_NAME    "kitchen_tv"
#define DEBUG               true
#define IR_LED D3

const char* ssid = "******";
const char* pswd = "***********";
const char* mqtt_server = "192.168.2.197";
const char* topic = "some_topic";
const char* mqtt_user = "*****";
const char* mqtt_password = "****";

IRsend irsend(IR_LED);
WiFiClient espClient;
PubSubClient client(espClient);

int status = WL_IDLE_STATUS;

void setup_wifi() {
  delay(10);
  if (DEBUG) {
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  }
  WiFi.begin(ssid, pswd);
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
  String command = "";
  for (int i = 0; i < length; i++) {
    command +=(char)payload[i];
  }
  if (DEBUG) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.println(command);
  }

  if (command == "Power") {
    irsend.sendRaw(codePower, sizeof(codePower) / 2, 38);
  } else if (command == "VolumeUp") {
    irsend.sendRaw(codeVolumeUp, sizeof(codeVolumeUp) / 2, 38);
  } else if (command == "VolumeDown") {
    irsend.sendRaw(codeVolumeDown, sizeof(codeVolumeDown) / 2, 38);
  }

  
}

void reconnect() {
  while (!client.connected()) {
    if (DEBUG) Serial.print("Attempting MQTT connection...");

    if (client.connect(MQTT_CLIENT_NAME,mqtt_user,mqtt_password)) {
      if (DEBUG) Serial.println("connected");
      delay(200);
      sendState("online");
      String subscription;
      subscription += topic;
      subscription += "/";
      subscription += MQTT_CLIENT_NAME;
      subscription += "/com";
      client.subscribe(subscription.c_str() );
      if (DEBUG) {
       Serial.print("subscribed to : ");
       Serial.println(subscription);
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

void sendState(String state) {
  if (client.connected()) {
    String pubTopic;
     pubTopic += topic ;
     pubTopic += "/";
     pubTopic += MQTT_CLIENT_NAME;
     pubTopic += "/state";
     if (DEBUG) {
      Serial.print("Publish topic: ");
      Serial.println(pubTopic);
      Serial.print("Publish message: ");
      Serial.println(state);
     }
    client.publish( (char*) pubTopic.c_str() , (char*) state.c_str(), true );
  }  
}

void setup() {
  irsend.begin();
  if (DEBUG) Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
