#include <HardwareSerial.h>

#include "DJIArmer.h"
#include "SBUSReader.h"
#include "GamepadHandler.h"
#include "MotorControl.h"


#define DEBUG 1  // Enable or disable debug mode


int8_t DJI_RX_PIN = D3;
int8_t DJI_TX_PIN = D4;
HardwareSerial DJI_SERIAL(0); 
DJIArmer djiArmer(DJI_SERIAL, DJI_RX_PIN, DJI_TX_PIN);


int8_t SBUS_RX_PIN = D10;
int8_t SBUS_TX_PIN = D9;
HardwareSerial SBUS_SERIAL(1);
SBUSReader sbusReader(SBUS_SERIAL, SBUS_RX_PIN, SBUS_TX_PIN);


GamepadHandler gamepadHandler;


int LEFT_MOTOR_PIN = D7;
int RIGHT_MOTOR_PIN = D8;
int WINCH_MOTOR_PIN = D9;
int LEFT_MOTOR_PWM_CHANNEL = 0;
int RIGHT_MOTOR_PWM_CHANNEL = 1;
int WINCH_MOTOR_PWM_CHANNEL = 2;
MotorControl motorControl(LEFT_MOTOR_PWM_CHANNEL, RIGHT_MOTOR_PWM_CHANNEL, WINCH_MOTOR_PWM_CHANNEL);


void setup() {
  Serial.begin(115200);

  djiArmer.begin();
  sbusReader.begin();

  gamepadHandler.begin();
  motorControl.begin(LEFT_MOTOR_PIN, RIGHT_MOTOR_PIN, WINCH_MOTOR_PIN);

}

void loop() {

  djiArmer.update();
  sbusReader.update();
  gamepadHandler.update();

  int forwardCommand = gamepadHandler.getForwardCommand();
  int steeringCommand = gamepadHandler.getSteeringCommand();
  int winchCommand = 0;  // Assume winch command is either a button press or a third axis, adjust as needed

  motorControl.update(forwardCommand, steeringCommand, winchCommand);


  delay(100);
}
