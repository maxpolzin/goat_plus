// DJIArmer.cpp

#include "DJIArmer.h"

DJIArmer::DJIArmer(HardwareSerial &serial, int8_t rx_pin, int8_t tx_pin)
  : mspSerial(serial), rx_pin(rx_pin), tx_pin(tx_pin) {}

void DJIArmer::begin() {
  mspSerial.begin(115200, SERIAL_8N1, rx_pin, tx_pin);
  while (!mspSerial){}
  msp.begin(mspSerial);

  delay(1000);

  status_DJI.cycleTime = 0x0080;
  status_DJI.i2cErrorCounter = 0;
  status_DJI.sensor = 0x23;
  status_DJI.configProfileIndex = 0;
  status_DJI.averageSystemLoadPercent = 7;
  status_DJI.accCalibrationAxisFlags = 0;
  status_DJI.DJI_ARMING_DISABLE_FLAGS_COUNT = 20;
  status_DJI.djiPackArmingDisabledFlags = (1 << 24);
  flightModeFlags = 0x00000002;

  Serial.println("DJIArmer: Waiting for DJI O3 Air Unit...");
}

void DJIArmer::update() {

  if (!activityDetected) {
    if(!msp.activityDetected()){
      return;
    }

    activityDetected = true;
    activityDetectedMillis_MSP = millis();

    Serial.printf("DJIArmer: DJI O3 Air Unit detected. Waiting %lu ms before arming...\n", unarmedMillis);
  }

  uint32_t currentMillis_MSP = millis();

  if ((uint32_t)(currentMillis_MSP - previousMillis_MSP) >= next_interval_MSP) {
    previousMillis_MSP = currentMillis_MSP;

    if ((activityDetectedMillis_MSP + unarmedMillis) <= currentMillis_MSP && !isArmed) {
      setArmFlightMode();
      isArmed = true;
    }

    sendMSPToAirUnit();
  }
}

void DJIArmer::setArmFlightMode() {
  Serial.println("DJIArmer: Arming.");
  flightModeFlags = 0x00000003;  // arm
}

void DJIArmer::sendMSPToAirUnit() {
  // MSP_FC_VARIANT
  memcpy(fc_variant.flightControlIdentifier, fcVariant, sizeof(fcVariant));
  msp.send(MSP_FC_VARIANT, &fc_variant, sizeof(fc_variant));

  // MSP_FC_VERSION
  fc_version.versionMajor = 4;
  fc_version.versionMinor = 5;
  fc_version.versionPatchLevel = 1;
  msp.send(MSP_FC_VERSION, &fc_version, sizeof(fc_version));

  // MSP_NAME
  memcpy(name.craft_name, craftname, sizeof(craftname));
  msp.send(MSP_NAME, &name, sizeof(name));

  // MSP_STATUS
  status_DJI.flightModeFlags = flightModeFlags;
  status_DJI.armingFlags = 0x0303;
  msp.send(MSP_STATUS_EX, &status_DJI, sizeof(status_DJI));
  status_DJI.armingFlags = 0x0000;
  msp.send(MSP_STATUS, &status_DJI, sizeof(status_DJI));
}

//*** USED ONLY FOR DEBUG ***
void DJIArmer::debugPrint() {
  Serial.printf("DJIArmer: Flight Mode: (%lu)\n",flightModeFlags);
}
