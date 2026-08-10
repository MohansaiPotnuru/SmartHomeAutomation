#include "alarm.h"
#include "pins.h"
#include "config.h"
#include "mqtt.h"

bool smokeAlarmActive  = false;
bool gasAlarmActive    = false;
bool waterAlarmActive  = false;
bool motionAlarmActive = false;

static bool buzzerActive = false;
static unsigned long buzzerStart = 0;
static unsigned long buzzerDuration = 0;

void alarm_init() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void alarm_startBuzzer(unsigned long durationMs) {
  digitalWrite(BUZZER_PIN, HIGH);
  buzzerActive = true;
  buzzerStart = millis();
  buzzerDuration = durationMs;
}

void alarm_service() {
  if (buzzerActive && (millis() - buzzerStart >= buzzerDuration)) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
  }
}

static void publishAggregateState() {
  const char* state = "CLEAR";
  if (smokeAlarmActive)       state = "SMOKE";
  else if (gasAlarmActive)    state = "GAS";
  else if (waterAlarmActive)  state = "WATER";
  else if (motionAlarmActive) state = "MOTION";
  mqtt_publish(TOPIC_ALARM_STATE, state, true);
}

void alarm_trigger(const char* reason, const char* stateName, bool* activeFlag) {
  if (*activeFlag) return; // already latched active, don't re-publish every cycle
  *activeFlag = true;

  Serial.printf("[ALARM] %s\n", reason);
  mqtt_publish(TOPIC_ALARM, reason, false); // transient event, NOT retained
  mqtt_publish(TOPIC_LOG, reason, false);
  publishAggregateState();
  alarm_startBuzzer(TONE_ALARM_MS);
}

void alarm_clear(const char* stateName, bool* activeFlag) {
  if (!*activeFlag) return;
  *activeFlag = false;
  Serial.printf("[ALARM] %s condition cleared.\n", stateName);
  publishAggregateState();
}

bool alarm_anyActive() {
  return smokeAlarmActive || gasAlarmActive || waterAlarmActive || motionAlarmActive;
}
