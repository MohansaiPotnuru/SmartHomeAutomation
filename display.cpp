#include "display.h"
#include "config.h"

#if FEATURE_OLED

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "pins.h"
#include "sensor.h"
#include "power.h"
#include "relay.h"
#include "mqtt.h"
#include "automation.h"
#include "alarm.h"

#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_ADDR    0x3C

static Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
static unsigned long lastUpdate = 0;
static uint8_t page = 0;
static bool oledOk = false;

void display_init() {
  // Wire.begin() is already called once in sensor_init(); calling it again
  // with the same pins is harmless and keeps this module self-contained.
  Wire.begin(I2C_SDA, I2C_SCL);
  oledOk = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  if (!oledOk) {
    Serial.println("[DISPLAY] WARNING: SSD1306 OLED not found at 0x3C.");
    return;
  }
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.println("Smart Home Booting...");
  oled.display();
}

static void drawPageStatus() {
  oled.printf("WiFi : %s\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Offline");
  oled.printf("MQTT : %s\n", mqtt_isConnected() ? "Connected" : "Offline");
  oled.printf("Armed: %s\n", systemArmed ? "YES" : "NO");
  oled.printf("Alarm: %s\n", alarm_anyActive() ? "ACTIVE!" : "Clear");
  oled.printf("Door : %s\n", relay_isDoorUnlocked() ? "UNLOCKED" : "Locked");
}

static void drawPageEnvironment() {
  oled.println("-- Environment --");
  if (!isnan(sensorData.temperature)) oled.printf("Temp: %.1f C\n", sensorData.temperature);
  else oled.println("Temp: N/A");
  if (!isnan(sensorData.humidity)) oled.printf("Hum : %.1f %%\n", sensorData.humidity);
  if (!isnan(sensorData.pressure)) oled.printf("Pres: %.0f hPa\n", sensorData.pressure);
  if (!isnan(sensorData.lux)) oled.printf("Lux : %.0f\n", sensorData.lux);
}

static void drawPageSafety() {
  oled.println("-- Safety --");
  oled.printf("Smoke: %d\n", sensorData.smokeRaw);
  oled.printf("Gas  : %d\n", sensorData.gasRaw);
  oled.printf("Water: %d\n", sensorData.waterRaw);
  oled.printf("Motion: %s\n", sensorData.motion ? "YES" : "no");
}

static void drawPagePower() {
  oled.println("-- Power (PZEM) --");
  if (powerData.ok) {
    oled.printf("V: %.1fV  I: %.2fA\n", powerData.voltage, powerData.current);
    oled.printf("P: %.1fW\n", powerData.power);
    oled.printf("E: %.3f kWh\n", powerData.energyKwh);
  } else {
    oled.println("PZEM not detected");
  }
}

void display_service() {
  if (!oledOk) return;

  unsigned long now = millis();
  if (now - lastUpdate < DISPLAY_INTERVAL_MS) return;
  lastUpdate = now;

  oled.clearDisplay();
  oled.setCursor(0, 0);

  switch (page) {
    case 0: drawPageStatus(); break;
    case 1: drawPageEnvironment(); break;
    case 2: drawPageSafety(); break;
    case 3: drawPagePower(); break;
  }

  oled.display();
  page = (page + 1) % 4;
}

#else

void display_init() {}
void display_service() {}

#endif // FEATURE_OLED
