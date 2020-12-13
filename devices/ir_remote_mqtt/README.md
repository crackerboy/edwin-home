# Built-in MQTT IR remote

Built-in into TV. Sends IR commands right to factory IR receiver. Detects 5V power on factory USB port to determine current TV state (on or off).

### Components
* LOLIN (Wemos) D1 mini ([wemos.cc](https://www.wemos.cc/en/latest/d1/d1_mini.html))
* LOLIN (Wemos) D1 mini IR Controller Shield ([wemos.cc](https://www.wemos.cc/en/latest/d1_mini_shield/ir.html))
* AC-DC 220V to 5V Step-Down Mini Power Supply ([amazon](https://www.amazon.com/Hi-link-HLK-PM01-Step-Down-Intelligent-Household/dp/B01B7G6LYE))

[MQTT IR bridge](/devices/edwin_mqtt_ir_bridge) can be used to read IR codes from any remote and encode it in Base64.

![](https://github.com/estevez-dev/edwin-home/raw/master/devices/ir_remote_mqtt/ir_remote_mqtt.png)

### Dependencies

* _ESP8266 Arduino board manager_ ([link](https://github.com/esp8266/Arduino)). Allows to develop and flash ESP8266-based boards with Arduino IDE 
* _WiFiManager for ESP8266 boards_ ([link](https://github.com/tzapu/WiFiManager)). Enables simple WiFi configuration for your projects. Allows to configure other custom parameters
* _PubSubClient_ library for MQTT support
* _Base64_ Arduino library for decoding received data
* _IRremoteESP8266_ library
* _EEPROM_ library for storing settings in EEPROM

### Changelog
**v.3.1**
* Add 5V curret detection on factory USB port
**v.3.0**
* WiFiManager integrated with additional config enries for MQTT connection settings.
* Write and read settings to/from EEPROM
