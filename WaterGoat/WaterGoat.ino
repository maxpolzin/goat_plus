#include <Bluepad32.h>
#include "SD_MMC.h"
#include "FS.h"
#include <Adafruit_INA219.h>
#include <Adafruit_PWMServoDriver.h>

#include <Wire.h>

// int motorPin = 12;      // left_motor

#define BUILTIN_LED 4

#define I2C_SDA 13
#define I2C_SCL 16


#define USMIN  1000
#define USMAX  2000 
#define SERVO_FREQ 50

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

TwoWire twoWire = TwoWire(0);
Adafruit_INA219 ina219;
Adafruit_PWMServoDriver pwm; 



// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

void dumpGamepad(ControllerPtr ctl) {
    // Serial.printf(
    //     "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
    //     "misc: 0x%02x, gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d\n",
    //     ctl->index(),        // Controller Index
    //     ctl->dpad(),         // DPAD
    //     ctl->buttons(),      // bitmask of pressed buttons
    //     ctl->axisX(),        // (-511 - 512) left X Axis
    //     ctl->axisY(),        // (-511 - 512) left Y axis
    //     ctl->axisRX(),       // (-511 - 512) right X axis
    //     ctl->axisRY(),       // (-511 - 512) right Y axis
    //     ctl->brake(),        // (0 - 1023): brake button
    //     ctl->throttle(),     // (0 - 1023): throttle (AKA gas) button
    //     ctl->miscButtons(),  // bitmak of pressed "misc" buttons
    //     ctl->gyroX(),        // Gyro X
    //     ctl->gyroY(),        // Gyro Y
    //     ctl->gyroZ(),        // Gyro Z
    //     ctl->accelX(),       // Accelerometer X
    //     ctl->accelY(),       // Accelerometer Y
    //     ctl->accelZ()        // Accelerometer Z
    // );

}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
}

void processGamepad(ControllerPtr ctl) {
    // There are different ways to query whether a button is pressed.
    // By query each button individually:
    //  a(), b(), x(), y(), l1(), etc...
    if (ctl->a()) {
        static int colorIdx = 0;
        // Some gamepads like DS4 and DualSense support changing the color LED.
        // It is possible to change it by calling:
        switch (colorIdx % 3) {
            case 0:
                // Red
                ctl->setColorLED(255, 0, 0);
                // analogWrite(motorPin, 0);

                appendFile(SD_MMC, "/hello.csv", "World red\n");
                analogWrite(12, 20);
                analogWrite(4, 20);

                break;
            case 1:
                // Green
                ctl->setColorLED(0, 255, 127);

                appendFile(SD_MMC, "/hello.csv", "World green\n");
                analogWrite(12, 100);
                analogWrite(4, 100);
                // analogWrite(motorPin, 0);
                break;
            case 2:
                // Blue
                ctl->setColorLED(0, 0, 255);
                // analogWrite(motorPin, 0);
                appendFile(SD_MMC, "/hello.csv", "World blue\n");
                analogWrite(12, 200);
                analogWrite(4, 200);
                break;
        }
        colorIdx++;
    }

    if (ctl->b()) {
        // Turn on the 4 LED. Each bit represents one LED.
        static int led = 0;
        led++;
        // Some gamepads like the DS3, DualSense, Nintendo Wii, Nintendo Switch
        // support changing the "Player LEDs": those 4 LEDs that usually indicate
        // the "gamepad seat".
        // It is possible to change them by calling:
        ctl->setPlayerLEDs(led & 0x0f);
    }

    if (ctl->x()) {
        // Duration: 255 is ~2 seconds
        // force: intensity
        // Some gamepads like DS3, DS4, DualSense, Switch, Xbox One S support
        // rumble.
        // It is possible to set it by calling:
        ctl->setRumble(0xc0 /* force */, 0xc0 /* duration */);
    }

    // Another way to query controller data is by getting the buttons() function.
    // See how the different "dump*" functions dump the Controller info.
    dumpGamepad(ctl);
}


// Arduino setup function. Runs in CPU 1
void setup() {

    Serial.begin(115200);

    // Initialize the SD card
    if (!SD_MMC.begin("/sdcard", true)){
        Serial.println("Failed to mount SD card");
        return;
    }

    // Specify that LED pin
    pinMode(BUILTIN_LED, OUTPUT);
    digitalWrite(BUILTIN_LED, LOW);

    // Check for an SD card
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }


    twoWire.setPins(I2C_SDA, I2C_SCL);


    pwm = Adafruit_PWMServoDriver(PCA9685_I2C_ADDRESS, twoWire);
    if (! pwm.begin()) {
      Serial.println("Failed to find PCA9685 PWM-Driver");
      while (1) { delay(10); }
    }
    pwm.setPWMFreq(SERVO_FREQ);
    Serial.println("Controlling motors with PCA9685 PWM-Driver ...");


    pinMode(12, OUTPUT);
    pinMode(4, OUTPUT);


    // Initialize the INA219.
    // By default the initialization will use the largest range (32V, 2A).  However
    // you can call a setCalibration function to change this range (see comments).
    if (! ina219.begin(&twoWire)) {
      Serial.println("Failed to find INA219 chip");
      while (1) { delay(10); }
    }
    // To use a slightly lower 32V, 1A range (higher precision on amps):
    //ina219.setCalibration_32V_1A();
    // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
    //ina219.setCalibration_16V_400mA();

    Serial.println("Measuring voltage and current with INA219 ...");





    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);


    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // "forgetBluetoothKeys()" should be called when the user performs
    // a "device factory reset", or similar.
    // Calling "forgetBluetoothKeys" in setup() just as an example.
    // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
    // But might also fix some connection / re-connection issues.
    // BP32.forgetBluetoothKeys();

    // Enables mouse / touchpad support for gamepads that support them.
    // When enabled controllers like DualSense and DualShock4 generate two connected devices:
    // - First one: the gamepad
    // - Second one, which is a "vritual device", is a mouse
    // By default it is disabled.
    BP32.enableVirtualDevice(false);
}

// Arduino loop function. Runs in CPU 1
void loop() {
    // This call fetches all the gamepad info from the NINA (ESP32) module.
    // Just call this function in your main loop.
    // The gamepads pointer (the ones received in the callbacks) gets updated
    // automatically.
    BP32.update();

    // It is safe to always do this before using the gamepad API.
    // This guarantees that the gamepad is valid and connected.
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        ControllerPtr myController = myControllers[i];

        if (myController && myController->isConnected()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            } else {
                Serial.printf("Data not available yet\n");
                continue;
            }
            // See ArduinoController.h for all the available functions.
        }
    }
    // The main loop must have some kind of "yield to lower priority task" event.
    // Otherwise the watchdog will get triggered.
    // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
    // Detailed info here:
    // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time

    // vTaskDelay(1);
    delay(50);
}
