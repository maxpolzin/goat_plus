#ifndef BOOSTCONVERTER_H
#define BOOSTCONVERTER_H

#include <cstddef>

class BoostConverter {
public:
    BoostConverter();
    void begin();                   // Initialization method
    void update(float pv_current);  // Update current readings
    bool isStable();                // Check stability

private:
    static const size_t history_size_ = 8;
    static const float stability_threshold_;
    static const float min_operating_current_; // mA

    float current_history_[history_size_];
    size_t history_index_;
    size_t history_count_;
};

#endif // BOOSTCONVERTER_H
