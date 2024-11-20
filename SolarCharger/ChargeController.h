#ifndef CHARGECONTROLLER_H
#define CHARGECONTROLLER_H

#include "MPPT.h"
#include "Potentiometer.h"

class ChargeController {
public:
    ChargeController(float trickle_voltage, float nominal_voltage, float max_current, float min_current);

    float update(float battery_voltage, float pv_voltage, float pv_current);

private:
    enum ChargingMode { MIN_CURRENT, TRICKLE, CONSTANT_CURRENT, CONSTANT_VOLTAGE };

    const float trickle_voltage_;
    const float nominal_voltage_;
    const float max_current_;
    const float min_current_;
    ChargingMode charging_mode_;

    MPPT mppt_;

    ChargingMode determineChargingMode(float battery_voltage);
    float handleMinCurrent();
    float handleTrickleCharge(float pv_voltage, float pv_current);
    float handleConstantCurrent(float pv_voltage, float pv_current);
    float handleConstantVoltage(float battery_voltage, float mppt_current);
};

#endif
