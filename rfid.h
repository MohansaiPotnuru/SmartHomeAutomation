#ifndef RFID_H
#define RFID_H

#include <Arduino.h>

enum RfidRole : uint8_t { ROLE_ADMIN = 0, ROLE_USER = 1, ROLE_GUEST = 2, ROLE_NONE = 255 };

void rfid_init();
void rfid_service();          // call every loop - polls reader, handles lockout

// Card management (persisted in NVS via Preferences)
bool rfid_addCard(const char* uid, RfidRole role, const char* label);
bool rfid_removeCard(const char* uid);
RfidRole rfid_lookup(const char* uid);
void rfid_factoryReset();     // wipes stored cards, reloads DEFAULT_CARDS from secrets.h

// Called when a valid "UNLOCK" command arrives over MQTT (bypasses card check).
void rfid_remoteUnlock();

#endif // RFID_H
