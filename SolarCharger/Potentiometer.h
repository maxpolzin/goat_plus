#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <ESP32Servo.h>

class Potentiometer {
public:
    Potentiometer(int pin);

    void setup();
    void setCurrent(float current);

private:
    const float min_current_;
    const float max_current_;
    const int min_pwm_;
    const int max_pwm_;
    int pin_;
    Servo sg90_;

    int mapCurrentToPWM(float current);
};

#endif
