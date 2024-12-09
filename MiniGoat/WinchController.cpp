#include "WinchController.h"

WinchController::WinchController(uint8_t ain1, uint8_t ain2, uint8_t bin1, uint8_t bin2, uint8_t pwmResolution, uint32_t pwmFrequency)
    : _ain1(ain1), _ain2(ain2), _bin1(bin1), _bin2(bin2), _pwmResolution(pwmResolution), _pwmFrequency(pwmFrequency) {}

void WinchController::begin() {
    pinMode(_ain1, OUTPUT);
    pinMode(_ain2, OUTPUT);
    pinMode(_bin1, OUTPUT);
    pinMode(_bin2, OUTPUT);

    ledcSetup(4, _pwmFrequency, _pwmResolution); // Channel 4 for AIN1
    ledcSetup(5, _pwmFrequency, _pwmResolution); // Channel 5 for AIN2
    ledcSetup(6, _pwmFrequency, _pwmResolution); // Channel 6 for BIN1
    ledcSetup(7, _pwmFrequency, _pwmResolution); // Channel 7 for BIN2

    ledcAttachPin(_ain1, 4);
    ledcAttachPin(_ain2, 5);
    ledcAttachPin(_bin1, 6);
    ledcAttachPin(_bin2, 7);
}

void WinchController::update(bool up, bool down, bool left, bool right) {
    if (up) {
        setWinchSpeed(_ain1, _ain2, -512); // Winch A reels out
    } else if (down) {
        setWinchSpeed(_ain1, _ain2, 512); // Winch A reels in
    } else {
        setWinchSpeed(_ain1, _ain2, 0); // Stop Winch A
    }

    if (left) {
        setWinchSpeed(_bin1, _bin2, 512); // Winch B reels in
    } else if (right) {
        setWinchSpeed(_bin1, _bin2, -512); // Winch B reels out
    } else {
        setWinchSpeed(_bin1, _bin2, 0); // Stop Winch B
    }
}

void WinchController::setWinchSpeed(uint8_t pin1, uint8_t pin2, int speed) {
    if (speed > 0) {
        ledcWrite(pin1, speed * 4095 / 512); // Forward direction
        ledcWrite(pin2, 0);
    } else if (speed < 0) {
        ledcWrite(pin1, 0);
        ledcWrite(pin2, -speed * 4095 / 512); // Reverse direction
    } else {
        ledcWrite(pin1, 0); // Stop
        ledcWrite(pin2, 0);
    }
}
