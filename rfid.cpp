#include "rfid.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Preferences.h>
#include "pins.h"
#include "config.h"
#include "secrets.h"
#include "relay.h"
#include "alarm.h"
#include "mqtt.h"
#include "errors.h"

static MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
static Preferences prefs;

static int failCount = 0;
static unsigned long lockoutUntil = 0;

// ---- Card storage helpers ----
// Stored under namespace "rfidcards" as key "card0".."card9" -> "UID|role|label"
static const char* NVS_NS = "rfidcards";

static void loadDefaultsIfEmpty() {
  prefs.begin(NVS_NS, false);
  bool provisioned = prefs.getBool("provisioned", false);
  if (!provisioned) {
    Serial.println("[RFID] No stored cards found - loading defaults from secrets.h");
    for (size_t i = 0; i < DEFAULT_CARD_COUNT; i++) {
      char key[16];
      snprintf(key, sizeof(key), "card%d", (int)i);
      String value = String(DEFAULT_CARDS[i].uid) + "|" + String(DEFAULT_CARDS[i].role) + "|" + String(DEFAULT_CARDS[i].label);
      prefs.putString(key, value);
    }
    prefs.putInt("count", (int)DEFAULT_CARD_COUNT);
    prefs.putBool("provisioned", true);
  }
  prefs.end();
}

RfidRole rfid_lookup(const char* uid) {
  prefs.begin(NVS_NS, true);
  int count = prefs.getInt("count", 0);
  RfidRole found = ROLE_NONE;
  for (int i = 0; i < count && i < RFID_MAX_CARDS; i++) {
    char key[16];
    snprintf(key, sizeof(key), "card%d", i);
    String entry = prefs.getString(key, "");
    if (entry.length() == 0) continue;
    int sep1 = entry.indexOf('|');
    if (sep1 < 0) continue;
    String storedUid = entry.substring(0, sep1);
    if (storedUid == uid) {
      int sep2 = entry.indexOf('|', sep1 + 1);
      String roleStr = (sep2 > 0) ? entry.substring(sep1 + 1, sep2) : entry.substring(sep1 + 1);
      found = (RfidRole)roleStr.toInt();
      break;
    }
  }
  prefs.end();
  return found;
}

bool rfid_addCard(const char* uid, RfidRole role, const char* label) {
  prefs.begin(NVS_NS, false);
  int count = prefs.getInt("count", 0);
  if (count >= RFID_MAX_CARDS) { prefs.end(); return false; }
  char key[16];
  snprintf(key, sizeof(key), "card%d", count);
  String value = String(uid) + "|" + String((int)role) + "|" + String(label);
  prefs.putString(key, value);
  prefs.putInt("count", count + 1);
  prefs.end();
  Serial.printf("[RFID] Added card %s as role %d (%s)\n", uid, role, label);
  return true;
}

bool rfid_removeCard(const char* uid) {
  prefs.begin(NVS_NS, false);
  int count = prefs.getInt("count", 0);
  bool removed = false;
  // Simple compact-on-remove: rewrite the list without the matching UID.
  String kept[RFID_MAX_CARDS];
  int keptCount = 0;
  for (int i = 0; i < count && i < RFID_MAX_CARDS; i++) {
    char key[16];
    snprintf(key, sizeof(key), "card%d", i);
    String entry = prefs.getString(key, "");
    if (entry.startsWith(String(uid) + "|")) {
      removed = true;
      continue;
    }
    kept[keptCount++] = entry;
  }
  for (int i = 0; i < keptCount; i++) {
    char key[16];
    snprintf(key, sizeof(key), "card%d", i);
    prefs.putString(key, kept[i]);
  }
  prefs.putInt("count", keptCount);
  prefs.end();
  return removed;
}

void rfid_factoryReset() {
  prefs.begin(NVS_NS, false);
  prefs.clear();
  prefs.end();
  loadDefaultsIfEmpty();
}

void rfid_init() {
  SPI.begin();
  rfid.PCD_Init();

  // VersionReg reads 0x00 or 0xFF when no MFRC522 answers on the SPI bus.
  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  bool ok = (version != 0x00 && version != 0xFF);
  if (!ok) Serial.println("[RFID] WARNING: MFRC522 not detected (check SPI wiring).");
  else Serial.printf("[RFID] MFRC522 firmware version 0x%02X\n", version);
  errors_setFlag(ERR_RFID, !ok);

  loadDefaultsIfEmpty();
}

static void doUnlock(const char* who) {
  relay_setDoorLock(true);
  alarm_startBuzzer(TONE_SHORT_MS);
  mqtt_publish(TOPIC_LOG, (String("Door unlocked by ") + who).c_str(), false);
}

void rfid_remoteUnlock() {
  doUnlock("MQTT command");
}

void rfid_service() {
  unsigned long now = millis();

  if (lockoutUntil != 0) {
    if (now < lockoutUntil) return;
    lockoutUntil = 0;
    failCount = 0;
    Serial.println("[RFID] Lockout period ended.");
  }

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  char cardUID[32] = {0};
  int pos = 0;
  for (byte i = 0; i < rfid.uid.size && pos < (int)sizeof(cardUID) - 3; i++) {
    pos += snprintf(cardUID + pos, sizeof(cardUID) - pos, "%02X", rfid.uid.uidByte[i]);
    if (i < rfid.uid.size - 1) cardUID[pos++] = ' ';
  }

  Serial.printf("[RFID] Scanned UID: %s\n", cardUID);
  mqtt_publish(TOPIC_RFID, cardUID, false);

  RfidRole role = rfid_lookup(cardUID);
  if (role != ROLE_NONE) {
    const char* roleName = (role == ROLE_ADMIN) ? "Admin" : (role == ROLE_USER) ? "User" : "Guest";
    Serial.printf("[RFID] Access Granted (%s).\n", roleName);
    failCount = 0;
    doUnlock(roleName);
  } else {
    Serial.println("[RFID] Access Denied.");
    failCount++;
    alarm_startBuzzer(TONE_REJECT_MS);
    mqtt_publish(TOPIC_LOG, (String("Denied RFID scan: ") + cardUID).c_str(), false);

    if (failCount >= RFID_MAX_FAILS) {
      lockoutUntil = now + RFID_LOCKOUT_MS;
      Serial.println("[RFID] Too many failed attempts - reader locked out temporarily.");
      mqtt_publish(TOPIC_ALARM, "SECURITY: Repeated invalid RFID scans - reader locked out.", false);
    }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
