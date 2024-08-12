#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

class MotorControl {
  const int SERVO_MIN = 500;
  const int SERVO_MAX = 2500;
  const int PWM_TIMER_12_BIT = 12;
  const int PWM_BASE_FREQ = 333;
  const int PWM_LEFT_CHANNEL;
  const int PWM_RIGHT_CHANNEL;
  const int PWM_WINCH_CHANNEL;
  const int PWM_CAMERA_CHANNEL;

public:
  MotorControl(int leftChannel, int rightChannel, int winchChannel, int cameraChannel);
  void begin(int leftPin, int rightPin, int winchPin, int cameraPin);
  void update(int forwardCommand, int steeringCommand, int winchCommand, int cameraCommand);

private:
  void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255);
};

#endif
