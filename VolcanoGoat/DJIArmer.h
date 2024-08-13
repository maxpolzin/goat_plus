// DJIArmer.h

#ifndef DJI_ARMER_H
#define DJI_ARMER_H

#include <HardwareSerial.h>
#include "MSP.h"
#include "MSP_OSD.h"

class DJIArmer {
  int8_t rx_pin;
  int8_t tx_pin;
  HardwareSerial &mspSerial;
  MSP msp;

  uint32_t unarmedMillis = 3000;
  uint32_t previousMillis_MSP = 0;
  uint32_t activityDetectedMillis_MSP = 0;
  bool activityDetected = false;
  const uint32_t next_interval_MSP = 100;
  uint32_t flightModeFlags = 0x00000002;
  bool isArmed = false;

  char fcVariant[5] = "BTFL";
  char craftname[15] = "goat";

  msp_name_t name = { 0 };
  msp_fc_version_t fc_version = { 0 };
  msp_fc_variant_t fc_variant = { 0 };
  msp_status_DJI_t status_DJI = { 0 };

public:
  DJIArmer(HardwareSerial &serial, int8_t rx_pin, int8_t tx_pin);
  void begin();
  void update();

private:
  void setArmFlightMode();
  void sendMSPToAirUnit();
  void debugPrint();
};

#endif
