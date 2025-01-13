#ifndef CHARGECONTROLLER_H
#define CHARGECONTROLLER_H

#include <Arduino.h>

class ChargeController {
public:
    ChargeController();
    int update(int pvVoltage_mV, int pvCurrent_mA, bool isStable, bool isBatteryConnected);

private:
    int currentSetpoint_mA;
};

#endif // CHARGECONTROLLER_H
