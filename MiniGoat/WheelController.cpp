#include "WheelController.h"

WheelController::WheelController(uint8_t ain1, uint8_t ain2, uint8_t bin1, uint8_t bin2, uint8_t pwmResolution, uint32_t pwmFrequency)
    : _ain1(ain1), _ain2(ain2), _bin1(bin1), _bin2(bin2), 
      _pwmResolution(pwmResolution), _pwmFrequency(pwmFrequency) {
    _channels[_ain1] = 0;
    _channels[_ain2] = 1;
    _channels[_bin1] = 2;
    _channels[_bin2] = 3;
}

void WheelController::begin() {
    for (const auto& [pin, channel] : _channels) {
        ledcSetup(channel, _pwmFrequency, _pwmResolution);
        ledcAttachPin(pin, channel);
    }
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
    uint8_t channel1 = _channels[pin1];
    uint8_t channel2 = _channels[pin2];

    if (speed > 0) {
        ledcWrite(channel1, speed * 4095 / 512);
        ledcWrite(channel2, 0);
    } else if (speed < 0) {
        ledcWrite(channel1, 0);
        ledcWrite(channel2, -speed * 4095 / 512);
    } else {
        ledcWrite(channel1, 0);
        ledcWrite(channel2, 0);
    }
}
