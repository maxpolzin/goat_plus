#ifndef CHARGECONTROLLER_H
#define CHARGECONTROLLER_H

#include <cstdint>
#include <cmath>
#include <Arduino.h>

class ChargeController {
public:
    ChargeController(float maximum_voltage);
    float update(float pv_voltage, float pv_current);

private:
    float maximum_voltage_;
    float target_voltage_;
    float step_size_;
    float current_stability_threshold_;
    int stability_check_steps_;

    float current_setpoint_;
    float current_history_[5];
    int history_index_;
    int history_count_;
    bool in_hysteresis_;

    bool isCurrentStable();
    bool isBatteryConnected();
};

#endif
