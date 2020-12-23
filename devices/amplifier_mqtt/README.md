# MQTT controlled integrated stereo amplifier

Controlls the power of Cambridge Audio AXA25 amplifier simulating short factory button click with relay. Detects the state of amplifier by detecting the curret on factory USB port.

### Components
* Cambridge Audio AXA25
* LOLIN (Wemos) D1 mini ([wemos.cc](https://www.wemos.cc/en/latest/d1/d1_mini.html))
* 5V Relay Brick by Itead ([itead.com](https://www.itead.cc/electronic-brick-5v-relay.html))
* AC-DC 220V to 5V Step-Down Mini Power Supply ([amazon](https://www.amazon.com/HLK-PM01-supply-module-intelligent-household/dp/B07G5GL4B8))

![](https://github.com/estevez-dev/edwin-home/raw/master/devices/amplifier_mqtt/amplifier_mqtt.png)

### Dependencies

* _ESP8266 Arduino board manager_ ([link](https://github.com/esp8266/Arduino)). Allows to develop and flash ESP8266-based boards with Arduino IDE 
* _WiFiManager for ESP8266 boards_ ([link](https://github.com/tzapu/WiFiManager)). Enables simple WiFi configuration for your projects. Allows to configure other custom parameters
* _PubSubClient_ library for MQTT support
* _EEPROM_ library for storing settings in EEPROM

### ESPHome
See [amplifier.yaml](/devices/amplifier_mqtt/amplifier.yaml) for ESPHome config.

### Changelog
**v.2.0**
- Migrated to ESPHome

**v.1.1**
- Switch WiFi to STA mode
- Ignore "turn on" command if factory USB is powered on
