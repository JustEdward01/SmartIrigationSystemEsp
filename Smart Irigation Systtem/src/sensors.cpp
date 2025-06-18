#include "sensors.h"
#include <DHT.h>
#include <ArduinoJson.h>
#include "config.h"
DHT dht(TEMP_PIN, DHTTYPE);
Sensors::Sensors() {}
void Sensors::init() { dht.begin(); }
float Sensors::readSoil() {
    int raw = analogRead(SOIL_MOISTURE_AO_PIN);
    return 100.0 * (4095 - raw) / 4095.0;
}
float Sensors::readLight() {
    int raw = analogRead(LIGHT_AO_PIN);
    return map(raw, 0, 4095, 0, 60000);
}
float Sensors::readTemp() { return dht.readTemperature(); }
float Sensors::readAirHumidity() { return dht.readHumidity(); }
SensorData Sensors::readAll() {
    SensorData data;
    data.soil_moisture = readSoil();
    data.temperature = readTemp();
    data.air_humidity = readAirHumidity();
    data.light = readLight();
    return data;
}
String Sensors::toJSON(const SensorData& data) {
    StaticJsonDocument<256> doc;
    doc["plant_type"] = "rosie";
    doc["soil_moisture"] = data.soil_moisture;
    doc["temperature"] = data.temperature;
    doc["air_humidity"] = data.air_humidity;
    doc["light"] = data.light;
    String payload;
    serializeJson(doc, payload);
    return payload;
}
