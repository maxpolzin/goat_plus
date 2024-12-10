#ifndef WHEELCONTROLLER_H
#define WHEELCONTROLLER_H

#include <Arduino.h>
#include <map>

class WheelController {
public:
    WheelController(uint8_t ain1, uint8_t ain2, uint8_t bin1, uint8_t bin2, uint8_t pwmResolution = 12, uint32_t pwmFrequency = 100);
    void begin();
    void update(int forwardCommand, int steeringCommand);

private:
    void setMotorSpeed(uint8_t pin1, uint8_t pin2, int speed);

    uint8_t _ain1, _ain2, _bin1, _bin2;
    uint8_t _pwmResolution;
    uint32_t _pwmFrequency;
    std::map<uint8_t, uint8_t> _channels;
};

#endif
