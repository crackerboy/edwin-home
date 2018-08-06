#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WEMOS_SHT3X.h>

#define WIFI_SSID           "XPEH"
#define WIFI_PASSWORD       "givemetheslowinternet"

#define MQTT_SERVER         "192.168.2.197"
#define MQTT_CLIENT_NAME    "d1mini01"
#define MQTT_TOPIC_TEMP     "sensor/d1mini01/temperature"
#define MQTT_TOPIC_HUMID    "sensor/d1mini01/humidity"

#define PUBLISH_RATE        7*60      // deep sleep timer in seconds

#define DEBUG               false      // debug to serial port

//Libraries initialization
WiFiClient    wifi;
PubSubClient  mqtt(MQTT_SERVER, 1883, wifi);
SHT3X         sht30(0x45);

bool connectWiFi() {
  int retryCounter = 0;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while ( WiFi.status() != WL_CONNECTED) {
    retryCounter++;
    if (DEBUG) Serial.println("connecting Wifi");
    //Waiting 10 seconds for connection
    if (retryCounter >= 20) {
      if (DEBUG) Serial.println("No WiFi connection. Will try tommorow");
      return false;
    }
    delay(500);
  }
  return true;    
}

bool connectMQTT() {
  int retryCounter = 0;
    while ( !mqtt.connected() ) {
      if (DEBUG) Serial.println("connecting MQTT");
      if ( !mqtt.connect(MQTT_CLIENT_NAME,"mqtt_user","http_api_password") ) {
        retryCounter++;
        //Trying to connect 3 times
        if (retryCounter > 3) {
          if (DEBUG) Serial.println("No MQTT connection. Will try tommorow");
          return false;
        }
        delay(500);
      }
    }
    return true;
}

void setup() {
  pinMode(D0, WAKEUP_PULLUP);
  if (DEBUG) Serial.begin(9600);

  if (!connectWiFi()) {
    goToSleep();  
  } else {
  
    sht30.get();
    float t = sht30.cTemp;
    float h = sht30.humidity;
  
    if (DEBUG) {
      Serial.print("temp: ");
      Serial.print(t);
      Serial.print(" | humid: ");
      Serial.println(h);
    }
  
    if (!connectMQTT()) {
      goToSleep();  
    } else {
  
      mqtt.publish(MQTT_TOPIC_TEMP,  String(t).c_str(), true);
      mqtt.publish(MQTT_TOPIC_HUMID, String(h).c_str(), true);
      delay(3000);
      if (DEBUG) {
        Serial.print("sleeping ");
        Serial.print(PUBLISH_RATE);
        Serial.print(" s");
      }
      goToSleep();
    }
  }
}

void goToSleep() {
  if (WiFi.status() == WL_CONNECTED) WiFi.disconnect();
  if (mqtt.connected()) mqtt.disconnect();
  ESP.deepSleep(PUBLISH_RATE * 1e6);  
}

void loop() {
}
