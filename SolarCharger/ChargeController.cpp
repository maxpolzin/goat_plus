#include "ChargeController.h"

ChargeController::ChargeController(float maximum_voltage)
    : maximum_voltage_(maximum_voltage),
      target_voltage_(5.25f),
      step_size_(5.0f),
      current_stability_threshold_(200.0f), //mA
      stability_check_steps_(5),
      current_setpoint_(0),
      history_index_(0),
      history_count_(0),
      in_hysteresis_(false)
{
    for (int i = 0; i < stability_check_steps_; i++) {
        current_history_[i] = 0.0f;
    }
}

float ChargeController::update(float pv_voltage, float pv_current) {
    current_history_[history_index_] = pv_current;
    history_index_ = (history_index_ + 1) % stability_check_steps_;
    if (history_count_ < stability_check_steps_) {
        history_count_++;
    }

    if (history_count_ < 2) {
        return current_setpoint_;
    }

    bool stable = isCurrentStable();
    bool battery_connected = isBatteryConnected();

    if (!battery_connected) {
        return current_setpoint_;
    }

    if (stable) {
        if (in_hysteresis_) {
            if (pv_voltage < 5.0f || pv_voltage > 5.5f) {
                in_hysteresis_ = false;
            } else {
                return current_setpoint_;
            }
        }
        if (pv_voltage > target_voltage_) {
            current_setpoint_ += step_size_;
        } else {
            current_setpoint_ -= step_size_;
        }
        if (pv_voltage >= 5.0f && pv_voltage <= 5.5f) {
            in_hysteresis_ = true;
        }
    } else {
        current_setpoint_ -= step_size_;
    }

    if (current_setpoint_ < 0) {
        current_setpoint_ = 0;
    }

    return current_setpoint_;
}

bool ChargeController::isCurrentStable() {
    if (history_count_ < stability_check_steps_) {
        return false;
    }

    float min_current = current_history_[0];
    float max_current = current_history_[0];

    for (int i = 1; i < history_count_; i++) {
        if (current_history_[i] < min_current) min_current = current_history_[i];
        if (current_history_[i] > max_current) max_current = current_history_[i];
    }

    return (max_current - min_current) <= current_stability_threshold_;
}

bool ChargeController::isBatteryConnected() {
    if (history_count_ < stability_check_steps_) {
        return true;
    }

    float max_current = current_history_[0];
    for (int i = 1; i < history_count_; i++) {
        if (current_history_[i] > max_current) max_current = current_history_[i];
    }

    return (max_current >= 60.0f);
}
