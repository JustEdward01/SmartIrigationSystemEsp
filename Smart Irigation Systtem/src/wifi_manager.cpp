#include "wifi_manager.h"
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"

WiFiManagerESP::WiFiManagerESP() : server(80), apMode(false) {}
bool WiFiManagerESP::validateInput(const String& s, const String& p) {
    if (s.length() < 2 || s.length() > 32) return false;
    for (char c : s) if (!isalnum(c) && c != '-' && c != '_' && c != '.') return false;
    if (p.length() < 4 || p.length() > 63) return false;
    return true;
}
bool WiFiManagerESP::loadConfig() {
    File f = SPIFFS.open("/wifi.json", FILE_READ);
    if (!f || f.size() == 0) return false;
    StaticJsonDocument<128> doc;
    if (deserializeJson(doc, f)) return false;
    ssid = doc["ssid"].as<String>();
    password = doc["password"].as<String>();
    f.close();
    return true;
}
void WiFiManagerESP::saveConfig(const String& s, const String& p) {
    StaticJsonDocument<128> doc;
    doc["ssid"] = s;
    doc["password"] = p;
    File f = SPIFFS.open("/wifi.json", FILE_WRITE);
    serializeJson(doc, f);
    f.close();
}
void WiFiManagerESP::handleRoot() {
    String html = R"rawliteral(
        <h2>Configurare WiFi SmartPlant</h2>
        <form method='POST' action='/set'>
            SSID:<br><input name='ssid'><br>
            Parola:<br><input name='pass' type='password'><br><br>
            <input type='submit' value='Salvează'>
        </form>
    )rawliteral";
    server.send(200, "text/html", html);
}
void WiFiManagerESP::handleSet() {
    String s = server.arg("ssid");
    String p = server.arg("pass");
    if (!validateInput(s, p)) {
        server.send(400, "text/html", "Date invalide!");
        return;
    }
    saveConfig(s, p);
    server.send(200, "text/html", "Salvat. Se restartează...");
    delay(2000);
    ESP.restart();
}
void WiFiManagerESP::handleResetWiFi() {
    SPIFFS.remove("/wifi.json");
    server.send(200, "text/plain", "WiFi config șters. Se restartează...");
    delay(1000);
    ESP.restart();
}
void WiFiManagerESP::handleStatus() {
    String json = "{\"mode\":\"AP\",\"ssid\":\"SmartPlant_Setup\"}";
    server.send(200, "application/json", json);
}
void WiFiManagerESP::startAP() {
    WiFi.softAP("SmartPlant_Setup");
    apMode = true;
    server.on("/", [this]() { handleRoot(); });
    server.on("/set", HTTP_POST, [this]() { handleSet(); });
    server.on("/reset-wifi", HTTP_POST, [this]() { handleResetWiFi(); });
    server.on("/status", [this]() { handleStatus(); });
    server.begin();
}
void WiFiManagerESP::init() {
    if (!SPIFFS.begin(true)) return;
    if (!loadConfig()) {
        startAP();
        return;
    }
    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 10000) { delay(250); }
    if (WiFi.status() == WL_CONNECTED) {
        logStatus();
        apMode = false;
    } else {
        startAP();
    }
}
WiFiStatus WiFiManagerESP::status() {
    if (apMode) return WIFI_CUSTOM_AP;
    if (WiFi.status() == WL_CONNECTED) return WIFI_OK;
    return WIFI_FAIL;
}
void WiFiManagerESP::reset() {
    SPIFFS.remove("/wifi.json");
    ESP.restart();
}
void WiFiManagerESP::logStatus() {
    if (WiFi.status() != WL_CONNECTED) return;
    StaticJsonDocument<128> doc;
    doc["ssid"] = ssid;
    doc["ip_address"] = WiFi.localIP().toString();
    doc["status"] = "online";
    String payload;
    serializeJson(doc, payload);
    HTTPClient http;
    http.begin(WIFI_LOG_URL);
    http.addHeader("Content-Type", "application/json");
    http.POST(payload);
    http.end();
}
void WiFiManagerESP::handleServer() { if (apMode) server.handleClient(); }
bool WiFiManagerESP::isAPMode() const { return apMode; }
String WiFiManagerESP::getSSID() { return ssid; }
