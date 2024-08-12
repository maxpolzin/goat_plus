#include "MotorControl.h"
#include <Arduino.h>

MotorControl::MotorControl(int leftChannel, int rightChannel, int winchChannel, int cameraChannel)
  : PWM_LEFT_CHANNEL(leftChannel), PWM_RIGHT_CHANNEL(rightChannel), PWM_WINCH_CHANNEL(winchChannel), PWM_CAMERA_CHANNEL(cameraChannel) {}

void MotorControl::begin(int leftPin, int rightPin, int winchPin, int cameraPin) {
  ledcSetup(PWM_LEFT_CHANNEL, PWM_BASE_FREQ, PWM_TIMER_12_BIT);
  ledcAttachPin(leftPin, PWM_LEFT_CHANNEL);

  ledcSetup(PWM_RIGHT_CHANNEL, PWM_BASE_FREQ, PWM_TIMER_12_BIT);
  ledcAttachPin(rightPin, PWM_RIGHT_CHANNEL);

  ledcSetup(PWM_WINCH_CHANNEL, PWM_BASE_FREQ, PWM_TIMER_12_BIT);
  ledcAttachPin(winchPin, PWM_WINCH_CHANNEL);

  ledcSetup(PWM_CAMERA_CHANNEL, PWM_BASE_FREQ, PWM_TIMER_12_BIT);
  ledcAttachPin(cameraPin, PWM_CAMERA_CHANNEL);
}

void MotorControl::update(int forwardCommand, int steeringCommand, int winchCommand, int cameraCommand) {
  int leftWheelCommand = forwardCommand + steeringCommand;
  int rightWheelCommand = forwardCommand - steeringCommand;

  int maxValue = max(max(abs(leftWheelCommand), abs(rightWheelCommand)), 512);

  leftWheelCommand = leftWheelCommand * 512 / maxValue;
  rightWheelCommand = rightWheelCommand * 512 / maxValue;

  int leftMotorPWM = map(leftWheelCommand, -511, 512, SERVO_MIN, SERVO_MAX);
  int rightMotorPWM = map(rightWheelCommand, -511, 512, SERVO_MIN, SERVO_MAX);
  int winchPWM = map(winchCommand, -511, 512, SERVO_MIN, SERVO_MAX);
  int cameraPWM = map(cameraCommand, -511, 512, SERVO_MIN, SERVO_MAX);

  uint16_t leftMotorSignal = map(leftMotorPWM, 0, 3000, 0, 255);
  uint16_t rightMotorSignal = map(rightMotorPWM, 0, 3000, 0, 255);
  uint16_t winchMotorSignal = map(winchPWM, 0, 3000, 0, 255);
  uint16_t cameraMotorSignal = map(cameraPWM, 0, 3000, 0, 255);

  ledcAnalogWrite(PWM_LEFT_CHANNEL, leftMotorSignal);
  ledcAnalogWrite(PWM_RIGHT_CHANNEL, rightMotorSignal);
  ledcAnalogWrite(PWM_WINCH_CHANNEL, winchMotorSignal);
  ledcAnalogWrite(PWM_CAMERA_CHANNEL, cameraMotorSignal);
}

void MotorControl::ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax) {
  uint32_t duty = (4095 / valueMax) * min(value, valueMax);
  ledcWrite(channel, duty);
}
