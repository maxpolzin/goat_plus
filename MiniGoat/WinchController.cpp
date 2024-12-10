#include "WinchController.h"

WinchController::WinchController(uint8_t ain1, uint8_t ain2, uint8_t bin1, uint8_t bin2, uint8_t pwmResolution, uint32_t pwmFrequency)
    : _ain1(ain1), _ain2(ain2), _bin1(bin1), _bin2(bin2), _pwmResolution(pwmResolution), _pwmFrequency(pwmFrequency) {}

void WinchController::begin() {
    pinMode(_ain1, OUTPUT);
    pinMode(_ain2, OUTPUT);
    pinMode(_bin1, OUTPUT);
    pinMode(_bin2, OUTPUT);

    // Reuse channels by sharing PWM for each winch's forward and reverse pins
    ledcSetup(2, _pwmFrequency, _pwmResolution); // Channel 2 for Winch A
    ledcSetup(3, _pwmFrequency, _pwmResolution); // Channel 3 for Winch B

    ledcAttachPin(_ain1, 2);
    ledcAttachPin(_ain2, 2);
    ledcAttachPin(_bin1, 3);
    ledcAttachPin(_bin2, 3);
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
        ledcWrite(2, speed * 4095 / 512); // Forward direction
        digitalWrite(pin2, LOW);          // Ensure reverse is off
    } else if (speed < 0) {
        ledcWrite(2, -speed * 4095 / 512); // Reverse direction
        digitalWrite(pin1, LOW);          // Ensure forward is off
    } else {
        digitalWrite(pin1, LOW); // Stop
        digitalWrite(pin2, LOW);
    }
}
