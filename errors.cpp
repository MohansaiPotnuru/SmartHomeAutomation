#include "errors.h"
#include "config.h"
#include "mqtt.h"

static bool flags[ERR_COUNT] = { false };
static char summaryBuf[96];
static char lastPublished[96] = "";

static const char* flagName(int i) {
  switch (i) {
    case ERR_BME280: return "BME280";
    case ERR_BH1750: return "BH1750";
    case ERR_PZEM:   return "PZEM";
    case ERR_RFID:   return "RFID";
    case ERR_WIFI:   return "WIFI";
    case ERR_MQTT:   return "MQTT";
    default:         return "?";
  }
}

void errors_init() {
  for (int i = 0; i < ERR_COUNT; i++) flags[i] = false;
  strcpy(summaryBuf, "OK");
}

static void rebuildSummary() {
  bool any = false;
  summaryBuf[0] = '\0';
  for (int i = 0; i < ERR_COUNT; i++) {
    if (flags[i]) {
      if (any) strcat(summaryBuf, ",");
      strcat(summaryBuf, flagName(i));
      any = true;
    }
  }
  if (!any) strcpy(summaryBuf, "OK");
}

void errors_setFlag(ErrorFlag flag, bool active) {
  if (flags[flag] == active) return; // no change, skip rebuild + publish
  flags[flag] = active;
  rebuildSummary();
  errors_publishIfChanged();
}

bool errors_anyActive() {
  for (int i = 0; i < ERR_COUNT; i++) if (flags[i]) return true;
  return false;
}

const char* errors_getSummary() { return summaryBuf; }

void errors_publishIfChanged() {
  if (strcmp(summaryBuf, lastPublished) == 0) return;
  strncpy(lastPublished, summaryBuf, sizeof(lastPublished) - 1);
  mqtt_publish(TOPIC_ERROR, summaryBuf, true);
  Serial.printf("[ERRORS] Fault summary: %s\n", summaryBuf);
}
