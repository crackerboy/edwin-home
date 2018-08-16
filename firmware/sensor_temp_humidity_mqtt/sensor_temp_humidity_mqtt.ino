//WiFi enabled MQTT temperature and humidity sensor based on Wemos D1 mini and STH30 shield.
//v.1.4

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WEMOS_SHT3X.h>

#define WIFI_SSID           "******"
#define WIFI_PASSWORD       "******"

#define MQTT_SERVER         "192.168.2.197"
#define MQTT_CLIENT_NAME    "edwin-ths.1.4"
#define MQTT_TOPIC_TEMP     "edwin/temphumisensor/temperature"
#define MQTT_TOPIC_HUMID    "edwin/temphumisensor/humidity"
#define MQTT_USER           "******"
#define MQTT_PASSWORD       "******"

#define PUBLISH_RATE        10*60

WiFiClient    wifi;
PubSubClient  mqtt(MQTT_SERVER, 1883, wifi);
SHT3X         sht30(0x45);

bool connectWiFi() {
  int retryCounter = 0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //Serial.println("Connecting to WiFi");
  while ( WiFi.status() != WL_CONNECTED) {
    //Serial.print(".");
    retryCounter++;
    if (retryCounter >= 100) {
      //Serial.println("fail");
      return false;
    }
    delay(100);
  }
  //Serial.println("done");
  return true;    
}

void setup() {
  //Serial.begin(9600);
  //Serial.setDebugOutput(true);
  //Serial.println("Wake Up!");
  pinMode(D0, WAKEUP_PULLUP);
  if (!connectWiFi()) {
    goToSleep();  
  } else {
    //Serial.print("Connecting to MQTT server...");
    if (!mqtt.connect(MQTT_CLIENT_NAME, MQTT_USER, MQTT_PASSWORD)) {
      //Serial.println("fail");
      goToSleep();  
    } else {
      //Serial.println("done");
      sht30.get();
      float t = sht30.cTemp;
      float h = sht30.humidity;
      //Serial.println("Sending data and waiting 500ms...");
      mqtt.publish(MQTT_TOPIC_TEMP,  String(t).c_str(), true);
      mqtt.publish(MQTT_TOPIC_HUMID, String(h).c_str(), true);
      delay(500);
      goToSleep();
    }
  }
}

void goToSleep() {
  //Serial.println("Closing all connections...");
  mqtt.disconnect();
  wifi.stop();
  WiFi.disconnect();
  //Serial.println("Slip.");
  ESP.deepSleep(PUBLISH_RATE * 1e6);  
}

void loop() {
}
