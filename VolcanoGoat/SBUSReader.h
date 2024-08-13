// SBUSReader.h

#ifndef SBUS_READER_H
#define SBUS_READER_H

#include <HardwareSerial.h>
#include "sbus.h"

class SBUSReader {
  bfs::SbusRx sbus;

public:
  SBUSReader(HardwareSerial &serial, int8_t rx_pin, int8_t tx_pin);
  void begin();
  void update();

  double forwardVelocityCommand;
  double steeringVelocityCommand;
  double winchVelocityCommand;
  double cameraPositionCommand;
  bool isConnected;

private:
  void processChannels();
  double normalize(int raw);

};

#endif
