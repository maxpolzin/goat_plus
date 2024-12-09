#include <SD.h>
#include <SPI.h>
#include <Adafruit_INA219.h>
#include "Potentiometer.h"
#include "ChargeController.h"

// Pin definitions
#define PIN_SG90 15
#define BATTERY_VOLTAGE_PIN A3

// Voltage divider resistors (in ohms)
#define R1 220000.0 // Top resistor (220kΩ)
#define R2 47000.0  // Bottom resistor (47kΩ)

// Voltage divider scaling factor
#define VOLTAGE_SCALING_FACTOR ((R1 + R2) / R2) // ~5.681

// ADC reference voltage
#define ADC_REF_VOLTAGE 3.3
#define ADC_RESOLUTION 1023.0 // Default 10-bit ADC resolution for Teensy

Adafruit_INA219 ina219_pv(0x40); // Sensor for PV panel
Potentiometer potentiometer(PIN_SG90);
ChargeController charge_controller(12.6); 

String filename;

void setup() {
    Serial.begin(115200);

    // // Initialize INA219 for PV panel
    if (!ina219_pv.begin()) {
        Serial.println("Failed to initialize INA219 sensor");
        while (1) { delay(10); }
    }
    Serial.println("INA219 sensor initialized");

    potentiometer.setup();

    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);


    // Initialize the SD card
    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println("Failed to initialize SD card");
        return;
    }
    Serial.println("SD card initialized");

    filename = getNextFilename();

    // Write CSV headers
    File file = SD.open(filename.c_str(), FILE_WRITE);
    if (file) {
        file.println("timestamp_ms,pv_current,pv_voltage,charging_current_limit,battery_voltage");
        file.close();
        Serial.println("CSV headers written");
    } else {
        Serial.println("Failed to open file for writing headers");
    }

}

void loop() {

    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)

    unsigned long timestamp = millis();

    float pv_current_mA = ina219_pv.getCurrent_mA();
    float pv_voltage_V = ina219_pv.getBusVoltage_V() + (ina219_pv.getShuntVoltage_mV() / 1000);

    float battery_voltage_adc = analogRead(BATTERY_VOLTAGE_PIN);
    float battery_voltage_raw = (battery_voltage_adc / ADC_RESOLUTION) * ADC_REF_VOLTAGE;
    float battery_voltage_V = battery_voltage_raw * VOLTAGE_SCALING_FACTOR;


    float charging_current_limit = charge_controller.update(battery_voltage_V, pv_voltage_V, pv_current_mA);
    potentiometer.setCurrent(charging_current_limit);


    Serial.printf("[%lu ms] PV Panel -> Current: %.3f mA, Voltage: %.3f V\n", timestamp, pv_current_mA, pv_voltage_V);
    Serial.printf("[%lu ms] Battery  -> Voltage: %.3f V\n", timestamp, battery_voltage_V);
    Serial.printf("[%lu ms] Adjusted Charging Current Limit: %.3f mA\n", timestamp, charging_current_limit);

    char message[200];
    snprintf(message, sizeof(message), "%lu,%.3f,%.3f,%.3f,%.3f\n", timestamp, pv_current_mA, pv_voltage_V, charging_current_limit, battery_voltage_V);
    appendFile(filename.c_str(), message);

    delay(100);                      
    digitalWrite(LED_BUILTIN, LOW);
    delay(900);     

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
