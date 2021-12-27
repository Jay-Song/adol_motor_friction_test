#include "stdafx.h"
#include "arduino_current_reader.h"
#include <iostream>

ArduinoCurrentReader::ArduinoCurrentReader()
{
  serial_ = 0;

}

ArduinoCurrentReader::~ArduinoCurrentReader()
{
  if (serial_ != 0)
    delete serial_;
}

bool ArduinoCurrentReader::connect(std::string port_name, int baud_num)
{
  if (serial_ != 0)
    delete serial_;

  initializeTxPacket();
  serial_ = new jay::SerialCommWin(port_name);
  if (serial_->openPort(baud_num)) // 1 Mbps
    return true;
  else
  {
    delete serial_;
    return false;
  }
}

bool ArduinoCurrentReader::txRxPacket()
{
  if (serial_->writePort(tx_packet_, 7) != 7) //total_packet_length = 7)
    return false;

  serial_->setPacketTimeout(9+1); // +1 is just safety factor

  uint8_t rx_length = 0;
  uint8_t checksum = 0;
  uint8_t header_idx = 0;
  uint8_t error = 0;
   
   while (true)
   {
     rx_length += serial_->readPort(&rx_packet_[rx_length], 9 - rx_length);

     if (rx_length >= 9)
     {
       //find header
       for (header_idx = 0; header_idx < 8; header_idx++)
       {
         if ((rx_packet_[header_idx] == 0xFF) && (rx_packet_[header_idx + 1] == 0xFF))
           break;
       }

       if (header_idx != 0) //memmove : use for loop because it will be also used in arduino
       {
         for (uint8_t i = header_idx; i < rx_length; i++)
           rx_packet_[i - header_idx] = rx_packet_[i];
         rx_length -= header_idx;
       }

       if (rx_length >= 9)
       {
         checksum = 0;
         for (uint16_t idx = 2; idx < 9 - 1; idx++)   // except header, checksum
           checksum += rx_packet_[idx];
         checksum = ~checksum; // checksum

         if (checksum != rx_packet_[8])
         {
           error = 2;
           break;
         }
         else
         {
           error = 0;
           break;
         }
       }
     }

     if (serial_->isPacketTimeout())
     {
       error = 1;
       break;
     }
   }

   if (error == 0)
   {
     current_ = *(float*)&rx_packet_[4];
     return true;
   }
   else if (error == 1)
   {
     std::cout << "[arduino] time out" << std::endl;
     return false;
   }
   else if (error == 2)
   {
     std::cout << "[arduino] rx corrupted" << std::endl;
     return false;
   }
   else
   {
     return false;
   }
}

float ArduinoCurrentReader::getCurrent()
{
  return current_;
}

void ArduinoCurrentReader::close()
{
  if (serial_ != 0)
  {
    serial_->closePort();
    delete serial_;

    serial_ = 0;
  }
}

void ArduinoCurrentReader::initializeTxPacket()
{
  tx_packet_[0] = 0xFF;
  tx_packet_[1] = 0xFF;
  tx_packet_[2] = 0x00; // reserved ID
  tx_packet_[3] = 0x03; // length from instruction
  tx_packet_[4] = 0x02; // instruction : read
  tx_packet_[5] = 0x00; // parameter : 0 for read, 1~255 for write

                       // add a checksum to the packet
  uint8_t checksum = 0;
  for (uint16_t idx = 2; idx < 7 - 1; idx++)   // except header, checksum
    checksum += tx_packet_[idx];
  tx_packet_[6] = ~checksum; // checksum
}