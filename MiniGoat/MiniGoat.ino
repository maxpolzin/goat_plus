#include <Bluepad32.h>
#include "WheelController.h"
#include "WinchController.h"

#define WINCH_AIN1_PIN D3
#define WINCH_AIN2_PIN D4
#define WINCH_BIN1_PIN D5
#define WINCH_BIN2_PIN D6

#define WHEEL_AIN1_PIN D7
#define WHEEL_AIN2_PIN D8
#define WHEEL_BIN1_PIN D9
#define WHEEL_BIN2_PIN D10

WinchController winchController(WINCH_AIN1_PIN, WINCH_AIN2_PIN, WINCH_BIN1_PIN, WINCH_BIN2_PIN);
WheelController wheelController(WHEEL_AIN1_PIN, WHEEL_AIN2_PIN, WHEEL_BIN1_PIN, WHEEL_BIN2_PIN);

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

void onConnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n",
                          ctl->getModelName().c_str(), properties.vendor_id, properties.product_id);
            myControllers[i] = ctl;
            return;
        }
    }
    Serial.println("CALLBACK: Controller connected, but could not find an empty slot");
}

void onDisconnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            return;
        }
    }
    Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
}

void processGamepad(ControllerPtr ctl) {
    int forwardCommand = abs(ctl->axisY()) < 50 ? 0 : ctl->axisY();
    int steeringCommand = abs(ctl->axisRX()) < 50 ? 0 : ctl->axisRX();
    wheelController.update(forwardCommand, steeringCommand);

    uint8_t dpadState = ctl->dpad();
    bool up = (dpadState & DPAD_UP);
    bool down = (dpadState & DPAD_DOWN);
    bool left = (dpadState & DPAD_LEFT);
    bool right = (dpadState & DPAD_RIGHT);


    uint16_t buttonState = ctl->buttons();
    const uint16_t BUTTON_TRIANGLE = (1 << 12);  // Triangle button
    const uint16_t BUTTON_CROSS = (1 << 13);     // Cross (X) button
    const uint16_t BUTTON_CIRCLE = (1 << 14);    // Circle button
    const uint16_t BUTTON_SQUARE = (1 << 15);    // Square button
    if (buttonState & BUTTON_TRIANGLE) {
        up = true; down = false; left = false; right = true;
    } else if (buttonState & BUTTON_CROSS) {
        up = false; down = true; left = true; right = false;
    } else if (buttonState & BUTTON_CIRCLE) {
        up = true; down = false; left = true; right = false;
    } else if (buttonState & BUTTON_SQUARE) {
        up = false; down = true; left = false; right = true;
    }

    // winchController.update(up, down, left, right);

    Serial.printf("Buttons: 0x%04x, Up: %d, Down: %d, Left: %d, Right: %d\n", buttonState, up, down, left, right);
}

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(10);
    }

    wheelController.begin();
    winchController.begin();

    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %02X:%02X:%02X:%02X:%02X:%02X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.enableVirtualDevice(false);
}

void loop() {
    BP32.update();

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        ControllerPtr myController = myControllers[i];
        if (myController && myController->isConnected()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            }
        }
    }

    // winchController.update(false,false,false,true);
    wheelController.update(250, 0); 

    delay(50);
}
