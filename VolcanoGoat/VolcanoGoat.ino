#include <HardwareSerial.h>




#include "DJIArmer.h"
#include "SBUSReader.h"
#include "GamepadHandler.h"
#include "MotorControl.h"



int8_t DJI_RX_PIN = D3;
int8_t DJI_TX_PIN = D4;
HardwareSerial DJI_SERIAL(0);
DJIArmer djiArmer(DJI_SERIAL, DJI_RX_PIN, DJI_TX_PIN);


int8_t SBUS_RX_PIN = D5;
int8_t SBUS_TX_PIN = D6;
HardwareSerial SBUS_SERIAL(1);
SBUSReader sbusReader(SBUS_SERIAL, SBUS_RX_PIN, SBUS_TX_PIN);


GamepadHandler gamepadHandler;


int LEFT_MOTOR_PIN = D0;
int RIGHT_MOTOR_PIN = D1;
int WINCH_MOTOR_PIN = D2;
int CAMERA_MOTOR_PIN = D3;
int LEFT_MOTOR_PWM_CHANNEL = 0;
int RIGHT_MOTOR_PWM_CHANNEL = 1;
int WINCH_MOTOR_PWM_CHANNEL = 2;
int CAMERA_MOTOR_PWM_CHANNEL = 3;
MotorControl motorControl(LEFT_MOTOR_PWM_CHANNEL, RIGHT_MOTOR_PWM_CHANNEL, WINCH_MOTOR_PWM_CHANNEL, CAMERA_MOTOR_PWM_CHANNEL);


void setup() {
  Serial.begin(115200);

  while(!Serial){
    delay(1);
  }

  Serial.println("Board starting...");

  djiArmer.begin();
  sbusReader.begin();

  // gamepadHandler.begin();
  // motorControl.begin(LEFT_MOTOR_PIN, RIGHT_MOTOR_PIN, WINCH_MOTOR_PIN, CAMERA_MOTOR_PIN);
}

void loop() {

  djiArmer.update();

  sbusReader.update();

  gamepadHandler.update();

  // // todo
  // - allow failure in sbus reader or gamepadhandler
  // - make djiarmer optional
  // - normalize inputs from gamepadhandler and sbusreceiver
  // - enable controlmode priority
    

  // int forwardCommand = gamepadHandler.getForwardCommand();
  // int steeringCommand = gamepadHandler.getSteeringCommand();
  // int winchCommand = 0;  
  // int cameraCommand = 0;

  // motorControl.update(forwardCommand, steeringCommand, winchCommand, cameraCommand);

  // Serial.println("Update...");

  delay(1);
}
