#include "automation.h"
#include <Preferences.h>
#include "config.h"
#include "sensor.h"
#include "relay.h"
#include "alarm.h"
#include "mqtt.h"

bool systemArmed = true;
static Preferences prefs;

void automation_init() {
  prefs.begin("smarthome", false);
  systemArmed = prefs.getBool("armed", true);
  prefs.end();
}

void security_setArmed(bool armed) {
  systemArmed = armed;
  prefs.begin("smarthome", false);
  prefs.putBool("armed", systemArmed);
  prefs.end();
  mqtt_publish(TOPIC_LOG, systemArmed ? "System ARMED" : "System DISARMED", false);
  Serial.printf("[SECURITY] Armed state: %s\n", systemArmed ? "true" : "false");
}

// ---- Rule: Temperature -> Fan 1 (with hysteresis) ----
// Also force-on for smoke/gas ventilation, overriding the temp reading.
static void rule_climateControl() {
  bool ventilationNeeded = smokeAlarmActive || gasAlarmActive;

  if (ventilationNeeded) {
    if (!relay_getFan1()) relay_setFan1(true, true);
    return;
  }

  if (isnan(sensorData.temperature)) return; // BME280 not available - skip rule safely

  if (sensorData.temperature > FAN_ON_TEMP && !relay_getFan1()) {
    relay_setFan1(true, true);
  } else if (sensorData.temperature < FAN_OFF_TEMP && relay_getFan1() && fan1AutoOn) {
    relay_setFan1(false, true);
  }
}

// ---- Rule: Ambient light -> Light 1 (auto night-light) ----
static void rule_autoLighting() {
  if (isnan(sensorData.lux)) return; // BH1750 not available

  if (sensorData.lux < LUX_DARK_THRESHOLD && !relay_getLight1()) {
    relay_setLight1(true, true);
  } else if (sensorData.lux >= LUX_DARK_THRESHOLD && relay_getLight1() && light1AutoOn) {
    relay_setLight1(false, true);
  }
}

// ---- Rule: Water tank level -> Pump ----
static void rule_waterPump() {
  if (sensorData.waterRaw < WATER_EMPTY_THRESH && !relay_getPump()) {
    relay_setPump(true, true);
    mqtt_publish(TOPIC_LOG, "Tank low - pump auto-started.", false);
  } else if (sensorData.waterRaw > WATER_FULL_THRESH && relay_getPump() && pumpAutoOn) {
    relay_setPump(false, true);
    mqtt_publish(TOPIC_LOG, "Tank full - pump auto-stopped.", false);
  }
}

// ---- Rule: Motion while armed -> intrusion alarm ----
static void rule_intrusion() {
  if (sensorData.motion && systemArmed) {
    alarm_trigger("ALERT: Motion detected while system armed!", "MOTION", &motionAlarmActive);
  } else {
    alarm_clear("MOTION", &motionAlarmActive);
  }
}

void automation_run() {
  rule_climateControl();
  rule_autoLighting();
  rule_waterPump();
  rule_intrusion();
}
