// ==================================================================
// SmartHomeAutomation.ino
// Professional ESP32 Smart Home Automation - modular version
//
// Modules:
//   pins.h / config.h / secrets.h  - configuration
//   relay      - actuator control (lights, fans, door, pump)
//   alarm      - buzzer + edge-triggered alarm state machine
//   wifi       - non-blocking WiFi connection manager
//   mqtt       - non-blocking MQTT connection + command dispatch
//   rfid       - multi-card RFID access control (NVS-backed)
//   sensor     - BME280 + BH1750 + MQ2 + MQ5 + water + PIR
//   power      - PZEM-004T energy monitoring
//   automation - the actual rule engine (climate/lighting/pump/security)
//   errors     - centralized fault/error flag aggregation
//   display    - OLED status dashboard
//   webserver  - local web dashboard with basic auth
//   switches   - manual physical override switches (works without WiFi)
//
// Required libraries (Library Manager):
//   PubSubClient, MFRC522, Adafruit BME280 Library, Adafruit Unified Sensor,
//   BH1750 (by Claws), PZEM004Tv30, Adafruit SSD1306, Adafruit GFX Library
// ==================================================================

#include <esp_task_wdt.h>
#include "config.h"
#include "pins.h"

#include "relay.h"
#include "alarm.h"
#include "wifi.h"
#include "mqtt.h"
#include "rfid.h"
#include "sensor.h"
#include "power.h"
#include "automation.h"
#include "errors.h"
#include "display.h"
#include "webserver.h"
#include "switches.h"

unsigned long lastSensorRead = 0;
unsigned long lastPowerRead  = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[SYSTEM] Initializing ESP32 Smart Home Node...");

  esp_task_wdt_init(WDT_TIMEOUT_SEC, true);
  esp_task_wdt_add(NULL);

  errors_init();
  relay_init();
  alarm_init();
  automation_init();
  rfid_init();
  sensor_init();
  power_init();

  wifi_init();
  mqtt_init();

#if FEATURE_OLED
  display_init();
#endif
#if FEATURE_WEBSERVER
  webserver_init();
#endif
#if FEATURE_MANUAL_SW
  switches_init();
#endif

  Serial.println("[SYSTEM] System Ready.");
}

void loop() {
  esp_task_wdt_reset();

  unsigned long now = millis();

  // ---- Networking (fully non-blocking) ----
  wifi_service();
  mqtt_service();

  // ---- Periodic sensor + power polling & automation ----
  if (now - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = now;
    sensor_read();
    sensor_evaluateSafety();
    automation_run();
  }

  if (now - lastPowerRead >= POWER_INTERVAL_MS) {
    lastPowerRead = now;
    power_read();
  }

  // ---- Fast-loop tasks ----
  rfid_service();
  alarm_service();
  relay_serviceDoorAutoLock();

#if FEATURE_MANUAL_SW
  switches_service();
#endif
#if FEATURE_OLED
  display_service();
#endif
#if FEATURE_WEBSERVER
  webserver_service();
#endif
}
