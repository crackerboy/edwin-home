# MQTT Humidifier

Controls factory ulatrasonic transducrer with relay. Takes power from 12V DC output found on factory board. Read the state of factory switch and sends it's state in info topic.

### Components
* Ballu UHB-200 air humidifier
* LOLIN (Wemos) D1 mini ([wemos.cc](https://www.wemos.cc/en/latest/d1/d1_mini.html))
* 5V Relay Brick by Itead ([itead.com](https://www.itead.cc/electronic-brick-5v-relay.html))
* 12V to 5V DC converter
* 10K ohm pull-down resistor

![](https://github.com/estevez-dev/edwin-home/raw/master/devices/humidifier_mqtt/humidifier_mqtt.png)

### Dependencies

* _ESP8266 Arduino board manager_ ([link](https://github.com/esp8266/Arduino)). Allows to develop and flash ESP8266-based boards with Arduino IDE. 
* _WiFiManager for ESP8266 boards_ ([link](https://github.com/tzapu/WiFiManager)). Enables simple WiFi configuration for your projects. Allows to configure other custom parameters.
* _PubSubClient_ library for MQTT support.
* _EEPROM_ library for storing settings in EEPROM.

### [Building story on some techy website](https://sometechy.website/how-to-make-wifi-enabled-smart-humidifier-from-a-regular-one)

### Changelog
**v.2.1**
* Add availability information
* Add info topic with switch state, version and IP address
* Disable relay state to always be the same as switch state

**v.2.0**
* Added OEM hardware switch support on D5 pin
* WiFiManager integrated with additional config enries for MQTT connection settings.
* Write and read settings to/from EEPROM
