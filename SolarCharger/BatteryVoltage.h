#ifndef BATTERYVOLTAGE_H
#define BATTERYVOLTAGE_H

#include <Arduino.h>

class BatteryVoltage {
public:
    BatteryVoltage(int pin);
    void begin();
    float readVoltage();
    bool isBatteryConnected();

private:
    int pin_;

    static const float R1;
    static const float R2;
    static const float ADC_REF_VOLTAGE;
    static const float ADC_RESOLUTION;
    static const float OPEN_CIRCUIT_VOLTAGE;
};

#endif // BATTERYVOLTAGE_H
