#include "buffer_manager.h"
#include <FS.h>
#include <SPIFFS.h>
#include "config.h"

BufferManager::BufferManager() {}
void BufferManager::init() { SPIFFS.begin(true); }
int BufferManager::countLines() {
    File file = SPIFFS.open("/buffer.json", FILE_READ);
    int lines = 0;
    while (file && file.available()) { file.readStringUntil('\n'); lines++; }
    file.close();
    return lines;
}
void BufferManager::enforceLimit() {
    int lines = countLines();
    if (lines <= MAX_BUFFER_LINES) return;
    File file = SPIFFS.open("/buffer.json", FILE_READ);
    String* arr = new String[MAX_BUFFER_LINES];
    int idx = 0;
    while (file && file.available()) {
        String ln = file.readStringUntil('\n');
        if (lines - idx <= MAX_BUFFER_LINES) arr[idx++] = ln;
    }
    file.close();
    SPIFFS.remove("/buffer.json");
    File out = SPIFFS.open("/buffer.json", FILE_WRITE);
    for (int i = 0; i < idx; i++) out.println(arr[i]);
    out.close();
    delete[] arr;
}
void BufferManager::save(const String& payload) {
    enforceLimit();
    File file = SPIFFS.open("/buffer.json", FILE_APPEND);
    if (file) { file.println(payload); file.close(); }
}
void BufferManager::flushToServer(bool(*sendFn)(const String&)) {
    File file = SPIFFS.open("/buffer.json", FILE_READ);
    if (!file || file.size() == 0) return;
    while (file.available()) {
        String line = file.readStringUntil('\n');
        if (!sendFn(line)) { file.close(); return; }
    }
    file.close();
    SPIFFS.remove("/buffer.json");
}
bool BufferManager::hasSpace() { return countLines() < MAX_BUFFER_LINES; }
void BufferManager::backupBuffer() {
    File file = SPIFFS.open("/buffer.json", FILE_READ);
    File backup = SPIFFS.open("/buffer.bak", FILE_WRITE);
    while (file && file.available()) backup.println(file.readStringUntil('\n'));
    file.close(); backup.close();
}
void BufferManager::restoreBuffer() {
    File file = SPIFFS.open("/buffer.bak", FILE_READ);
    File mainf = SPIFFS.open("/buffer.json", FILE_WRITE);
    while (file && file.available()) mainf.println(file.readStringUntil('\n'));
    file.close(); mainf.close();
}
