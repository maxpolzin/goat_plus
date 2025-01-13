#include "ChargeController.h"

ChargeController::ChargeController()
    : currentSetpoint_mA(0)
{
}

int ChargeController::update(int pvVoltage_mV, int pvCurrent_mA, bool isStable, bool isBatteryConnected)
{
    static const unsigned long nSeconds = 60;
    static const int stepIncrement_mA = 3;
    static unsigned long lastProbeTime = 0;
    static bool isProbing = true;
    static int stableSetpoint_mA = 0;

    unsigned long now = millis();

    if (!isBatteryConnected) {
        currentSetpoint_mA = 0;
        stableSetpoint_mA = 0;
        isProbing = true;
        lastProbeTime = now;
        return currentSetpoint_mA;
    }

    if (!isProbing) {
        if (!isStable) {
            currentSetpoint_mA -= stepIncrement_mA;
            if (currentSetpoint_mA < 0) {
                currentSetpoint_mA = 0;
            }
        } else {
            stableSetpoint_mA = currentSetpoint_mA;
        }
        if ((now - lastProbeTime) >= (nSeconds * 1000UL)) {
            isProbing = true;
        }
    } else {
        if (isStable) {
            stableSetpoint_mA = currentSetpoint_mA;
            currentSetpoint_mA += stepIncrement_mA;
        } else {
            currentSetpoint_mA = stableSetpoint_mA;
            isProbing = false;
            lastProbeTime = now;
        }
    }

    return currentSetpoint_mA;
}
