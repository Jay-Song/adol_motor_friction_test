#include <Wire.h>
#include <Adafruit_INA219.h>
#include <MsTimer2.h>

Adafruit_INA219 ina219;

union
{
  /* data */
  float valuef;
  unsigned char bytes[4];
} current_mA;

unsigned char checksum = 0;
unsigned char error = 0;
int rx_length = 0;
uint8_t rx_packet[30];
uint8_t tx_packet[30] = { 0 };
uint8_t idx = 0;
bool sensing_flag = false;

void measureCurrent()
{
  sensing_flag = true;
  current_mA.valuef = ina219.getCurrent_mA();
  sensing_flag = false;
}


void setup()
{
  // put your setup code here, to run once:
  tx_packet[0] = 0xFF;
  tx_packet[1] = 0xFF;
  tx_packet[2] = 0x00; // reserved ID
  tx_packet[3] = 0x05; // length from instruction
  tx_packet[4] = 0x00; // parameter 1
  tx_packet[5] = 0x00; // parameter 2
  tx_packet[6] = 0x00; // parameter 3
  tx_packet[7] = 0x00; // parameter 4
  tx_packet[8] = 0x00; // check sum

  current_mA.valuef = 0;

  Serial.begin(1000000);
  while (!Serial)
  {
    delay(1);
  }

    if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }

  MsTimer2::set(1, measureCurrent);
  MsTimer2::start();  
}

void loop()
{
  if (Serial.available() > 0)
  {
    rx_packet[rx_length] = Serial.read();
    rx_length++;
  }

  if (rx_length >= 7)
  {
    //find header
    for (idx = 0; idx < 6; idx++)
    {
      if ((rx_packet[idx] == 0xFF) && (rx_packet[idx + 1] == 0xFF))
        break;
    }

    if (idx != 0) //memmove : use for loop because it will be also used in arduino
    {
      for (uint8_t i = idx; i < rx_length; i++)
        rx_packet[i - idx] = rx_packet[idx];
      rx_length -= idx;
    }
  }

  if (rx_length >= 7)
  {
    checksum = 0;
    for (idx = 2; idx < 7 - 1; idx++) // except header, checksum
      checksum += rx_packet[idx];
    checksum = ~checksum; // checksum

    if (checksum != rx_packet[6])
    {
      error = 2;
      rx_length = 0;
    }
    else
    {
      error = 0;
    }
  }

  if ((rx_length >= 7) && (error == 0))
  {
    while(sensing_flag);
    tx_packet[4] = current_mA.bytes[0];
    tx_packet[5] = current_mA.bytes[1];
    tx_packet[6] = current_mA.bytes[2];
    tx_packet[7] = current_mA.bytes[3];

    checksum = 0;
    for (idx = 2; idx < 9 - 1; idx++) // except header, checksum
      checksum += tx_packet[idx];
    checksum = ~checksum; // checksum

    tx_packet[8] = checksum;

    Serial.write(tx_packet, 9);
    rx_length = 0;
    error = 0;
  }

}
