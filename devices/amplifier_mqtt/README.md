# MQTT controlled integrated stereo amplifier

Controlls the power of Cambridge Audio AXA25 amplifier simulating short factory button click with relay. Detects the state of amplifier by detecting the curret on factory USB port.

### See [bulding guide](https://sometechy.website/diy-smart-appliance-adding-wifi-to-common-sound-amplifier) for components and other info.

### Dependencies

* _ESP8266 Arduino board manager_ ([link](https://github.com/esp8266/Arduino)). Allows to develop and flash ESP8266-based boards with Arduino IDE 
* _WiFiManager for ESP8266 boards_ ([link](https://github.com/tzapu/WiFiManager)). Enables simple WiFi configuration for your projects. Allows to configure other custom parameters
* _PubSubClient_ library for MQTT support
* _EEPROM_ library for storing settings in EEPROM

### ESPHome
See [bulding guide](/devices/amplifier_mqtt/amplifier.yaml) for ESPHome config.
