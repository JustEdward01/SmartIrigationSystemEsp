#pragma once
#include <Arduino.h>
class PumpController {
public:
    PumpController();
    void init();
    void pumpOn();
    void pumpOff();
    bool getStatus();
};
