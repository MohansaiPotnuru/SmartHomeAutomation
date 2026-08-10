#include "sensor.h"
#include <Wire.h>
#include "pins.h"
#include "config.h"
#include "mqtt.h"
#include "alarm.h"
#include "errors.h"

#if FEATURE_BME280
  #include <Adafruit_Sensor.h>
  #include <Adafruit_BME280.h>
  static Adafruit_BME280 bme;
#endif

#if FEATURE_BH1750
  #include <BH1750.h>
  static BH1750 lightMeter;
#endif

SensorData sensorData;

void sensor_init() {
  Wire.begin(I2C_SDA, I2C_SCL);

  pinMode(PIR_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);
#if FEATURE_MQ5
  pinMode(MQ5_PIN, INPUT);
#endif
  pinMode(WATER_PIN, INPUT);

#if FEATURE_BME280
  sensorData.bmeOk = bme.begin(0x76) || bme.begin(0x77);
  if (!sensorData.bmeOk) {
    Serial.println("[SENSOR] WARNING: BME280 not found at 0x76 or 0x77.");
  }
  errors_setFlag(ERR_BME280, !sensorData.bmeOk);
#endif

#if FEATURE_BH1750
  sensorData.luxOk = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  if (!sensorData.luxOk) {
    Serial.println("[SENSOR] WARNING: BH1750 not detected.");
  }
  errors_setFlag(ERR_BH1750, !sensorData.luxOk);
#endif
}

void sensor_read() {
#if FEATURE_BME280
  if (sensorData.bmeOk) {
    sensorData.temperature = bme.readTemperature();
    sensorData.humidity    = bme.readHumidity();
    sensorData.pressure    = bme.readPressure() / 100.0F; // Pa -> hPa
    sensorData.altitude    = bme.readAltitude(1013.25);   // sea-level hPa reference

    mqtt_publish(TOPIC_TEMP, sensorData.temperature, 1, true);
    mqtt_publish(TOPIC_HUM, sensorData.humidity, 1, true);
    mqtt_publish(TOPIC_PRESSURE, sensorData.pressure, 1, true);
    mqtt_publish(TOPIC_ALTITUDE, sensorData.altitude, 1, true);
  }
#endif

#if FEATURE_BH1750
  if (sensorData.luxOk) {
    sensorData.lux = lightMeter.readLightLevel();
    mqtt_publish(TOPIC_LUX, sensorData.lux, 0, true);
  }
#endif

  sensorData.smokeRaw = analogRead(MQ2_PIN);
  mqtt_publish(TOPIC_SMOKE, sensorData.smokeRaw, false);

#if FEATURE_MQ5
  sensorData.gasRaw = analogRead(MQ5_PIN);
  mqtt_publish(TOPIC_GAS, sensorData.gasRaw, false);
#endif

  sensorData.waterRaw = analogRead(WATER_PIN);
  mqtt_publish(TOPIC_WATER, sensorData.waterRaw, false);

  sensorData.motion = digitalRead(PIR_PIN) == HIGH;
  mqtt_publish(TOPIC_MOTION, sensorData.motion ? "ON" : "OFF", false);
}

void sensor_evaluateSafety() {
  if (sensorData.smokeRaw > SMOKE_THRESHOLD) {
    alarm_trigger("CRITICAL: Smoke detected!", "SMOKE", &smokeAlarmActive);
  } else {
    alarm_clear("SMOKE", &smokeAlarmActive);
  }

#if FEATURE_MQ5
  if (sensorData.gasRaw > GAS_THRESHOLD) {
    alarm_trigger("CRITICAL: LPG / gas leak detected!", "GAS", &gasAlarmActive);
  } else {
    alarm_clear("GAS", &gasAlarmActive);
  }
#endif

  if (sensorData.waterRaw > WATER_FULL_THRESH) {
    alarm_trigger("WARNING: Water overflow detected!", "WATER", &waterAlarmActive);
  } else {
    alarm_clear("WATER", &waterAlarmActive);
  }
}
