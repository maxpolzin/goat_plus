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

  const int PWM_WINCH_BASE_FREQ = 50;
  const int WINCH_SERVO_MIN = 1000;
  const int WINCH_SERVO_MAX = 2000;
  const int PWM_WINCH_CHANNEL;

  const int PWM_CAMERA_BASE_FREQ = 50;
  const int CAMERA_SERVO_MIN = 500;
  const int CAMERA_SERVO_MAX = 2400;
  const int PWM_CAMERA_CHANNEL;

public:
  MotorControl(int leftChannel, int rightChannel, int winchChannel, int cameraChannel);
  void begin(int leftPin, int rightPin, int winchPin, int cameraPin);
  void update(double forwardVelocityCommand, double steeringVelocityCommand, double winchVelocityCommand, double cameraPositionCommand);

private:
  void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255);
};

#endif
