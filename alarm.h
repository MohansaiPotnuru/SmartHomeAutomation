#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>

void alarm_init();
void alarm_service();               // call every loop - services the non-blocking buzzer

void alarm_startBuzzer(unsigned long durationMs);

// Each condition is independent and edge-triggered: publishes once when it
// becomes active, once when it clears, never spams on every sensor cycle.
void alarm_trigger(const char* reason, const char* stateName, bool* activeFlag);
void alarm_clear(const char* stateName, bool* activeFlag);

extern bool smokeAlarmActive;
extern bool gasAlarmActive;
extern bool waterAlarmActive;
extern bool motionAlarmActive;

bool alarm_anyActive();

#endif // ALARM_H
