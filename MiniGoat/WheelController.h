#ifndef WHEELCONTROLLER_H
#define WHEELCONTROLLER_H

#include <Arduino.h>

class WheelController {
public:
    // Constructor to initialize motor driver pins
    WheelController(uint8_t ain1, uint8_t ain2, uint8_t bin1, uint8_t bin2, uint8_t pwmResolution = 12, uint32_t pwmFrequency = 333);

    // Initialize the motor driver
    void begin();

    // Method to update motor speeds based on forward and steering commands
    void update(int forwardCommand, int steeringCommand);

private:
    uint8_t _ain1, _ain2, _bin1, _bin2;
    uint8_t _pwmResolution;
    uint32_t _pwmFrequency;

    void setMotorSpeed(uint8_t pin1, uint8_t pin2, int speed);
};

#endif // WHEELCONTROLLER_H
