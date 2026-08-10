#ifndef ERRORS_H
#define ERRORS_H

#include <Arduino.h>

enum ErrorFlag {
  ERR_BME280 = 0,
  ERR_BH1750,
  ERR_PZEM,
  ERR_RFID,
  ERR_WIFI,
  ERR_MQTT,
  ERR_COUNT
};

void errors_init();
void errors_setFlag(ErrorFlag flag, bool active);
bool errors_anyActive();
const char* errors_getSummary(); // e.g. "BME280,PZEM" or "OK" - retained on TOPIC_ERROR
void errors_publishIfChanged();

#endif // ERRORS_H
