#include <SD.h>
#include <SPI.h>
#include <Adafruit_INA219.h>
#include "Potentiometer.h"
#include "ChargeController.h"
#include "BoostConverter.h"
#include "BatteryVoltage.h"

#define PIN_SG90 15
#define BATTERY_VOLTAGE_PIN A3

Adafruit_INA219 ina219_pv(0x40);
Potentiometer potentiometer(PIN_SG90);
BoostConverter boost_converter;
ChargeController charge_controller;
BatteryVoltage batteryVoltage(BATTERY_VOLTAGE_PIN);

String filename;

void setup() {
    Serial.begin(115200);

    if (!ina219_pv.begin()) {
        Serial.println("Failed to initialize INA219 sensor");
        while (true) { delay(10); }
    }
    Serial.println("INA219 sensor initialized");

    potentiometer.begin();
    boost_converter.begin();
    batteryVoltage.begin();

    pinMode(LED_BUILTIN, OUTPUT);

    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println("Failed to initialize SD card");
        return;
    }
    Serial.println("SD card initialized");

    filename = getNextFilename();
    File file = SD.open(filename.c_str(), FILE_WRITE);
    if (file) {
        file.println("time_s,pv_current_mA,pv_voltage_V,charging_current_limit_mA,battery_voltage_V,pv_power_W");
        file.close();
        Serial.println("CSV headers written");
    } else {
        Serial.println("Failed to open file for writing headers");
    }
}

void loop() {
    static unsigned long lastFastUpdate = 0, lastSlowUpdate = 0;
    static unsigned long blinkStart = 0;
    static float pv_current_mA = 0.0f, pv_voltage_V = 0.0f, battery_voltage_V = 0.0f;

    unsigned long now = millis();

    if (now - lastFastUpdate >= 100) {
        lastFastUpdate = now;

        digitalWrite(LED_BUILTIN, HIGH);
        blinkStart = now;

        pv_current_mA = ina219_pv.getCurrent_mA();
        pv_voltage_V = ina219_pv.getBusVoltage_V() + (ina219_pv.getShuntVoltage_mV() / 1000.0f);
        battery_voltage_V = batteryVoltage.readVoltage();

        boost_converter.update(pv_current_mA);
    }

    if (digitalRead(LED_BUILTIN) == HIGH && (now - blinkStart >= 100)) {
        digitalWrite(LED_BUILTIN, LOW);
    }

    if (now - lastSlowUpdate >= 500) {
        lastSlowUpdate = now;

        bool stable = boost_converter.isStable();
        bool batteryConnected = batteryVoltage.isBatteryConnected();
        float charging_current_limit = charge_controller.update(pv_voltage_V, pv_current_mA, stable, batteryConnected);
        potentiometer.setCurrent(charging_current_limit);

        float time_s = (float)now / 1000.0f;
        float pv_power_W = (pv_current_mA / 1000.0f) * pv_voltage_V;

        Serial.printf("Current: %.2f mA | Stable: %s | Battery Connected: %s\n", pv_current_mA, stable ? "Yes" : "No", batteryConnected ? "Yes" : "No");
        Serial.printf("[%.3f s] PV Panel -> Current: %.3f mA, Voltage: %.3f V, Power: %.3f W\n", time_s, pv_current_mA, pv_voltage_V, pv_power_W);
        Serial.printf("[%.3f s] Battery -> Voltage: %.3f V\n", time_s, battery_voltage_V);
        Serial.printf("[%.3f s] Adjusted Charging Current Limit: %.3f mA\n", time_s, charging_current_limit);
        Serial.println("-------------");

        char message[200];
        snprintf(message, sizeof(message), "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n", time_s, pv_current_mA, pv_voltage_V, charging_current_limit, battery_voltage_V, pv_power_W);
        appendFile(filename.c_str(), message);
    }
}

String getNextFilename() {
    String baseName = "data_";
    String extension = ".csv";
    int index = 0;
    String filename;

    do {
        filename = baseName + index + extension;
        index++;
    } while (SD.exists(filename.c_str()));

    return filename;
}

void appendFile(const char *path, const char *message) {
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (!file.print(message)) {
        Serial.println("Append failed");
    }
    file.close();
}
