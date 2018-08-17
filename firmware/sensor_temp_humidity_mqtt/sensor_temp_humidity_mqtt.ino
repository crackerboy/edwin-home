//WiFi enabled MQTT temperature and humidity sensor based on Wemos D1 mini and STH30 shield.
//v.1.4
extern "C" {
  #include <user_interface.h>
}
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WEMOS_SHT3X.h>

#define WIFI_SSID           "000"
#define WIFI_PASSWORD       "0000000"

#define MQTT_SERVER         "192.168.2.197"
#define MQTT_CLIENT_NAME    "edwin-ths.1.4"
#define MQTT_TOPIC_TEMP     "edwin/temphumisensor/temperature"
#define MQTT_TOPIC_HUMID    "edwin/temphumisensor/humidity"
#define MQTT_USER           "0000"
#define MQTT_PASSWORD       "0000000"

#define PUBLISH_RATE        10//*60

WiFiClient    wifi;
PubSubClient  mqtt(MQTT_SERVER, 1883, wifi);
SHT3X         sht30(0x45);

bool connectWiFi() {
  int retryCounter = 0;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    retryCounter++;
    if (retryCounter >= 100) {
      Serial.println("fail");
      return false;
    }
    delay(100);
  }
  Serial.println("done");
  return true;    
}

void setup() {
  Serial.begin(9600);
  //Serial.setDebugOutput(true);
  Serial.println("Still alive");
  pinMode(D0, WAKEUP_PULLUP);
  rst_info *rinfo;
  rinfo = ESP.getResetInfoPtr();
  Serial.println(String("ResetInfo.reason = ") + (*rinfo).reason);
  if ((*rinfo).reason == REASON_DEEP_SLEEP_AWAKE) {
    Serial.println("Woke from deep sleep, performing full reset") ;
    ESP.restart() ;
  }
  if (!connectWiFi()) {
    goToSleep();  
  } else {
    Serial.print("Connecting to MQTT server...");
    if (!mqtt.connect(MQTT_CLIENT_NAME, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("fail");
      goToSleep();  
    } else {
      Serial.println("done");
      sht30.get();
      float t = sht30.cTemp;
      float h = sht30.humidity;
      Serial.print("Sending data...");
      delay(500);
      mqtt.publish(MQTT_TOPIC_TEMP,  String(t).c_str(), true);
      mqtt.publish(MQTT_TOPIC_HUMID, String(h).c_str(), true);
      delay(500);
      Serial.println("done");
      goToSleep();
    }
  }
}

void goToSleep() {
  Serial.print("Closing all connections...");
  mqtt.disconnect();
  wifi.stop();
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(1000);
  Serial.println("done");
  Serial.println("Sleep...");
  ESP.deepSleep(PUBLISH_RATE * 1e6, WAKE_RF_DISABLED);  
}

void loop() {
}
