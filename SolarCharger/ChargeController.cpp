#include "ChargeController.h"

ChargeController::ChargeController(float maximum_voltage)
    : maximum_voltage_(maximum_voltage), mppt_() {}

float ChargeController::update(float battery_voltage, float pv_voltage, float pv_current) {
    if (battery_voltage >= maximum_voltage_) {
        return handleBatteryDisconnectedOrFull();
    } else {
        return handleNominalCharge(pv_voltage, pv_current);
    }
}

float ChargeController::handleBatteryDisconnectedOrFull() {
    return 0.0;
}

float ChargeController::handleNominalCharge(float pv_voltage, float pv_current) {
    return mppt_.update(pv_voltage, pv_current);
}
