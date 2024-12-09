#include "MPPT.h"
#include <Arduino.h>

MPPT::MPPT()
    : step_size_increase_(5), step_size_decrease_(5), voltage_stability_threshold_(0.5), stability_check_steps_(5),
      pause_duration_ms_(30000), current_setpoint_(0), prev_voltage_(0), history_index_(0), history_count_(0), last_decrease_time_(0) {
    for (int i = 0; i < stability_check_steps_; i++) {
        voltage_history_[i] = 0.0;
    }
}

float MPPT::update(float voltage, float current) {
    voltage_history_[history_index_] = voltage;
    history_index_ = (history_index_ + 1) % stability_check_steps_;
    if (history_count_ < stability_check_steps_) {
        history_count_++;
    }

    if (isVoltageStable() && !isPaused()) {
        current_setpoint_ += step_size_increase_;
    } else if (std::abs(voltage - prev_voltage_) > voltage_stability_threshold_) {
        current_setpoint_ -= step_size_decrease_;
        if (current_setpoint_ < 0) {
            current_setpoint_ = 0;
        }
        last_decrease_time_ = millis();
    }

    prev_voltage_ = voltage;

    return current_setpoint_;
}

bool MPPT::isVoltageStable() {
    if (history_count_ < stability_check_steps_) {
        return false;
    }

    float min_voltage = voltage_history_[0];
    float max_voltage = voltage_history_[0];

    for (int i = 1; i < history_count_; i++) {
        if (voltage_history_[i] < min_voltage) min_voltage = voltage_history_[i];
        if (voltage_history_[i] > max_voltage) max_voltage = voltage_history_[i];
    }

    return (max_voltage - min_voltage) <= voltage_stability_threshold_;
}

bool MPPT::isPaused() {
    return (millis() - last_decrease_time_) < pause_duration_ms_;
}
