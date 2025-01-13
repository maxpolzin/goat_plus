#include "BoostConverter.h"
#include <Arduino.h>

const float BoostConverter::stability_threshold_ = 100.0f; // mA

BoostConverter::BoostConverter()
    : history_index_(0), history_count_(0) {}

void BoostConverter::begin() {
    for (size_t i = 0; i < history_size_; ++i) {
        current_history_[i] = 0.0f;
    }
}

void BoostConverter::update(float pv_current) {
    current_history_[history_index_] = pv_current;
    history_index_ = (history_index_ + 1) % history_size_;
    if (history_count_ < history_size_) {
        history_count_++;
    }
}

bool BoostConverter::isStable() {
    if (history_count_ < 2) {
        return true;
    }

    for (size_t i = 1; i < history_count_; ++i) {
        size_t prevIndex = (history_index_ + history_size_ - i - 1) % history_size_;
        size_t currIndex = (history_index_ + history_size_ - i) % history_size_;
        float drop = current_history_[prevIndex] - current_history_[currIndex];
        if (drop > stability_threshold_) {
            return false;
        }
    }
    return true;
}

