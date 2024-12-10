#include "WinchController.h"

WinchController::WinchController(uint8_t ain1, uint8_t ain2, uint8_t bin1, uint8_t bin2, uint8_t pwmResolution, uint32_t pwmFrequency)
    : _ain1(ain1), _ain2(ain2), _bin1(bin1), _bin2(bin2), 
      _pwmResolution(pwmResolution), _pwmFrequency(pwmFrequency) {
    _channels[_ain1] = 4;
    _channels[_ain2] = 5;
    _channels[_bin1] = 2;
    _channels[_bin2] = 3;
}

void WinchController::begin() {
    for (const auto& [pin, channel] : _channels) {
        ledcSetup(channel, _pwmFrequency, _pwmResolution);
        ledcAttachPin(pin, channel);
    }
}

void WinchController::update(bool up, bool down, bool left, bool right) {
    if (up) {
        setWinchSpeed(_ain1, _ain2, 4095);
    } else if (down) {
        setWinchSpeed(_ain1, _ain2, -4095);
    } else {
        setWinchSpeed(_ain1, _ain2, 0);
    }

    if (left) {
        setWinchSpeed(_bin1, _bin2, 4095);
    } else if (right) {
        setWinchSpeed(_bin1, _bin2, -4095);
    } else {
        setWinchSpeed(_bin1, _bin2, 0);
    }
}

void WinchController::setWinchSpeed(uint8_t pin1, uint8_t pin2, int speed) {
    uint8_t channel1 = _channels[pin1];
    uint8_t channel2 = _channels[pin2];

    if (speed > 0) {
        ledcWrite(channel1, speed);
        ledcWrite(channel2, 0);
    } else if (speed < 0) {
        ledcWrite(channel1, 0);
        ledcWrite(channel2, -speed);
    } else {
        ledcWrite(channel1, 0);
        ledcWrite(channel2, 0);
    }
}
