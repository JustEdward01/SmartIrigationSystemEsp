#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "buffer_manager.h"
#include "sensors.h"
#include "pump_controller.h"
#include "security.h"
#include "debug_server.h"
#include <HTTPClient.h>
#include <esp_task_wdt.h>

WiFiManagerESP wifiManager;
BufferManager bufferManager;
Sensors sensors;
PumpController pumpController;
WebServer server(80);
DebugServer debugServer(server);

unsigned long lastSend = 0;
const unsigned long SEND_INTERVAL = 10000; // 🔧 test rapid: 10s

unsigned long lastRetry = 0;
unsigned long retryWait = 10000;

bool sendToServer(const String& payload) {
    if (wifiManager.status() != WIFI_OK) {
        Serial.println("⚠️ WiFi nu e conectat – date salvate local.");
        return false;
    }
    HTTPClient http;
    http.begin(BACKEND_URL);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(payload);
    String resp = http.getString();
    http.end();

    Serial.print("📡 POST server status: "); Serial.println(code);
    Serial.print("📡 Server response: "); Serial.println(resp);

    return code == 200 && Security::validateServerResponse(resp);
}

void runMainLogic() {
    Serial.println("🧪 Citire senzori...");
    SensorData data = sensors.readAll();

    Serial.print("🌱 Umiditate sol: "); Serial.println(data.soil_moisture);
    Serial.print("🌡️ Temperatură: "); Serial.println(data.temperature);
    Serial.print("💧 Umiditate aer: "); Serial.println(data.air_humidity);
    Serial.print("☀️ Lumină: "); Serial.println(data.light);

    String payload = sensors.toJSON(data);
    Serial.println("📦 Payload JSON: " + payload);

    if (wifiManager.status() == WIFI_OK) {
        if (sendToServer(payload)) {
            Serial.println("✅ Trimitere reușită.");
            bufferManager.flushToServer(sendToServer);
            retryWait = 10000;
        } else {
            Serial.println("❌ Trimitere eșuată. Salvăm local...");
            bufferManager.save(payload);
            lastRetry = millis();
            retryWait *= 2;
            if (retryWait > 600000) retryWait = 600000;
        }
    } else {
        Serial.println("🚫 Nu avem WiFi – salvăm local.");
        bufferManager.save(payload);
    }
}

void tryRetryFlush() {
    if (wifiManager.status() == WIFI_OK && millis() - lastRetry > retryWait) {
        Serial.println("♻️ Retry flush din buffer...");
        bufferManager.flushToServer(sendToServer);
        lastRetry = millis();
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("=== 🔌 ESP32 BOOT ===");

    wifiManager.init();
    Serial.println("✅ WiFiManager OK");

    WiFiStatus status = wifiManager.status();
    Serial.print("🌐 WiFi Status: ");
    if (status == WIFI_OK) Serial.println("Conectat");
    else if (status == WIFI_CUSTOM_AP) Serial.println("AP Mode (config)");
    else Serial.println("Eșuat");

    bufferManager.init();
    Serial.println("✅ BufferManager OK");

    sensors.init();
    Serial.println("✅ Sensors OK");

    pumpController.init();
    Serial.println("✅ Pompa OK");

    debugServer.setupEndpoints();
    Serial.println("✅ Debug server OK");

    esp_task_wdt_init(10, true);
    esp_task_wdt_add(NULL);

    Serial.println("🏁 Setup complet.");

    // 🔁 Forțează o rulare imediată
    Serial.println("🔁 Test rulare imediată runMainLogic()");
    runMainLogic();
}

void loop() {
    esp_task_wdt_reset();

    if (wifiManager.isAPMode()) {
        wifiManager.handleServer();
        delay(10);
        return;
    }

    unsigned long now = millis();
    if (now - lastSend > SEND_INTERVAL) {
        lastSend = now;
        Serial.println("⏱️ Execut logica principală...");
        runMainLogic();
    }

    tryRetryFlush();
    delay(10);
}
