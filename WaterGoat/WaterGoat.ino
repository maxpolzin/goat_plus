#include <Bluepad32.h>
#include "SD_MMC.h"
#include "FS.h"
#include <Wire.h>
#include <Adafruit_INA219.h>
// #include <Adafruit_PWMServoDriver.h>


#define I2C_SDA 13
#define I2C_SCL 16

#define PWM_LEFT 4
#define PWM_RIGHT 12


ControllerPtr myControllers[BP32_MAX_GAMEPADS];
TwoWire twoWire = TwoWire(0);
Adafruit_INA219 ina219;
// Adafruit_PWMServoDriver pwm; 



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



    // convert axis commands to pwm signals for a skid steered rover
    


      int forwardCommand = ctl->axisY(); // -511, 512
      int steeringCommand = ctl->axisRX(); // -511, 512
      
      int leftWheelCommand = forwardCommand + steeringCommand;
      int rightWheelCommand = forwardCommand - steeringCommand;

      int maxValue = max(max(abs(leftWheelCommand), abs(rightWheelCommand)), 512);

      leftWheelCommand = leftWheelCommand * 512 / maxValue;
      rightWheelCommand = rightWheelCommand * 512 / maxValue;

      const int SERVO_MIN=1000;
      const int SERVO_MAX=2000;

      // Calculate the PWM signals for the left and right motors
      int leftMotorPWM = map(leftWheelCommand, -511, 512, SERVO_MIN, SERVO_MAX);
      int rightMotorPWM = map(rightWheelCommand, -511, 512, SERVO_MIN, SERVO_MAX);

      uint16_t leftMotorSignal = map(leftMotorPWM, 0, 20000, 0, 1023);
      uint16_t rightMotorSignal = map(rightMotorPWM, 0, 20000, 0, 1023);

      // // Write PWM signals to the motors
      analogWrite(PWM_LEFT, leftMotorSignal);
      analogWrite(PWM_RIGHT, rightMotorSignal);


      // // Read current from INA219 sensor
      float current_mA = ina219.getCurrent_mA();

      // Prepare the message to write to file
      char message[100];
      snprintf(message, sizeof(message), "Current: %.2f mA, Left PWM: %d, Right PWM: %d\n", current_mA, leftMotorPWM, rightMotorPWM);

      // Write current and PWM signals to file
      appendFile(SD_MMC, "/hello.csv", message);

      // Print the message to Serial for debugging
      Serial.print(message);


}





void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (!file.print(message)) {
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
                break;
            case 1:
                // Green
                ctl->setColorLED(0, 255, 127);
                break;
            case 2:
                // Blue
                ctl->setColorLED(0, 0, 255);
                break;
        }
        colorIdx++;
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

    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }


    // Output PWM from ESP32Cam directly
    pinMode(PWM_RIGHT, OUTPUT);
    pinMode(PWM_LEFT, OUTPUT);

    uint32_t frequency = 50;  // Set the PWM frequency to 50Hz.
    uint8_t resolution = 10;  // Set the resolution to 10 bits.

    analogWriteFrequency(frequency);  // Set the frequency.
    analogWriteResolution(resolution);  // Set the resolution.

    // Init I2C-bus
    twoWire.setPins(I2C_SDA, I2C_SCL);


    // Initialize the PCA9685
    // pwm = Adafruit_PWMServoDriver(PCA9685_I2C_ADDRESS, twoWire);
    // if (! pwm.begin()) {
    //   Serial.println("Failed to find PCA9685 PWM-Driver");
    //   while (1) { delay(10); }
    // }
    // pwm.setPWMFreq(SERVO_FREQ);
    // Serial.println("Controlling motors with PCA9685 PWM-Driver ...");


    // Initialize the INA219.
    if (! ina219.begin(&twoWire)) {
      Serial.println("Failed to find INA219 chip");
      while (1) { delay(10); }
    }
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
