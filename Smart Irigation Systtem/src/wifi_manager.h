#pragma once
#include <Arduino.h>
#include <WebServer.h>

enum WiFiStatus { WIFI_OK, WIFI_CUSTOM_AP, WIFI_FAIL };

class WiFiManagerESP {
public:
    WiFiManagerESP();
    void init();
    WiFiStatus status();
    void reset();
    void handleServer();
    void logStatus();
    bool isAPMode() const;
    String getSSID();
private:
    String ssid, password;
    WebServer server;
    bool apMode;
    void startAP();
    void handleRoot();
    void handleSet();
    void handleResetWiFi();
    void handleStatus();
    bool loadConfig();
    void saveConfig(const String&, const String&);
    bool validateInput(const String&, const String&);
};
