#ifndef SWITCHES_H
#define SWITCHES_H

#include <Arduino.h>

// Momentary push-buttons wired INPUT_PULLUP (pressed = LOW).
// Each press toggles its relay directly, independent of WiFi/MQTT -
// this is the offline fallback the review asked for.
void switches_init();
void switches_service();  // call every loop; internally throttled to SWITCH_POLL_MS

#endif // SWITCHES_H
