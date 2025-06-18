#include "debug_server.h"
#include <FS.h>
#include <SPIFFS.h>
DebugServer::DebugServer(WebServer& server) {
    server.on("/status", [&]() {
        String json = "{\"status\":\"ok\"}";
        server.send(200, "application/json", json);
    });
    server.on("/last", [&]() {
        File file = SPIFFS.open("/buffer.json", FILE_READ);
        String last = "";
        while (file && file.available()) last = file.readStringUntil('\n');
        file.close();
        server.send(200, "application/json", last);
    });
    server.on("/log", [&]() {
        File file = SPIFFS.open("/buffer.json", FILE_READ);
        String log = "";
        while (file && file.available()) log += file.readStringUntil('\n');
        file.close();
        server.send(200, "text/plain", log);
    });
    server.on("/reset", HTTP_POST, [&]() {
        ESP.restart();
    });
}
void DebugServer::setupEndpoints() {}
