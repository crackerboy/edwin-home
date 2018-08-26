//WiFi enabled MQTT hunidifier based on Wemos D1 mini and relay module.
//v.1.0

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_CLIENT_NAME    "humidifier"
#define DEBUG               false

const char* ssid = "SSID";
const char* pswd = "WiFi password";
const char* mqtt_server = "192.168.2.197";
const char* topic = "edwin";
const char* mqtt_user = "user";
const char* mqtt_password = "password";

const int relayPin = D1;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
int value = 0;

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
  } else {
    digitalWrite(relayPin, LOW);
  }

  delay(500);
  sendStatus();
}


void reconnect() {
  while (!client.connected()) {
    if (DEBUG) Serial.print("Attempting MQTT connection...");

    if (client.connect(MQTT_CLIENT_NAME,mqtt_user,mqtt_password)) {
      if (DEBUG) Serial.println("connected");
      delay(200);
      sendStatus();
      String subscription;
      subscription += topic;
      subscription += "/";
      subscription += MQTT_CLIENT_NAME;
      subscription += "/power";
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

void setup() {
  pinMode(relayPin, OUTPUT); 
  if (DEBUG) Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
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
    String pubTopic;
     pubTopic += topic ;
     pubTopic += "/";
     pubTopic += MQTT_CLIENT_NAME;
     pubTopic += "/state";
     if (DEBUG) {
      Serial.print("Publish topic: ");
      Serial.println(pubTopic);
      Serial.print("Publish message: ");
      Serial.println(payload);
     }
    client.publish( (char*) pubTopic.c_str() , (char*) payload.c_str(), true );
  }  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
