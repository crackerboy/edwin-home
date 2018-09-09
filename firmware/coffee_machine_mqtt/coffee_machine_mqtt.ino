//WiFi enabled MQTT hunidifier based on Wemos D1 mini and relay module.
//v.1.0

#include <ESP8266WiFi.h>
#include <ACS712.h>
#include <PubSubClient.h>
#include "edwin_secrets.h" //contains some private information

#define DEBUG               true

// All this variables declared in secrets.h
//const char* ssid = "WiFi_Name";
//const char* pswd = "mywifipassword";
//const char* mqtt_server = "192.168.1.1";
//const char* mqtt_user = "user";
//const char* mqtt_password = "password";

const char* mqttClientName = "edwin_coffee";
const char* subscriptionPowerCommands = "com/edwin_coffee/power";
const char* topicState = "state/edwin_coffee/power";

const int relayPin = D1;

WiFiClient espClient;
PubSubClient client(espClient);
ACS712 powerSensor(ACS712_30A, A0);

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
  if (DEBUG) {
    Serial.print("There is a message on topic: ");
    Serial.println(topic);
  }
  if (strcmp(topic, subscriptionPowerCommands) == 0) {
    if (DEBUG) {
      Serial.println("Processing POWER command...");
      Serial.println("Payload:");
    }
    if ((char)payload[0] == '1') {
      digitalWrite(relayPin, HIGH);
      delay(100);
      digitalWrite(relayPin, LOW);
    }
    
  }
  
  delay(500);
  sendStatus();
}


void reconnect() {
  while (!client.connected()) {
    if (DEBUG) Serial.print("Attempting MQTT connection...");

    if (client.connect(mqttClientName,mqtt_user,mqtt_password)) {
      if (DEBUG) Serial.println("connected");
      delay(200);
      sendStatus();
      client.subscribe(subscriptionPowerCommands);
      if (DEBUG) {
       Serial.print("subscribed to : ");
       Serial.println(subscriptionPowerCommands);
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

void setup() {
  pinMode(relayPin, OUTPUT); 
  if (DEBUG) Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void sendStatus() {
  if (client.connected()) {
    float I = powerSensor.getCurrentAC();
    if (DEBUG) Serial.println(String("I = ") + I + " A");
    
    String payload = "";
    if (I > 0) {
      payload = "on";
    } else {
      payload = "off";
    }
    if (DEBUG) {
      Serial.print("Publish topic: ");
      Serial.println(topicState);
      Serial.print("Publish message: ");
      Serial.println(payload);
    }
    client.publish( topicState , (char*) payload.c_str(), true );
  }  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
