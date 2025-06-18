#pragma once
#include <Arduino.h>
struct SensorData {
    float soil_moisture;
    float temperature;
    float air_humidity;
    float light;
};
class Sensors {
public:
    Sensors();
    void init();
    float readSoil();
    float readLight();
    float readTemp();
    float readAirHumidity();
    SensorData readAll();
    String toJSON(const SensorData&);
};
