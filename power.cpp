#include "power.h"
#include "config.h"

#if FEATURE_PZEM
  #include <PZEM004Tv30.h>
  #include "pins.h"
  // Uses HardwareSerial 2 on the ESP32
  static PZEM004Tv30 pzem(Serial2, PZEM_RX_PIN, PZEM_TX_PIN);
#endif

#include "mqtt.h"
#include "errors.h"

PowerData powerData;

void power_init() {
#if FEATURE_PZEM
  // PZEM004Tv30 constructor above already opens Serial2 internally.
  Serial.println("[POWER] PZEM-004T initialized.");
#endif
}

void power_read() {
#if FEATURE_PZEM
  float v = pzem.voltage();
  float i = pzem.current();
  float p = pzem.power();
  float e = pzem.energy();
  float f = pzem.frequency();
  float pf = pzem.pf();

  powerData.ok = !isnan(v);
  if (!powerData.ok) {
    errors_setFlag(ERR_PZEM, true);
    Serial.println("[POWER] WARNING: PZEM-004T read failed (check wiring).");
    return;
  }
  errors_setFlag(ERR_PZEM, false);

  powerData.voltage   = v;
  powerData.current   = i;
  powerData.power     = p;
  powerData.energyKwh = e;
  powerData.frequency = f;
  powerData.pf        = pf;

  mqtt_publish(TOPIC_VOLTAGE, v, 1, true);
  mqtt_publish(TOPIC_CURRENT, i, 2, true);
  mqtt_publish(TOPIC_POWER, p, 1, true);
  mqtt_publish(TOPIC_ENERGY, e, 3, true);
  mqtt_publish(TOPIC_FREQUENCY, f, 1, true);
  mqtt_publish(TOPIC_PF, pf, 2, true);

  // A mains dropout mid-measurement is worth a log line - useful for a
  // "power failure" event in the final-year report.
  static bool wasPowered = true;
  bool isPowered = v > 10.0f; // PZEM reports ~0V with no mains present
  if (wasPowered && !isPowered) {
    mqtt_publish(TOPIC_LOG, "Mains power loss detected (PZEM reads 0V).", false);
  } else if (!wasPowered && isPowered) {
    mqtt_publish(TOPIC_LOG, "Mains power restored.", false);
  }
  wasPowered = isPowered;
#endif
}
