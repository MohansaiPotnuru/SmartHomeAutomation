#ifndef POWER_H
#define POWER_H

#include <Arduino.h>

struct PowerData {
  float voltage    = NAN;
  float current    = NAN;
  float power      = NAN;
  float energyKwh  = NAN;
  float frequency  = NAN;
  float pf         = NAN;
  bool  ok         = false;
};

extern PowerData powerData;

void power_init();
void power_read();  // call on POWER_INTERVAL_MS

#endif // POWER_H
