#include "WheelController.h"

WheelController::WheelController(uint8_t ain1, uint8_t ain2, uint8_t bin1, uint8_t bin2, uint8_t pwmResolution, uint32_t pwmFrequency)
    : _ain1(ain1), _ain2(ain2), _bin1(bin1), _bin2(bin2), _pwmResolution(pwmResolution), _pwmFrequency(pwmFrequency) {}

void WheelController::begin() {
    pinMode(_ain1, OUTPUT);
    pinMode(_ain2, OUTPUT);
    pinMode(_bin1, OUTPUT);
    pinMode(_bin2, OUTPUT);

    // Reuse channels by sharing PWM for each motor's forward and reverse pins
    ledcSetup(0, _pwmFrequency, _pwmResolution); // Channel 0 for A Motor
    ledcSetup(1, _pwmFrequency, _pwmResolution); // Channel 1 for B Motor

    ledcAttachPin(_ain1, 0);
    ledcAttachPin(_ain2, 0);
    ledcAttachPin(_bin1, 1);
    ledcAttachPin(_bin2, 1);
}

void WheelController::update(int forwardCommand, int steeringCommand) {
    int leftSpeed = forwardCommand + steeringCommand;
    int rightSpeed = forwardCommand - steeringCommand;

    int maxValue = max(max(abs(leftSpeed), abs(rightSpeed)), 512);
    leftSpeed = leftSpeed * 512 / maxValue;
    rightSpeed = rightSpeed * 512 / maxValue;

    setMotorSpeed(_ain1, _ain2, leftSpeed);
    setMotorSpeed(_bin1, _bin2, rightSpeed);
}

void WheelController::setMotorSpeed(uint8_t pin1, uint8_t pin2, int speed) {
    if (speed > 0) {
        ledcWrite(0, speed * 4095 / 512); // Forward direction
        digitalWrite(pin2, LOW);         // Ensure reverse is off
    } else if (speed < 0) {
        ledcWrite(0, -speed * 4095 / 512); // Reverse direction
        digitalWrite(pin1, LOW);           // Ensure forward is off
    } else {
        digitalWrite(pin1, LOW); // Stop
        digitalWrite(pin2, LOW);
    }
}
