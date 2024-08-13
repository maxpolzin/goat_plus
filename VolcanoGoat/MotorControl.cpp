#include "MotorControl.h"
#include <Arduino.h>

MotorControl::MotorControl(int leftChannel, int rightChannel, int winchChannel, int cameraChannel)
  : PWM_LEFT_CHANNEL(leftChannel), PWM_RIGHT_CHANNEL(rightChannel), PWM_WINCH_CHANNEL(winchChannel), PWM_CAMERA_CHANNEL(cameraChannel) {}

void MotorControl::begin(int leftPin, int rightPin, int winchPin, int cameraPin) {
  ledcSetup(PWM_LEFT_CHANNEL, PWM_BASE_FREQ, PWM_TIMER_12_BIT);
  ledcAttachPin(leftPin, PWM_LEFT_CHANNEL);

  ledcSetup(PWM_RIGHT_CHANNEL, PWM_BASE_FREQ, PWM_TIMER_12_BIT);
  ledcAttachPin(rightPin, PWM_RIGHT_CHANNEL);

  ledcSetup(PWM_WINCH_CHANNEL, PWM_WINCH_BASE_FREQ, PWM_TIMER_12_BIT);
  ledcAttachPin(winchPin, PWM_WINCH_CHANNEL);

  ledcSetup(PWM_CAMERA_CHANNEL, PWM_CAMERA_BASE_FREQ, PWM_TIMER_12_BIT);
  ledcAttachPin(cameraPin, PWM_CAMERA_CHANNEL);
}

void MotorControl::update(double forwardVelocityCommand, double steeringVelocityCommand, double winchVelocityCommand, double cameraPositionCommand) {
  double leftWheelCommand = forwardVelocityCommand + steeringVelocityCommand;
  double rightWheelCommand = forwardVelocityCommand - steeringVelocityCommand;

  double maxValue = max(max(abs(leftWheelCommand), abs(rightWheelCommand)), 1.0);

  leftWheelCommand = leftWheelCommand / maxValue;
  rightWheelCommand = rightWheelCommand / maxValue;

  int leftMotorPulsewidth = map(leftWheelCommand*1000, -1000, 1000, SERVO_MIN, SERVO_MAX);
  int rightMotorPulsewidth = map(rightWheelCommand*1000, -1000, 1000, SERVO_MIN, SERVO_MAX);

  uint16_t leftMotorDutyCycle = map(leftMotorPulsewidth, 0, 1e6/PWM_BASE_FREQ, 0, 4095);
  uint16_t rightMotorDutyCycle = map(rightMotorPulsewidth, 0, 1e6/PWM_BASE_FREQ, 0, 4095);

  ledcAnalogWrite(PWM_LEFT_CHANNEL, leftMotorDutyCycle, 4095);
  ledcAnalogWrite(PWM_RIGHT_CHANNEL, rightMotorDutyCycle, 4095);


  int winchPulsewidth = map(winchVelocityCommand*1000, -1000, 1000, WINCH_SERVO_MIN, WINCH_SERVO_MAX);  
  uint16_t winchServoDutyCycle = map(winchPulsewidth, 0, 1e6/PWM_WINCH_BASE_FREQ, 0, 4095);
  ledcAnalogWrite(PWM_WINCH_CHANNEL, winchServoDutyCycle, 4095);

  int cameraPulsewidth = map(cameraPositionCommand*1000, -1000, 1000, CAMERA_SERVO_MIN, CAMERA_SERVO_MAX);  
  uint16_t cameraServoDutyCycle = map(cameraPulsewidth, 0, 1e6/PWM_CAMERA_BASE_FREQ, 0, 4095);
  ledcAnalogWrite(PWM_CAMERA_CHANNEL, cameraServoDutyCycle, 4095);

}

void MotorControl::ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax) {
  uint32_t duty = (4095 / valueMax) * min(value, valueMax);
  ledcWrite(channel, duty);
}
