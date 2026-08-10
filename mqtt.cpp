#include "mqtt.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "secrets.h"
#include "pins.h"
#include "relay.h"
#include "rfid.h"
#include "alarm.h"
#include "errors.h"
#include "automation.h"
#include <esp_system.h>

static WiFiClient espClient;
static PubSubClient client(espClient);
static unsigned long lastAttempt = 0;

static void handleMessage(char* topic, byte* payload, unsigned int length) {
  char message[32];
  unsigned int len = length < sizeof(message) - 1 ? length : sizeof(message) - 1;
  for (unsigned int i = 0; i < len; i++) message[i] = (char)payload[i];
  message[len] = '\0';
  for (unsigned int i = 0; i < len; i++) message[i] = toupper(message[i]);

  Serial.printf("[MQTT] %s = %s\n", topic, message);
  bool isOn = (strcmp(message, "ON") == 0);

  if (strcmp(topic, TOPIC_LIGHT1) == 0) {
    relay_setLight1(isOn, false);
  } else if (strcmp(topic, TOPIC_LIGHT2) == 0) {
    relay_setLight2(isOn, false);
  } else if (strcmp(topic, TOPIC_FAN1) == 0) {
    relay_setFan1(isOn, false);
  } else if (strcmp(topic, TOPIC_FAN2) == 0) {
    relay_setFan2(isOn, false);
  } else if (strcmp(topic, TOPIC_PUMP) == 0) {
    relay_setPump(isOn, false);
  } else if (strcmp(topic, TOPIC_DOOR) == 0) {
    if (strcmp(message, "UNLOCK") == 0) rfid_remoteUnlock();
    else if (strcmp(message, "LOCK") == 0) relay_setDoorLock(false);
  } else if (strcmp(topic, TOPIC_ARM) == 0) {
    security_setArmed(isOn || strcmp(message, "TRUE") == 0);
  }
}

void mqtt_init() {
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(handleMessage);
  client.setSocketTimeout(2); // seconds - keep well under the hardware WDT timeout
  client.setBufferSize(512);  // headroom for JSON-ish payloads from the web dashboard later
}

bool mqtt_isConnected() { return client.connected(); }

void mqtt_service() {
  if (WiFi.status() != WL_CONNECTED) return;

  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastAttempt < MQTT_RETRY_INTERVAL_MS) return; // throttle - avoid hammering broker/WDT
    lastAttempt = now;

    Serial.print("[MQTT] Connecting...");
    String clientId = "ESP32_SmartHome_" + String((uint32_t)esp_random(), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS, TOPIC_STATUS, 1, true, "offline")) {
      Serial.println(" connected.");
      errors_setFlag(ERR_MQTT, false);
      client.publish(TOPIC_STATUS, "online", true);
      client.publish(TOPIC_ERROR, errors_getSummary(), true);

      client.subscribe(TOPIC_LIGHT1);
      client.subscribe(TOPIC_LIGHT2);
      client.subscribe(TOPIC_FAN1);
      client.subscribe(TOPIC_FAN2);
      client.subscribe(TOPIC_PUMP);
      client.subscribe(TOPIC_DOOR);
      client.subscribe(TOPIC_ARM);
    } else {
      Serial.printf(" failed, rc=%d\n", client.state());
      errors_setFlag(ERR_MQTT, true);
    }
    return;
  }

  client.loop();
}

void mqtt_publish(const char* topic, const char* payload, bool retain) {
  if (!client.connected()) return;
  client.publish(topic, payload, retain);
}

void mqtt_publish(const char* topic, float value, uint8_t decimals, bool retain) {
  if (!client.connected()) return;
  char buf[16];
  dtostrf(value, 0, decimals, buf);
  client.publish(topic, buf, retain);
}

void mqtt_publish(const char* topic, int value, bool retain) {
  if (!client.connected()) return;
  char buf[12];
  itoa(value, buf, 10);
  client.publish(topic, buf, retain);
}
