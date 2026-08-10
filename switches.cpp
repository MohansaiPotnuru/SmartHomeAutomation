#include "switches.h"
#include "config.h"

#if FEATURE_MANUAL_SW

#include "pins.h"
#include "relay.h"

struct SwitchState {
  uint8_t pin;
  bool lastReading;
  unsigned long lastChangeTime;
};

static SwitchState swLight1 = { SWITCH_LIGHT_1, HIGH, 0 };
static SwitchState swLight2 = { SWITCH_LIGHT_2, HIGH, 0 };
static SwitchState swFan1   = { SWITCH_FAN_1,   HIGH, 0 };
static SwitchState swDoor   = { SWITCH_DOOR,    HIGH, 0 };

static unsigned long lastPoll = 0;

void switches_init() {
  pinMode(SWITCH_LIGHT_1, INPUT_PULLUP);
  pinMode(SWITCH_LIGHT_2, INPUT_PULLUP);
  pinMode(SWITCH_FAN_1, INPUT_PULLUP);
  pinMode(SWITCH_DOOR, INPUT_PULLUP);
}

// Simple, reliable edge-triggered debounce: toggle only on a LOW reading that
// wasn't already LOW last poll, gated by a minimum time since the last toggle.
static void pollToggle(SwitchState* sw, void (*onToggle)()) {
  bool reading = digitalRead(sw->pin);
  unsigned long now = millis();

  if (reading == LOW && sw->lastReading == HIGH && (now - sw->lastChangeTime) > SWITCH_DEBOUNCE_MS) {
    sw->lastChangeTime = now;
    onToggle();
  }
  sw->lastReading = reading;
}

static void toggleLight1() { relay_setLight1(!relay_getLight1(), false); }
static void toggleLight2() { relay_setLight2(!relay_getLight2(), false); }
static void toggleFan1()   { relay_setFan1(!relay_getFan1(), false); }
static void toggleDoor() {
  if (relay_isDoorUnlocked()) relay_setDoorLock(false);
  else relay_setDoorLock(true);
}

void switches_service() {
  unsigned long now = millis();
  if (now - lastPoll < SWITCH_POLL_MS) return;
  lastPoll = now;

  pollToggle(&swLight1, toggleLight1);
  pollToggle(&swLight2, toggleLight2);
  pollToggle(&swFan1, toggleFan1);
  pollToggle(&swDoor, toggleDoor);
}

#else

void switches_init() {}
void switches_service() {}

#endif // FEATURE_MANUAL_SW
