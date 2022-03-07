#pragma once

#include <windows.h>
#include <iostream>
#include <mmsystem.h> // for multimedia timer
#include "serial_comm_win.h"


class ArduinoCurrentReader
{
public:
  ArduinoCurrentReader();
  ~ArduinoCurrentReader();

  bool connect(std::string port_name, int baud_num);
  
  bool txPacket();
  bool rxPacket();

  bool txRxPacket();
  
  float getCurrent();

  void close();

  void startTimer();
  void stopTimer();

  static void CALLBACK procArduinoCurrentCallback(UINT m_nTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

private:
  void initializeTxPacket();

  jay::SerialCommWin *serial_;

  UINT time_id_; // for timer

  uint8_t tx_packet_[7]; // total_packet_length = 7
  uint8_t rx_packet_[30]; 

  float current_;
};