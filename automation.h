#ifndef AUTOMATION_H
#define AUTOMATION_H

#include <Arduino.h>

extern bool systemArmed;

void automation_init();          // loads "armed" state from NVS
void automation_run();           // call once per SENSOR_INTERVAL_MS tick, after sensor_read()
void security_setArmed(bool armed);

#endif // AUTOMATION_H
