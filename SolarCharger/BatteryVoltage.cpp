#include "BatteryVoltage.h"

const float BatteryVoltage::R1               = 220000.0f;
const float BatteryVoltage::R2               = 47000.0f;
const float BatteryVoltage::ADC_REF_VOLTAGE  = 3.3f;
const float BatteryVoltage::ADC_RESOLUTION   = 1023.0f;
const float BatteryVoltage::OPEN_CIRCUIT_VOLTAGE = 12.4f;

BatteryVoltage::BatteryVoltage(int pin)
    : pin_(pin)
{
}

void BatteryVoltage::begin() {
    pinMode(pin_, INPUT);
}

float BatteryVoltage::readVoltage() {
    float rawAdc = analogRead(pin_);
    float voltageRaw = (rawAdc / ADC_RESOLUTION) * ADC_REF_VOLTAGE;
    float voltageScalingFactor = (R1 + R2) / R2;
    return voltageRaw * voltageScalingFactor;
}

bool BatteryVoltage::isBatteryConnected() {
    float voltage = readVoltage();
    return (voltage <= OPEN_CIRCUIT_VOLTAGE);
}
