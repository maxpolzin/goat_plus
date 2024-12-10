#ifndef WINCHCONTROLLER_H
#define WINCHCONTROLLER_H

#include <Arduino.h>

class WinchController {
public:
    WinchController(uint8_t ain1, uint8_t ain2, uint8_t bin1, uint8_t bin2, uint8_t pwmResolution = 12, uint32_t pwmFrequency = 100);
    void begin();
    void update(bool up, bool down, bool left, bool right);

private:
    uint8_t _ain1, _ain2, _bin1, _bin2;
    uint8_t _pwmResolution;
    uint32_t _pwmFrequency;

    void setWinchSpeed(uint8_t pin1, uint8_t pin2, int speed);
};

#endif // WINCHCONTROLLER_H
