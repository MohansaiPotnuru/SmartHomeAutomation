#include "wifi_mgr.h"
#include <WiFi.h>
#include "config.h"
#include "secrets.h"
#include "errors.h"

static bool connecting = false;
static unsigned long lastAttempt = 0;
static unsigned long connectStarted = 0;

void wifi_init() {
  WiFi.mode(WIFI_STA);
  // Kick off the first attempt immediately.
  lastAttempt = 0;
}

bool wifi_isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void wifi_service() {
  unsigned long now = millis();

  if (wifi_isConnected()) {
    connecting = false;
    errors_setFlag(ERR_WIFI, false);
    return;
  }
  errors_setFlag(ERR_WIFI, true);

  if (!connecting) {
    if (now - lastAttempt < WIFI_CHECK_INTERVAL_MS) return; // throttle retries

    lastAttempt = now;
    connectStarted = now;
    connecting = true;

    Serial.printf("[WIFI] Connecting to %s...\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    return;
  }

  // Currently attempting - poll status without blocking the loop.
  if (wifi_isConnected()) {
    Serial.print("[WIFI] Connected! IP: ");
    Serial.println(WiFi.localIP());
    connecting = false;
    return;
  }

  if (now - connectStarted >= WIFI_CONNECT_TIMEOUT_MS) {
    Serial.println("[WIFI] Connect attempt timed out, will retry later.");
    connecting = false; // next throttle window will retry
  }
}
