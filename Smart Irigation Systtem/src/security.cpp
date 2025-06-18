#include "security.h"
#include <ArduinoJson.h>
namespace Security {
bool validateServerResponse(const String& payload) {
    StaticJsonDocument<128> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) return false;
    if (!doc.containsKey("status")) return false;
    return true;
}
}
