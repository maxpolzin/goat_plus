#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

class Potentiometer {
public:
    Potentiometer(int pin);
    void begin();
    void setCurrent(float current);

private:
    const float min_current_;
    const float max_current_;
    const int min_pwm_;
    const int max_pwm_;
    int pin_;
    int mapCurrentToPWM(float current);
};

#endif
