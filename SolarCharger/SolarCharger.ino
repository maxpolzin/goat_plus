#include <Adafruit_INA219.h>
#include "Potentiometer.h"
#include "MPPT.h"

#define I2C_SDA 14
#define I2C_SCL 15

#define PIN_SG90 16

TwoWire twoWire = TwoWire(0);
Adafruit_INA219 ina219;

Potentiometer potentiometer(PIN_SG90);
MPPT mppt;


void setup() {
    Serial.begin(115200);

    twoWire.setPins(I2C_SDA, I2C_SCL);
    if (!ina219.begin(&twoWire)) {
        Serial.println("Failed to find INA219 chip");
        while (1) { delay(10); }
    }
    Serial.println("Measuring voltage and current with INA219 ...");

    potentiometer.setup(); // Setup potentiometer (SG90 servo)
}

void loop() {
    delay(2000);

    float current_mA = ina219.getCurrent_mA();
    float loadvoltage = ina219.getBusVoltage_V() + (ina219.getShuntVoltage_mV() / 1000);

    Serial.printf("Current: %.3f mA, Voltage: %.3f V\n", current_mA, loadvoltage);

    float current_limit = mppt.update(loadvoltage, current_mA);
    potentiometer.setCurrent(current_limit);

    Serial.printf("Adjusted Current Limit: %.3f mA\n", current_limit);
}
