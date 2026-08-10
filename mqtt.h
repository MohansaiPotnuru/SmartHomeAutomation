#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>

void mqtt_init();
void mqtt_service();          // call every loop - throttled reconnect + client.loop()
bool mqtt_isConnected();

// Safe publish: silently no-ops if not connected, so callers never need
// to guard every publish() call with an "if connected" check themselves.
void mqtt_publish(const char* topic, const char* payload, bool retain);
void mqtt_publish(const char* topic, float value, uint8_t decimals, bool retain);
void mqtt_publish(const char* topic, int value, bool retain);

#endif // MQTT_H
