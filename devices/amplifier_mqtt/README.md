# MQTT controlled integrated stereo amplifier

Controlls the power of Cambridge Audio AXA25 amplifier simulating short factory button click with relay. Detects the state of amplifier by detecting the curret on factory USB port.

### Components
* Cambridge Audio AXA25
* LOLIN (Wemos) D1 mini ([wemos.cc](https://www.wemos.cc/en/latest/d1/d1_mini.html))
* 5V Relay Brick by Itead ([itead.com](https://www.itead.cc/electronic-brick-5v-relay.html))
* 12V to 5V DC converter

![](https://github.com/estevez-dev/edwin-home/raw/master/devices/amplifier_mqtt/amplifier_mqtt.png)

### Dependencies

* _ESP8266 Arduino board manager_ ([link](https://github.com/esp8266/Arduino)). Allows to develop and flash ESP8266-based boards with Arduino IDE 
* _WiFiManager for ESP8266 boards_ ([link](https://github.com/tzapu/WiFiManager)). Enables simple WiFi configuration for your projects. Allows to configure other custom parameters
* _PubSubClient_ library for MQTT support
* _EEPROM_ library for storing settings in EEPROM
