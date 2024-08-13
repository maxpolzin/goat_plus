// SBUSReader.cpp

#include "SBUSReader.h"

SBUSReader::SBUSReader(HardwareSerial &serial, int8_t rx_pin, int8_t tx_pin)
  : sbus(&serial, rx_pin, tx_pin, true), isConnected(false), forwardVelocityCommand(0), steeringVelocityCommand(0), winchVelocityCommand(0), cameraPositionCommand(0) {}

void SBUSReader::begin() {
  sbus.Begin();

  Serial.println("SBUSReader initialized.");

}

void SBUSReader::update() {
  if (sbus.Read()) {
    isConnected = true;
    processChannels();
  }
}

double SBUSReader::normalize(int raw){
  static const int MAX_VALUE=1811;
  static const int MIN_VALUE=172;

  int mapped = map(raw, MIN_VALUE, MAX_VALUE, -1000, 1000);
  return mapped/1000.0;
}

void SBUSReader::processChannels() {
  static unsigned long lastPrintTime = 0;
  unsigned long currentTime = millis();

  bfs::SbusData data = sbus.data();

  if (data.failsafe || data.lost_frame) {

    if (currentTime - lastPrintTime >= 1000) {
      Serial.println("SBUSReader: Failsafe active or frame lost!");
      lastPrintTime = currentTime;
    }    

    isConnected = false;

    steeringVelocityCommand = 0.0;
    forwardVelocityCommand = 0.0;
    winchVelocityCommand = 0.0;
    
    return;
  }

  steeringVelocityCommand = normalize(data.ch[0]);
  forwardVelocityCommand = normalize(data.ch[1]);
  if (abs(forwardVelocityCommand) < 0.3) {
      forwardVelocityCommand = 0.0;
  }
  cameraPositionCommand = normalize(data.ch[3]);
  winchVelocityCommand = normalize(data.ch[4]);

}
