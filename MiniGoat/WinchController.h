#ifndef WINCHCONTROLLER_H
#define WINCHCONTROLLER_H

#include <Arduino.h>

class WinchController {
public:
    WinchController(uint8_t ain1, uint8_t ain2, uint8_t bin1, uint8_t bin2);
    void begin();
    void update(bool up, bool down, bool left, bool right);

private:
    void setWinchDirection(uint8_t pin1, uint8_t pin2, int direction);

    uint8_t _ain1, _ain2, _bin1, _bin2;
};

#endif
