// secrets.example.h - template for secrets.h
//
//   1. Copy this file to secrets.h  (secrets.h is git-ignored)
//   2. Fill in your own values
//
// Never commit real credentials.
#ifndef SECRETS_H
#define SECRETS_H

#include <Arduino.h>
#include "rfid.h"

// ---------- Wi-Fi ----------
static const char* const WIFI_SSID     = "your-wifi-ssid";
static const char* const WIFI_PASSWORD = "your-wifi-password";

// ---------- MQTT broker ----------
static const char* const MQTT_SERVER = "192.168.1.10";   // broker IP or hostname
static const uint16_t    MQTT_PORT   = 1883;
// Leave as nullptr for an anonymous broker; set both for authenticated brokers.
static const char* const MQTT_USER   = nullptr;
static const char* const MQTT_PASS   = nullptr;

// ---------- Local web dashboard (HTTP Basic Auth) ----------
static const char* const WEB_ADMIN_USER = "admin";
static const char* const WEB_ADMIN_PASS = "change-me";

// ---------- RFID cards loaded into NVS on first boot ----------
// UID format matches the serial log: upper-case hex bytes separated by spaces.
// role: ROLE_ADMIN, ROLE_USER or ROLE_GUEST
struct RfidCard {
  const char* uid;
  uint8_t     role;
  const char* label;
};

static const RfidCard DEFAULT_CARDS[] = {
  { "DE AD BE EF", ROLE_ADMIN, "Owner"   },
  { "12 34 56 78", ROLE_GUEST, "Visitor" },
};
static const size_t DEFAULT_CARD_COUNT = sizeof(DEFAULT_CARDS) / sizeof(DEFAULT_CARDS[0]);

#endif // SECRETS_H
