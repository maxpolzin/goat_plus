#include "WheelController.h"

WheelController::WheelController(uint8_t ain1, uint8_t ain2, uint8_t bin1, uint8_t bin2, uint8_t pwmResolution, uint32_t pwmFrequency)
    : _ain1(ain1), _ain2(ain2), _bin1(bin1), _bin2(bin2), _pwmResolution(pwmResolution), _pwmFrequency(pwmFrequency) {}

void WheelController::begin() {
    pinMode(_ain1, OUTPUT);
    pinMode(_ain2, OUTPUT);
    pinMode(_bin1, OUTPUT);
    pinMode(_bin2, OUTPUT);

    ledcSetup(0, _pwmFrequency, _pwmResolution); // Channel 0 for AIN1
    ledcSetup(1, _pwmFrequency, _pwmResolution); // Channel 1 for AIN2
    ledcSetup(2, _pwmFrequency, _pwmResolution); // Channel 2 for BIN1
    ledcSetup(3, _pwmFrequency, _pwmResolution); // Channel 3 for BIN2

    ledcAttachPin(_ain1, 0);
    ledcAttachPin(_ain2, 1);
    ledcAttachPin(_bin1, 2);
    ledcAttachPin(_bin2, 3);
}

void WheelController::update(int forwardCommand, int steeringCommand) {
    int leftSpeed = forwardCommand + steeringCommand;
    int rightSpeed = forwardCommand - steeringCommand;

    int maxValue = max(max(abs(leftSpeed), abs(rightSpeed)), 512);
    leftSpeed = leftSpeed * 512 / maxValue;
    rightSpeed = rightSpeed * 512 / maxValue;

    setMotorSpeed(_ain1, _ain2, leftSpeed);
    setMotorSpeed(_bin1, _bin2, rightSpeed);


      // const int SERVO_MIN=500;
      // const int SERVO_MAX=2500;

      // int leftMotorPWM = map(leftWheelCommand, -511, 512, SERVO_MIN, SERVO_MAX);
      // int rightMotorPWM = map(rightWheelCommand, -511, 512, SERVO_MIN, SERVO_MAX);

      // uint16_t leftMotorSignal = map(leftMotorPWM, 0, 3000, 0, 255);
      // uint16_t rightMotorSignal = map(rightMotorPWM, 0, 3000, 0, 255);

      // ledcAnalogWrite(PWM_LEFT_CHANNEL, leftMotorSignal);
      // ledcAnalogWrite(PWM_RIGHT_CHANNEL, rightMotorSignal);

}

void WheelController::setMotorSpeed(uint8_t pin1, uint8_t pin2, int speed) {
    if (speed > 0) {
        ledcWrite(pin1, speed * 4095 / 512);
        ledcWrite(pin2, 0);
    } else if (speed < 0) {
        ledcWrite(pin1, 0);
        ledcWrite(pin2, -speed * 4095 / 512);
    } else {
        ledcWrite(pin1, 0); // Stop
        ledcWrite(pin2, 0);
    }
}
