// SBUSReader.cpp

#include "SBUSReader.h"

SBUSReader::SBUSReader(HardwareSerial &serial, int8_t rx_pin, int8_t tx_pin)
  : sbus(&serial, rx_pin, tx_pin, true) {}

void SBUSReader::begin() {
  sbus.Begin();

  Serial.println("SBUSReader initialized.");

}

void SBUSReader::update() {
  // Read SBUS data
  if (sbus.Read()) {
    processChannels();
  }
}

void SBUSReader::processChannels() {
  bfs::SbusData data = sbus.data();


  // Output channel data to the serial monitor
  for (int i = 0; i < bfs::SbusData::NUM_CH; i++) {
    Serial.print("Channel ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(data.ch[i]);
  }

  // Check failsafe and frame lost status
  if (data.failsafe) {
    Serial.println("Failsafe active!");
  }
  if (data.lost_frame) {
    Serial.println("Frame lost!");
  }
}
