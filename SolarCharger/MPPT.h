#ifndef MPPT_H
#define MPPT_H

class MPPT {
public:
    MPPT();

    float update(float voltage, float current);

private:
    const float step_size_;
    const float current_threshold_;
    float prev_voltage_;
    float prev_current_;
};

#endif
