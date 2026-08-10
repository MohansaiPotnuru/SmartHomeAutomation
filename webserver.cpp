#include "webserver.h"
#include "config.h"

#if FEATURE_WEBSERVER

#include <WebServer.h>
#include <WiFi.h>
#include "secrets.h"
#include "sensor.h"
#include "power.h"
#include "relay.h"
#include "automation.h"
#include "alarm.h"
#include "errors.h"
#include "mqtt.h"

static WebServer server(80);

static bool checkAuth() {
  if (!server.authenticate(WEB_ADMIN_USER, WEB_ADMIN_PASS)) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

static void handleRoot() {
  if (!checkAuth()) return;

  String html;
  html.reserve(3000);
  html += "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>Smart Home Dashboard</title><style>";
  html += "body{font-family:sans-serif;background:#111;color:#eee;margin:0;padding:16px}";
  html += "h2{border-bottom:1px solid #444;padding-bottom:6px}";
  html += ".card{background:#1e1e1e;border-radius:10px;padding:12px 16px;margin-bottom:12px}";
  html += ".row{display:flex;justify-content:space-between;align-items:center;padding:6px 0}";
  html += "button{background:#2d7dfa;color:#fff;border:none;padding:8px 14px;border-radius:6px;font-size:14px}";
  html += "button.off{background:#444}";
  html += ".alarm{color:#ff5555;font-weight:bold}";
  html += "</style></head><body>";
  html += "<h2>Smart Home Automation</h2>";

  html += "<div class='card'><b>System</b>";
  html += "<div class='row'><span>WiFi</span><span>" + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Offline") + "</span></div>";
  html += "<div class='row'><span>MQTT</span><span>" + String(mqtt_isConnected() ? "Connected" : "Offline") + "</span></div>";
  html += "<div class='row'><span>Armed</span><span>" + String(systemArmed ? "YES" : "NO") + "</span></div>";
  html += "<div class='row'><span>Faults</span><span>" + String(errors_getSummary()) + "</span></div>";
  if (alarm_anyActive()) html += "<div class='row alarm'>ALARM ACTIVE</div>";
  html += "</div>";

  html += "<div class='card'><b>Actuators</b>";
  html += "<div class='row'><span>Light 1</span><a href='/control?d=light1&s=" + String(relay_getLight1() ? "off" : "on") + "'><button class='" + String(relay_getLight1() ? "" : "off") + "'>" + String(relay_getLight1() ? "ON" : "OFF") + "</button></a></div>";
  html += "<div class='row'><span>Light 2</span><a href='/control?d=light2&s=" + String(relay_getLight2() ? "off" : "on") + "'><button class='" + String(relay_getLight2() ? "" : "off") + "'>" + String(relay_getLight2() ? "ON" : "OFF") + "</button></a></div>";
  html += "<div class='row'><span>Fan 1</span><a href='/control?d=fan1&s=" + String(relay_getFan1() ? "off" : "on") + "'><button class='" + String(relay_getFan1() ? "" : "off") + "'>" + String(relay_getFan1() ? "ON" : "OFF") + "</button></a></div>";
  html += "<div class='row'><span>Pump</span><a href='/control?d=pump&s=" + String(relay_getPump() ? "off" : "on") + "'><button class='" + String(relay_getPump() ? "" : "off") + "'>" + String(relay_getPump() ? "ON" : "OFF") + "</button></a></div>";
  html += "<div class='row'><span>Door</span><a href='/control?d=door&s=" + String(relay_isDoorUnlocked() ? "lock" : "unlock") + "'><button class='" + String(relay_isDoorUnlocked() ? "" : "off") + "'>" + String(relay_isDoorUnlocked() ? "UNLOCKED" : "LOCKED") + "</button></a></div>";
  html += "<div class='row'><span>Armed</span><a href='/control?d=arm&s=" + String(systemArmed ? "off" : "on") + "'><button class='" + String(systemArmed ? "" : "off") + "'>" + String(systemArmed ? "ARMED" : "DISARMED") + "</button></a></div>";
  html += "</div>";

  html += "<div class='card'><b>Environment</b>";
  if (!isnan(sensorData.temperature)) html += "<div class='row'><span>Temperature</span><span>" + String(sensorData.temperature, 1) + " C</span></div>";
  if (!isnan(sensorData.humidity)) html += "<div class='row'><span>Humidity</span><span>" + String(sensorData.humidity, 1) + " %</span></div>";
  if (!isnan(sensorData.pressure)) html += "<div class='row'><span>Pressure</span><span>" + String(sensorData.pressure, 0) + " hPa</span></div>";
  if (!isnan(sensorData.lux)) html += "<div class='row'><span>Light level</span><span>" + String(sensorData.lux, 0) + " lux</span></div>";
  html += "<div class='row'><span>Smoke (raw)</span><span>" + String(sensorData.smokeRaw) + "</span></div>";
  html += "<div class='row'><span>Gas (raw)</span><span>" + String(sensorData.gasRaw) + "</span></div>";
  html += "<div class='row'><span>Water (raw)</span><span>" + String(sensorData.waterRaw) + "</span></div>";
  html += "<div class='row'><span>Motion</span><span>" + String(sensorData.motion ? "Detected" : "Clear") + "</span></div>";
  html += "</div>";

  html += "<div class='card'><b>Power (PZEM-004T)</b>";
  if (powerData.ok) {
    html += "<div class='row'><span>Voltage</span><span>" + String(powerData.voltage, 1) + " V</span></div>";
    html += "<div class='row'><span>Current</span><span>" + String(powerData.current, 2) + " A</span></div>";
    html += "<div class='row'><span>Power</span><span>" + String(powerData.power, 1) + " W</span></div>";
    html += "<div class='row'><span>Energy</span><span>" + String(powerData.energyKwh, 3) + " kWh</span></div>";
  } else {
    html += "<div class='row'><span>Status</span><span>Not detected</span></div>";
  }
  html += "</div>";

  html += "<p style='color:#666;font-size:12px'>Auto-refreshes every 5s.</p>";
  html += "<script>setTimeout(()=>location.reload(),5000)</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

static void handleControl() {
  if (!checkAuth()) return;

  String device = server.arg("d");
  String state = server.arg("s");
  bool on = (state == "on" || state == "unlock");

  if (device == "light1") relay_setLight1(on, false);
  else if (device == "light2") relay_setLight2(on, false);
  else if (device == "fan1") relay_setFan1(on, false);
  else if (device == "pump") relay_setPump(on, false);
  else if (device == "door") relay_setDoorLock(state == "unlock");
  else if (device == "arm") security_setArmed(on);

  server.sendHeader("Location", "/");
  server.send(303);
}

static void handleApiStatus() {
  if (!checkAuth()) return;
  String json = "{";
  json += "\"wifi\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
  json += "\"mqtt\":" + String(mqtt_isConnected() ? "true" : "false") + ",";
  json += "\"armed\":" + String(systemArmed ? "true" : "false") + ",";
  json += "\"alarm\":" + String(alarm_anyActive() ? "true" : "false") + ",";
  json += "\"faults\":\"" + String(errors_getSummary()) + "\",";
  json += "\"temperature\":" + String(sensorData.temperature, 1) + ",";
  json += "\"humidity\":" + String(sensorData.humidity, 1) + ",";
  json += "\"lux\":" + String(sensorData.lux, 0) + ",";
  json += "\"voltage\":" + String(powerData.voltage, 1) + ",";
  json += "\"power\":" + String(powerData.power, 1);
  json += "}";
  server.send(200, "application/json", json);
}

void webserver_init() {
  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.on("/api/status", handleApiStatus);
  server.begin();
  Serial.println("[WEB] Dashboard started on port 80.");
}

void webserver_service() {
  server.handleClient();
}

#else

void webserver_init() {}
void webserver_service() {}

#endif // FEATURE_WEBSERVER
