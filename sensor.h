#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

struct SensorData {
  float temperature = NAN;   // deg C (BME280)
  float humidity    = NAN;   // % RH (BME280)
  float pressure    = NAN;   // hPa (BME280)
  float altitude    = NAN;   // m, estimated from pressure (BME280)
  float lux         = NAN;   // BH1750
  int   smokeRaw    = 0;     // MQ2
  int   gasRaw      = 0;     // MQ5
  int   waterRaw    = 0;     // water level
  bool  motion      = false; // PIR
  bool  bmeOk       = false;
  bool  luxOk       = false;
};

extern SensorData sensorData;

void sensor_init();
void sensor_read();     // call on SENSOR_INTERVAL_MS - reads + publishes everything
void sensor_evaluateSafety(); // checks smoke/gas/water thresholds and raises/clears alarms

#endif // SENSOR_H
