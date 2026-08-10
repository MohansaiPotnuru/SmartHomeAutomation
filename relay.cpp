#include "relay.h"
#include "pins.h"
#include "config.h"
#include "mqtt.h"

bool fan1AutoOn    = false;
bool light1AutoOn  = false;
bool light2AutoOn  = false;
bool pumpAutoOn    = false;

static bool doorUnlocked = false;
static unsigned long doorUnlockStart = 0;

void relay_init() {
  pinMode(RELAY_LIGHT_1, OUTPUT);
  pinMode(RELAY_LIGHT_2, OUTPUT);
  pinMode(RELAY_FAN_1, OUTPUT);
  pinMode(RELAY_FAN_2, OUTPUT);
  pinMode(RELAY_DOOR_LOCK, OUTPUT);
  pinMode(RELAY_PUMP, OUTPUT);

  digitalWrite(RELAY_LIGHT_1, RELAY_OFF);
  digitalWrite(RELAY_LIGHT_2, RELAY_OFF);
  digitalWrite(RELAY_FAN_1, RELAY_OFF);
  digitalWrite(RELAY_FAN_2, RELAY_OFF);
  digitalWrite(RELAY_DOOR_LOCK, RELAY_OFF);
  digitalWrite(RELAY_PUMP, RELAY_OFF);
}

void relay_setLight1(bool on, bool isAuto) {
  digitalWrite(RELAY_LIGHT_1, on ? RELAY_ON : RELAY_OFF);
  light1AutoOn = isAuto && on;
  mqtt_publish(TOPIC_LIGHT1, on ? "ON" : "OFF", true);
}

void relay_setLight2(bool on, bool isAuto) {
  digitalWrite(RELAY_LIGHT_2, on ? RELAY_ON : RELAY_OFF);
  light2AutoOn = isAuto && on;
  mqtt_publish(TOPIC_LIGHT2, on ? "ON" : "OFF", true);
}

void relay_setFan1(bool on, bool isAuto) {
  digitalWrite(RELAY_FAN_1, on ? RELAY_ON : RELAY_OFF);
  fan1AutoOn = isAuto && on;
  mqtt_publish(TOPIC_FAN1, on ? "ON" : "OFF", true);
}

void relay_setFan2(bool on, bool isAuto) {
  digitalWrite(RELAY_FAN_2, on ? RELAY_ON : RELAY_OFF);
  mqtt_publish(TOPIC_FAN2, on ? "ON" : "OFF", true);
}

void relay_setPump(bool on, bool isAuto) {
  digitalWrite(RELAY_PUMP, on ? RELAY_ON : RELAY_OFF);
  pumpAutoOn = isAuto && on;
  mqtt_publish(TOPIC_PUMP, on ? "ON" : "OFF", true);
}

void relay_setDoorLock(bool unlock) {
  digitalWrite(RELAY_DOOR_LOCK, unlock ? RELAY_ON : RELAY_OFF);
  doorUnlocked = unlock;
  if (unlock) doorUnlockStart = millis();
  mqtt_publish(TOPIC_DOOR, unlock ? "UNLOCKED" : "LOCKED", true);
}

bool relay_isDoorUnlocked() { return doorUnlocked; }

void relay_serviceDoorAutoLock() {
  if (doorUnlocked && (millis() - doorUnlockStart >= DOOR_UNLOCK_TIME_MS)) {
    relay_setDoorLock(false);
  }
}

bool relay_getLight1() { return digitalRead(RELAY_LIGHT_1) == RELAY_ON; }
bool relay_getLight2() { return digitalRead(RELAY_LIGHT_2) == RELAY_ON; }
bool relay_getFan1()   { return digitalRead(RELAY_FAN_1) == RELAY_ON; }
bool relay_getFan2()   { return digitalRead(RELAY_FAN_2) == RELAY_ON; }
bool relay_getPump()   { return digitalRead(RELAY_PUMP) == RELAY_ON; }
