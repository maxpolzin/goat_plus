#include "GamepadHandler.h"

ControllerPtr GamepadHandler::myControllers[BP32_MAX_GAMEPADS] = { nullptr };

GamepadHandler::GamepadHandler()
  : forwardCommand(0), steeringCommand(0) {}

void GamepadHandler::begin() {
  BP32.setup(&GamepadHandler::onConnectedController, &GamepadHandler::onDisconnectedController);
}

void GamepadHandler::update() {
  BP32.update();

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    ControllerPtr myController = myControllers[i];

    if (myController && myController->isConnected()) {
      processGamepad(myController);
    }
  }
}

int GamepadHandler::getForwardCommand() {
  return forwardCommand;
}

int GamepadHandler::getSteeringCommand() {
  return steeringCommand;
}

void GamepadHandler::onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      myControllers[i] = ctl;
      break;
    }
  }
}

void GamepadHandler::onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      break;
    }
  }
}

void GamepadHandler::processGamepad(ControllerPtr ctl) {
  forwardCommand = abs(ctl->axisY()) < 50 ? 0 : ctl->axisY();
  steeringCommand = abs(ctl->axisRX()) < 50 ? 0 : ctl->axisRX();
}
