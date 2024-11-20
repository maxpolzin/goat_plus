#include "MPPT.h"

MPPT::MPPT()
    : step_size_(50), current_threshold_(60), prev_voltage_(0), prev_current_(0) {}

float MPPT::update(float voltage, float current) {
    if (current < current_threshold_) {
        return current_threshold_; // Return the minimum current limit
    }

    float delta_v = voltage - prev_voltage_;
    float delta_i = current - prev_current_;
    float adjustment = 0;

    if (delta_v == 0) {
        adjustment = (delta_i > 0) ? step_size_ : (delta_i < 0) ? -step_size_ : 0;
    } else {
        float incremental_conductance = delta_i / delta_v;
        float instantaneous_conductance = current / voltage;

        if (incremental_conductance + instantaneous_conductance > 0) {
            adjustment = step_size_;
        } else if (incremental_conductance + instantaneous_conductance < 0) {
            adjustment = -step_size_;
        }
    }

    prev_voltage_ = voltage;
    prev_current_ = current;

    return current + adjustment;
}
