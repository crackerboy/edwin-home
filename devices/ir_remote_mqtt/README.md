# MQTT IR remote built-in in TV

### Components
* LOLIN (Wemos) D1 mini ([wemos.cc](https://www.wemos.cc/en/latest/d1/d1_mini.html))
* LOLIN (Wemos) D1 mini IR Controller Shield ([wemos.cc](https://www.wemos.cc/en/latest/d1_mini_shield/ir.html))
* AC-DC 220V to 5V Step-Down Mini Power Supply ([amazon](https://www.amazon.com/Hi-link-HLK-PM01-Step-Down-Intelligent-Household/dp/B01B7G6LYE))

![](https://github.com/estevez-dev/edwin-home/raw/master/devices/ir_remote_mqtt/ir_remote_mqtt.png)

### Dependencies

* _ESP8266 Arduino board manager_ ([link](https://github.com/esp8266/Arduino)). Allows to develop and flash ESP8266-based boards with Arduino IDE 
* _WiFiManager for ESP8266 boards_ ([link](https://github.com/tzapu/WiFiManager)). Enables simple WiFi configuration for your projects. Allows to configure other custom parameters
* _PubSubClient_ library for MQTT support
* Base64 Arduino library for decoding received data
* IRremoteESP8266 library
* _EEPROM_ library for storing settings in EEPROM

### Changelog
**v.3.0**
* WiFiManager integrated with additional config enries for MQTT connection settings.
* Write and read settings to/from EEPROM
