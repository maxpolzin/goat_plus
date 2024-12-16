#include "Potentiometer.h"
#include <Arduino.h>

Potentiometer::Potentiometer(int pin)
    : min_current_(60), max_current_(400), min_pwm_(2500), max_pwm_(500), pin_(pin) {}

void Potentiometer::setup() {
    pinMode(pin_, OUTPUT);
    analogWriteFrequency(pin_, 50);
    analogWriteResolution(12);
    analogWrite(pin_, 0);
}

void Potentiometer::setCurrent(float current) {
    if (current < min_current_) current = min_current_;
    if (current > max_current_) current = max_current_;
    int pwm_signal_us = mapCurrentToPWM(current);
    int pwm_value = (pwm_signal_us * 4095) / 20000; // Convert microseconds to 12-bit duty cycle
  
    analogWrite(pin_, pwm_value);
}

int Potentiometer::mapCurrentToPWM(float current) {
    return (int)((current - min_current_) / (max_current_ - min_current_) * (max_pwm_ - min_pwm_) + min_pwm_);
}
