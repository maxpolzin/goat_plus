#ifndef MPPT_H
#define MPPT_H

#include <cstdint>
#include <cmath>

class MPPT {
public:
    MPPT();

    float update(float voltage, float current);

private:
    const float step_size_increase_;
    const float step_size_decrease_;
    const float voltage_stability_threshold_;
    const int stability_check_steps_;
    const uint32_t pause_duration_ms_;

    float current_setpoint_;
    float prev_voltage_;
    float voltage_history_[5];
    int history_index_;
    int history_count_;
    uint32_t last_decrease_time_;

    bool isVoltageStable();
    bool isPaused();
};

#endif
