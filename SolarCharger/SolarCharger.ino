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


void setup() {
    Serial.begin(115200);

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


    potentiometer.setup()
}

void loop() {
    delay(2000);

    float pv_current_mA = ina219_pv.getCurrent_mA();
    float pv_voltage_V = ina219_pv.getBusVoltage_V() + (ina219_pv.getShuntVoltage_mV() / 1000);

    float battery_current_mA = ina219_battery.getCurrent_mA();
    float battery_voltage_V = ina219_battery.getBusVoltage_V() + (ina219_battery.getShuntVoltage_mV() / 1000);

    Serial.printf("PV Panel -> Current: %.3f mA, Voltage: %.3f V\n", pv_current_mA, pv_voltage_V);

    Serial.printf("Battery  -> Current: %.3f mA, Voltage: %.3f V\n", battery_current_mA, battery_voltage_V);


    float charging_current_limit = mppt.update(pv_voltage_V, pv_current_mA);
    potentiometer.setCurrent(charging_current_limit);

    Serial.printf("Adjusted Charging Current Limit: %.3f mA\n", charging_current_limit);
}
