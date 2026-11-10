#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// FEATURE TOGGLES
// Flip these off if a particular sensor module isn't physically
// wired up yet - the rest of the system keeps working.
// ==========================================
#define FEATURE_BME280      1
#define FEATURE_BH1750      1
#define FEATURE_MQ5         1
#define FEATURE_PZEM        1
#define FEATURE_OLED        1
#define FEATURE_WEBSERVER   1
#define FEATURE_MANUAL_SW   1

// ==========================================
// SYSTEM TIMING
// ==========================================
#define WDT_TIMEOUT_SEC         8
#define DOOR_UNLOCK_TIME_MS     5000
#define SENSOR_INTERVAL_MS      3000
#define DISPLAY_INTERVAL_MS     1500
#define POWER_INTERVAL_MS       2000
#define SWITCH_POLL_MS          50
#define SWITCH_DEBOUNCE_MS      250

#define WIFI_CHECK_INTERVAL_MS      10000
#define WIFI_CONNECT_TIMEOUT_MS     10000
#define MQTT_RETRY_INTERVAL_MS      5000

// ==========================================
// THRESHOLDS
// ==========================================
#define SMOKE_THRESHOLD     2000   // MQ2 raw ADC (0-4095)
#define GAS_THRESHOLD       2000   // MQ5 raw ADC (0-4095)
#define WATER_FULL_THRESH   1500   // tank full -> pump auto-OFF
#define WATER_OVERFLOW_THRESH 1800 // well above "full" -> overflow alarm
#define WATER_EMPTY_THRESH  300    // below this = tank considered empty -> pump ON

#define FAN_ON_TEMP         35.0f
#define FAN_OFF_TEMP        32.0f

#define LUX_DARK_THRESHOLD  100.0f  // below this -> lights auto ON at night

// ==========================================
// TONE DURATIONS (non-blocking buzzer)
// ==========================================
#define TONE_SHORT_MS       100
#define TONE_REJECT_MS      150
#define TONE_ALARM_MS       300

// ==========================================
// RFID
// ==========================================
#define RFID_MAX_FAILS      5
#define RFID_LOCKOUT_MS     30000
#define RFID_MAX_CARDS      10   // stored in Preferences (admin/guest/user roles)

// ==========================================
// MQTT TOPICS
// ==========================================
// Actuator commands (subscribed). Payloads: ON / OFF, door: UNLOCK / LOCK,
// arm: ON / OFF (or TRUE / FALSE).
#define TOPIC_LIGHT1        "home/light1"
#define TOPIC_LIGHT2        "home/light2"
#define TOPIC_FAN1          "home/fan1"
#define TOPIC_FAN2          "home/fan2"
#define TOPIC_DOOR          "home/door"
#define TOPIC_ARM           "home/arm"
#define TOPIC_PUMP          "home/pump"

// Actuator state (published, retained). Kept on separate topics so the node
// never receives its own state messages back as commands.
#define TOPIC_LIGHT1_STATE  "home/light1/state"
#define TOPIC_LIGHT2_STATE  "home/light2/state"
#define TOPIC_FAN1_STATE    "home/fan1/state"
#define TOPIC_FAN2_STATE    "home/fan2/state"
#define TOPIC_DOOR_STATE    "home/door/state"
#define TOPIC_ARM_STATE     "home/arm/state"
#define TOPIC_PUMP_STATE    "home/pump/state"

// Sensors / telemetry (published)
#define TOPIC_TEMP          "home/env/temperature"
#define TOPIC_HUM           "home/env/humidity"
#define TOPIC_PRESSURE      "home/env/pressure"
#define TOPIC_ALTITUDE      "home/env/altitude"
#define TOPIC_LUX           "home/env/lux"
#define TOPIC_SMOKE         "home/safety/smoke"
#define TOPIC_GAS           "home/safety/gas"
#define TOPIC_WATER         "home/safety/waterlevel"
#define TOPIC_MOTION        "home/security/motion"
#define TOPIC_RFID          "home/security/rfid"
#define TOPIC_RFID_ADMIN    "home/security/rfid/admin"   // publish CARD:ROLE for provisioning

// Power monitoring
#define TOPIC_VOLTAGE       "home/power/voltage"
#define TOPIC_CURRENT       "home/power/current"
#define TOPIC_POWER         "home/power/watts"
#define TOPIC_ENERGY        "home/power/energy_kwh"
#define TOPIC_FREQUENCY     "home/power/frequency"
#define TOPIC_PF            "home/power/powerfactor"

// System / status
#define TOPIC_STATUS        "home/status"
#define TOPIC_ALARM         "home/alarm"          // transient event log, not retained
#define TOPIC_ALARM_STATE   "home/alarm/state"    // retained: CLEAR / SMOKE / GAS / WATER / MOTION
#define TOPIC_ERROR         "home/system/error"   // retained: comma list of active faults, or "OK"
#define TOPIC_LOG           "home/system/log"     // event log stream (door/RFID/power events)

#endif // CONFIG_H
