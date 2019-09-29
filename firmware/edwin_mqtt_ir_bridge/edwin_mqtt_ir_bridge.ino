//Edwin Bridge - MQTT <-> IR
const char* firmwareVer = "1.1";
#include <base64.hpp>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include <ESP8266WiFi.h>
#include "edwin_secrets.h" //contains some private information
//MQTT_MAX_PACKET_SIZE in PubSubClient should be changed to 1024
#include <PubSubClient.h>

#define DEBUG               false
#define IR_LED D3
#define IR_PIN D4
#define CAPTURE_BUFFER_SIZE 1024 //IR
#define TIMEOUT 15U // Suits most messages, while not swallowing many repeats.
#define MIN_UNKNOWN_SIZE 12

#define BYTE_T uint8_t
#define TWOBYTE_T uint16_t
#define LOWBYTE(x)          ((BYTE_T)x)
#define HIGHBYTE(x)         ((TWOBYTE_T)x >> 0x8)
#define BYTE_COMBINE(h, l)  (((BYTE_T)h << 0x8) + (BYTE_T)l)

// All this variables declared in secrets.h
//const char* ssid = "WiFi_Name";
//const char* pswd = "mywifipassword";
//const char* mqtt_server = "192.168.1.1";
//const char* mqtt_user = "user";
//const char* mqtt_password = "password";

const char* mqttClientName = "edwinbridge";
const char* subscriptionIRCommands = "com/edwinbridge/ir";
const char* subscriptionModeCommands = "com/edwinbridge/mode";
const char* stateTopic = "state/edwinbridge/state";

IRsend irsend(IR_LED);
IRrecv irrecv(IR_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);

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
  } else if (strcmp(topic, subscriptionModeCommands) == 0) {
    if (payload[0] == 'i') {
      startIRListening();
    }
  }
 
}

void startIRListening() {
  boolean done = false;
  sendState("IR listening");
  if (DEBUG) Serial.println("Starting IR listening...");
  decode_results results;
  irrecv.enableIRIn();

  while (!done) {
    yield();
    // Check if the IR code has been received.
    if (irrecv.decode(&results)) {
      uint16_t rawLength = getCorrectedRawLength(&results);
      if (DEBUG) {
        Serial.print("length=");
        Serial.println(rawLength);
      }
      uint16_t dataArray[rawLength];
      Serial.println("Data array:");
      for (int i = 1; i < results.rawlen; i++) {
        dataArray[i-1] = results.rawbuf[i] * RAWTICK;
        if (DEBUG) {
          Serial.print(dataArray[i-1]);
          Serial.print(",");
        }
        yield();  // Feed the WDT (again)
      }
      if (DEBUG) {
        Serial.println("");
        Serial.println("Encoding data:");
      }
      uint16_t splittedLength = rawLength*2;
      uint8_t split_values[splittedLength];
      int val_count = 0;
      for (int i=0; i<splittedLength; i+=2) {
          split_values[i] = HIGHBYTE(dataArray[val_count]);
          split_values[i+1] = LOWBYTE(dataArray[val_count]);
          val_count++;
      }
      unsigned char base64[400];

      unsigned int base64_length = encode_base64((unsigned char *)split_values, splittedLength, base64);
      
      if (DEBUG) {
        Serial.print("Base64 length=");
        Serial.println(base64_length);
        Serial.println((char *) base64);
      }
      sendStateWithData("Heard", "IR", typeToString(results.decode_type, results.repeat), (char *)base64);
      done = true;
    }
  }
  irrecv.disableIRIn();
}

void reconnect() {
  while (!client.connected()) {
    if (DEBUG) Serial.print("Attempting MQTT connection...");

    if (client.connect(mqttClientName,mqtt_user,mqtt_password)) {
      if (DEBUG) Serial.println("connected");
      delay(200);
      client.subscribe(subscriptionIRCommands);
      client.subscribe(subscriptionModeCommands);
      sendState("idle");
      if (DEBUG) {
       Serial.println("Subscribed to:");
       Serial.println(subscriptionIRCommands);
       Serial.println(subscriptionModeCommands);
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

void sendStateWithData(String state, String source, String type, String theData) {
  String jsonData = "{\"state\" : \"";
  jsonData += state;
  jsonData += "\", \"dataSource\" : \"";
  jsonData += source;
  jsonData += "\", \"dataType\" : \"";
  jsonData += type;
  jsonData += "\", \"data\" : \"";
  jsonData += theData;
  jsonData += "\"";
  sendStateJSON(jsonData);
}

void sendState(String state) {
  String jsonData = "{\"state\" : \"";
  jsonData += state;
  jsonData += "\"";
  sendStateJSON(jsonData);
}

void sendStateJSON(String jsonData) {
  jsonData += ", \"firmwareVersion\": \"";
  jsonData += firmwareVer;
  jsonData += "\"}";
  if (client.connected()) {
    if (DEBUG) {
      Serial.print("Publish topic: ");
      Serial.println(stateTopic);
      Serial.print("Publish message: ");
      Serial.println(jsonData);
     }
    client.publish( stateTopic , (char*) jsonData.c_str(), true );
  }    
}

void setup() {
  irsend.begin();
  //pinMode(D2, INPUT_PULLUP);
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
