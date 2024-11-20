#include "SD_MMC.h"
#include "FS.h"

#include <Adafruit_INA219.h>
#include "Potentiometer.h"
#include "MPPT.h"

#define I2C_SDA 14
#define I2C_SCL 15

#define PIN_SG90 16

TwoWire twoWire = TwoWire(0);
Adafruit_INA219 ina219_pv(0x40);  // Sensor for PV panel
Adafruit_INA219 ina219_battery(0x41);  // Sensor for battery

Potentiometer potentiometer(PIN_SG90);
MPPT mppt;

String filename;


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

    // Determine the next available file name
    filename = getNextFilename();

    // Write CSV headers
    File file = SD_MMC.open(filename, FILE_WRITE);
    if (file) {
        file.println("pv_current,pv_voltage,charging_current_limit,battery_current,battery_voltage");
        file.close();
    } else {
        Serial.println("Failed to open file for writing");
    }


    twoWire.setPins(I2C_SDA, I2C_SCL);

    if (!ina219_pv.begin(&twoWire)) {
        Serial.println("Failed to find INA219 chip for PV panel");
        while (1) { delay(10); }
    }
    Serial.println("Initialized INA219 for PV panel");

    if (!ina219_battery.begin(&twoWire)) {
        Serial.println("Failed to find INA219 chip for battery");
        while (1) { delay(10); }
    }
    Serial.println("Initialized INA219 for battery");


    potentiometer.setup();
}

void loop() {
    delay(2000);

    float pv_current_mA = ina219_pv.getCurrent_mA();
    float pv_voltage_V = ina219_pv.getBusVoltage_V() + (ina219_pv.getShuntVoltage_mV() / 1000);

    float battery_current_mA = ina219_battery.getCurrent_mA();
    float battery_voltage_V = ina219_battery.getBusVoltage_V() + (ina219_battery.getShuntVoltage_mV() / 1000);

    float charging_current_limit = mppt.update(pv_voltage_V, pv_current_mA);
    potentiometer.setCurrent(charging_current_limit);


    Serial.printf("PV Panel -> Current: %.3f mA, Voltage: %.3f V\n", pv_current_mA, pv_voltage_V);
    Serial.printf("Battery  -> Current: %.3f mA, Voltage: %.3f V\n", battery_current_mA, battery_voltage_V);
    Serial.printf("Adjusted Charging Current Limit: %.3f mA\n", charging_current_limit);

    char message[150];
    snprintf(message, sizeof(message), "%.3f,%.3f,%.3f,%.3f,%.3f\n", pv_current_mA, pv_voltage_V, charging_current_limit, battery_current_mA, battery_voltage_V);
    appendFile(SD_MMC, filename.c_str(), message);
}


String getNextFilename() {
  String baseName = "/data_";
  String extension = ".csv";
  int index = 0;
  String filename;

  do {
    filename = baseName + index + extension;
    index++;
  } while (SD_MMC.exists(filename));

  return filename;
}


void appendFile(fs::FS &fs, const char *path, const char *message) {
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (!file.print(message)) {
    Serial.println("Append failed");
  }
}
