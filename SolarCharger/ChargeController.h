#ifndef CHARGECONTROLLER_H
#define CHARGECONTROLLER_H

#include "MPPT.h"

class ChargeController {
public:
    ChargeController(float maximum_voltage);

    float update(float battery_voltage, float pv_voltage, float pv_current);

private:
    const float maximum_voltage_;
    MPPT mppt_;

    float handleBatteryDisconnectedOrFull();
    float handleNominalCharge(float pv_voltage, float pv_current);
};

#endif
