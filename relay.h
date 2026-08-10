#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

// Tracks whether each actuator's current ON state was set by the
// automation engine (true) or by manual/MQTT command (false).
// Automation should never fight a manual override.
extern bool fan1AutoOn;
extern bool light1AutoOn;
extern bool light2AutoOn;
extern bool pumpAutoOn;

void relay_init();

void relay_setLight1(bool on, bool isAuto);
void relay_setLight2(bool on, bool isAuto);
void relay_setFan1(bool on, bool isAuto);
void relay_setFan2(bool on, bool isAuto);
void relay_setPump(bool on, bool isAuto);

void relay_setDoorLock(bool unlocked);
bool relay_isDoorUnlocked();
void relay_serviceDoorAutoLock(); // call every loop - handles the 5s auto-relock

bool relay_getLight1();
bool relay_getLight2();
bool relay_getFan1();
bool relay_getFan2();
bool relay_getPump();

#endif // RELAY_H
