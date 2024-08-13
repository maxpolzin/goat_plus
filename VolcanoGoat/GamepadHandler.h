#ifndef GAMEPAD_HANDLER_H
#define GAMEPAD_HANDLER_H

#include <Bluepad32.h>

class GamepadHandler {
  static ControllerPtr myControllers[BP32_MAX_GAMEPADS];
  int forwardCommand;
  int steeringCommand;
  int winchCommand;
  int cameraCommand;

public:
  GamepadHandler();
  void begin();
  void update();
  int getForwardCommand();
  int getSteeringCommand();

private:
  static void onConnectedController(ControllerPtr ctl);
  static void onDisconnectedController(ControllerPtr ctl);
  void processGamepad(ControllerPtr ctl);
};

#endif
