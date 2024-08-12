// DJIArmer.cpp

#include "DJIArmer.h"

DJIArmer::DJIArmer(HardwareSerial &serial, int8_t rx_pin, int8_t tx_pin)
  : mspSerial(serial), rx_pin(rx_pin), tx_pin(tx_pin) {}

void DJIArmer::begin() {
  mspSerial.begin(115200, SERIAL_8N1, rx_pin, tx_pin);
  while (!mspSerial);
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

  #ifdef DEBUG
    Serial.println("Initialized");
  #endif
}

void DJIArmer::update() {
  if (!activityDetected) {
    #ifdef DEBUG
      Serial.println("Waiting for AU...");
    #endif

    // Wait for Air Unit to send data
    while (!msp.activityDetected());
    activityDetected = true;
    activityDetectedMillis_MSP = millis();

    #ifdef DEBUG
      Serial.println("AU Detected, waiting (unarmedMillis) time till arm");
    #endif
  }

  uint32_t currentMillis_MSP = millis();

  if ((uint32_t)(currentMillis_MSP - previousMillis_MSP) >= next_interval_MSP) {
    previousMillis_MSP = currentMillis_MSP;

    if (currentMillis_MSP < (activityDetectedMillis_MSP + unarmedMillis)) {
      setFlightModeFlags(false);
    } else {
      setFlightModeFlags(true);
    }

    #ifdef DEBUG
      debugPrint();
    #endif

    sendMSPToAirUnit();
  }
}

void DJIArmer::setFlightModeFlags(bool arm) {
  if ((flightModeFlags == 0x00000002) && arm) {
    flightModeFlags = 0x00000003;    // arm
    #ifdef DEBUG
      Serial.println("ARMING");
    #endif
  } else if ((flightModeFlags == 0x00000003) && !arm) {
    flightModeFlags = 0x00000002;    // disarm 
    #ifdef DEBUG
      Serial.println("DISARMING");
    #endif
  }
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
  Serial.println("**********************************");
  Serial.print("Flight Mode: ");
  Serial.println(flightModeFlags);
}
