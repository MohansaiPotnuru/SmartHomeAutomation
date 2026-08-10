# ESP32 Smart Home Automation - v2 (Modular / Professional Build)

This is the upgraded version of the original single-file `claude.ino`, restructured
into modules and expanded with the sensors, security, monitoring, and interface
features from the review.

## What changed vs the original single-file version

| Area | Before | Now |
|---|---|---|
| File structure | One 510-line `.ino` | 13 modules with clear responsibilities |
| Environment sensing | DHT22 (temp/humidity) | BME280 (temp/humidity/**pressure**/**altitude**) |
| Light | none | BH1750 lux sensor + auto night-light rule |
| Gas safety | MQ2 only | MQ2 (smoke) + **MQ5 (LPG/gas)** |
| Power | none | **PZEM-004T** (voltage/current/power/energy/frequency/PF) |
| Display | none | SSD1306 OLED, 4 rotating status pages |
| Web control | none | Local web dashboard (basic-auth protected) |
| Manual control | none | Physical pushbuttons for light/fan/door - works with WiFi down |
| RFID | 1 hardcoded card | Multiple cards with Admin/User/Guest roles, stored in NVS, provisionable |
| Alarms | Retained MQTT message that never cleared, republished every 3s | Edge-triggered: publishes once on trigger, once on clear, retained "state" topic always reflects current truth |
| Fan control | Turned on above 35C, never turned back off | Hysteresis (ON > 35C, OFF < 32C) |
| Fault detection | none | Centralized `errors` module tracks BME280/BH1750/PZEM/RFID/WiFi/MQTT health, published to `home/system/error` |
| Credentials | Hardcoded in the sketch | Isolated in `secrets.h` (gitignore this file) |
| Networking | Blocking `delay()` reconnect loops | Fully non-blocking WiFi/MQTT state machines |

## File structure

```
SmartHomeAutomation/
  SmartHomeAutomation.ino   - setup()/loop(), wires all modules together
  config.h                  - thresholds, timing, MQTT topics, feature toggles
  pins.h                    - all GPIO assignments in one place
  secrets.h                 - WiFi/MQTT/web credentials + default RFID cards (KEEP OUT OF GIT)
  relay.h / relay.cpp       - actuator control (lights, fans, door, pump)
  alarm.h / alarm.cpp       - non-blocking buzzer + edge-triggered alarm state machine
  wifi.h / wifi.cpp         - non-blocking WiFi connect/reconnect
  mqtt.h / mqtt.cpp         - non-blocking MQTT connect + command dispatch + safe publish helpers
  rfid.h / rfid.cpp         - multi-card RFID access control, roles stored in NVS
  sensor.h / sensor.cpp     - BME280 + BH1750 + MQ2 + MQ5 + water level + PIR
  power.h / power.cpp       - PZEM-004T energy monitoring
  automation.h / automation.cpp - the rule engine (climate/lighting/pump/security)
  errors.h / errors.cpp     - centralized fault-flag aggregation
  display.h / display.cpp   - OLED status dashboard (4 rotating pages)
  webserver.h / webserver.cpp - local web dashboard with basic auth
  switches.h / switches.cpp - manual physical override switches
```

## Required Arduino libraries (install via Library Manager)

- `PubSubClient` (Nick O'Leary)
- `MFRC522` (GithubCommunity / miguelbalboa)
- `Adafruit BME280 Library` + `Adafruit Unified Sensor`
- `BH1750` (by Christopher Laws / claws)
- `PZEM004Tv30` (by Jakub Mandula)
- `Adafruit SSD1306` + `Adafruit GFX Library`

Board package: **ESP32 by Espressif Systems** (Boards Manager).

## Wiring summary (see `pins.h` for the authoritative list)

| Function | GPIO |
|---|---|
| Light 1 / Light 2 relay | 13 / 14 |
| Fan 1 / Fan 2 relay | 16 / 17 |
| Door lock relay | 21 |
| Water pump relay | 25 |
| Buzzer | 27 |
| I2C SDA / SCL (BME280, BH1750, OLED - shared bus) | 32 / 33 |
| PIR motion | 26 |
| MQ2 (smoke) | 34 |
| MQ5 (gas) | 36 |
| Water level (analog) | 35 |
| RFID SS / RST (SPI) | 5 / 22 |
| PZEM-004T RX / TX (ESP32 Serial2) | 4 / 2 |
| Manual switch: Light1 / Light2 / Fan1 / Door | 18 / 19 / 23 / 15 |

All manual switches are wired to a button between the pin and GND; the code
uses `INPUT_PULLUP`, so no external resistors are needed.

## Configuring before first upload

1. Open `secrets.h` and set your real `WIFI_SSID`, `WIFI_PASSWORD`, `MQTT_SERVER`,
   web dashboard login, and your actual RFID card UIDs (scan a card once with
   `Serial.println` enabled to read its UID, then update `DEFAULT_CARDS`).
2. Add `secrets.h` to `.gitignore` if you put this project in version control.
3. If a sensor module isn't physically connected yet, flip its toggle to `0`
   in `config.h` (e.g. `#define FEATURE_PZEM 0`) rather than deleting code -
   the rest of the system keeps running normally.
4. First boot writes the default RFID cards into NVS. After that, `secrets.h`'s
   `DEFAULT_CARDS` list is ignored unless you call `rfid_factoryReset()`.

## Security notes carried over from the review

- MQTT still has no TLS/auth configured by default (`MQTT_USER`/`MQTT_PASS`
  are empty in `secrets.h`) - set real credentials and enable TLS on your
  broker before exposing this beyond a trusted LAN, since `home/door` and
  `home/arm` can unlock the door / disarm the system.
- The web dashboard uses HTTP Basic Auth only (no TLS) - fine on a private
  LAN, not fine on the open internet without a reverse proxy/VPN.
- RFID lockout (5 failed scans -> 30s reader lockout) slows brute forcing but
  does not stop a determined attacker with physical access to the reader.

## Known limitations / next steps

- The web dashboard is a single static page with a manual refresh timer, not
  a live WebSocket feed - fine for a demo, worth upgrading if this becomes a
  real deployment.
- RFID logs (`home/security/rfid`, `home/system/log`) are only as durable as
  your MQTT broker's retention/logging setup - pair with Node-RED, InfluxDB,
  or a simple subscriber script if you need permanent history for a report.
- `PZEM_TX_PIN` (GPIO2) is an ESP32 strapping pin. It behaves normally once
  the board has booted, but avoid grounding it externally during power-on.

## Wi-Fi Layer
Handles ESP32 Wi-Fi initialization and connection management.
