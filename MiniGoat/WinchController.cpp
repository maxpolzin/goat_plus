#include "WinchController.h"

WinchController::WinchController(uint8_t ain1, uint8_t ain2, uint8_t bin1, uint8_t bin2)
    : _ain1(ain1), _ain2(ain2), _bin1(bin1), _bin2(bin2) {}

void WinchController::begin() {
    pinMode(_ain1, OUTPUT);
    pinMode(_ain2, OUTPUT);
    pinMode(_bin1, OUTPUT);
    pinMode(_bin2, OUTPUT);
}

void WinchController::update(bool up, bool down, bool left, bool right) {
    if (up) {
        setWinchDirection(_ain1, _ain2, 1); // Forward
    } else if (down) {
        setWinchDirection(_ain1, _ain2, -1); // Reverse
    } else {
        setWinchDirection(_ain1, _ain2, 0); // Stop
    }

    if (left) {
        setWinchDirection(_bin1, _bin2, 1); // Forward
    } else if (right) {
        setWinchDirection(_bin1, _bin2, -1); // Reverse
    } else {
        setWinchDirection(_bin1, _bin2, 0); // Stop
    }
}

void WinchController::setWinchDirection(uint8_t pin1, uint8_t pin2, int direction) {
    if (direction > 0) {
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, LOW);
    } else if (direction < 0) {
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH);
    } else {
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, LOW);
    }
}
