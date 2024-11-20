#include "ChargeController.h"

ChargeController::ChargeController(float trickle_voltage, float nominal_voltage, float max_current, float min_current)
    : trickle_voltage_(trickle_voltage), nominal_voltage_(nominal_voltage), max_current_(max_current), min_current_(min_current), charging_mode_(MIN_CURRENT), mppt_() {}

float ChargeController::update(float battery_voltage, float pv_voltage, float pv_current) {
    charging_mode_ = determineChargingMode(battery_voltage);

    switch (charging_mode_) {
        case MIN_CURRENT:
            return handleMinCurrent();
        case TRICKLE:
            return handleTrickleCharge(pv_voltage, pv_current);
        case CONSTANT_CURRENT:
            return handleConstantCurrent(pv_voltage, pv_current);
        case CONSTANT_VOLTAGE:
            return handleConstantVoltage(battery_voltage, mppt_.update(pv_voltage, pv_current));
    }
    return 0.0; // Fallback
}

ChargeController::ChargingMode ChargeController::determineChargingMode(float battery_voltage) {
    if (battery_voltage < 9.0) {
        return MIN_CURRENT;
    } else if (battery_voltage < trickle_voltage_) {
        return TRICKLE;
    } else if (battery_voltage < nominal_voltage_) {
        return CONSTANT_CURRENT;
    } else {
        return CONSTANT_VOLTAGE;
    }
}

float ChargeController::handleMinCurrent() {
    return min_current_; // Minimum safe current
}

float ChargeController::handleTrickleCharge(float pv_voltage, float pv_current) {
    float mppt_current = mppt_.update(pv_voltage, pv_current);
    return fmin(mppt_current, 160.0); // Use MPPT but cap at 160mA
}

float ChargeController::handleConstantCurrent(float pv_voltage, float pv_current) {
    return mppt_.update(pv_voltage, pv_current); // Solely use MPPT for max power
}

float ChargeController::handleConstantVoltage(float battery_voltage, float mppt_current) {
    float voltage_error = nominal_voltage_ - battery_voltage;
    if (voltage_error <= 0) {
        return min_current_; // Gradually reduce to min current when fully charged
    }
    return fmax(min_current_, mppt_current * (voltage_error / nominal_voltage_)); // Gradual reduction
}
