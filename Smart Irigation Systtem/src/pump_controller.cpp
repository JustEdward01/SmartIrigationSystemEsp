#include "pump_controller.h"
#include "config.h"
PumpController::PumpController() {}
void PumpController::init() { pinMode(PUMP_PIN, OUTPUT); pumpOff(); }
void PumpController::pumpOn() { digitalWrite(PUMP_PIN, HIGH); }
void PumpController::pumpOff() { digitalWrite(PUMP_PIN, LOW); }
bool PumpController::getStatus() { return digitalRead(PUMP_PIN) == HIGH; }
