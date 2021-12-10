#pragma once

#include "serial_comm_win.h"

class ArduinoCurrentReader
{
public:
  ArduinoCurrentReader();
  ~ArduinoCurrentReader();

  bool connect(std::string port_name, int baud_num);
  bool txRxPacket();
  
  float getCurrent();

  void close();

private:
  void initializeTxPacket();


  jay::SerialCommWin *serial_;

  uint8_t tx_packet_[7]; // total_packet_length = 7
  uint8_t rx_packet_[30]; 

  float current_;
};