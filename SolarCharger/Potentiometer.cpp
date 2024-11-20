#include "Potentiometer.h"

Potentiometer::Potentiometer(int pin)
    : min_current_(80), max_current_(400), min_pwm_(500), max_pwm_(2500), pin_(pin) {}

void Potentiometer::setup() {
    sg90_.setPeriodHertz(50);
    sg90_.attach(pin_);
    sg90_.writeMicroseconds(max_pwm_); // Start with maximum PWM (80mA equivalent)
}

void Potentiometer::setCurrent(float current) {
    int pwm_signal = mapCurrentToPWM(current);
    sg90_.writeMicroseconds(pwm_signal);
}

int Potentiometer::mapCurrentToPWM(float current) {
    if (current < min_current_) return max_pwm_;
    if (current > max_current_) return min_pwm_;

    return (int)((max_pwm_ - min_pwm_) * (max_current_ - current) / (max_current_ - min_current_) + min_pwm_);
}
