#include <iostream>
#include "dxl_ctrl.h"

#define PROTOCOL_VERSION (2.0)

using namespace adol;


ResultDataMX28::ResultDataMX28()
{
  dxl_id_                       = 0;
  goal_position_mx28_with_pid_.i32_value = 0;
  goal_position_mx28_.i32_value = 0;
  goal_velocity_mx28_.i32_value = 0;
  goal_pwm_mx28_.i16_value      = 0;

  present_pwm_mx28_.i16_value      = 0;
  present_position_mx28_.i32_value = 0;
  present_velocity_mx28_.i32_value = 0;
  present_temperature_mx28_        = 0;
}

ResultDataMX28::~ResultDataMX28()
{ }


CtrlMX28::CtrlMX28(std::vector<uint8_t> dxl_ID_list, uint8_t dxl_ctrl_mode)
{
  dxl_port_       = 0;
  dxl_packet_     = 0;
  dxl_sync_write_ = 0;
  dxl_sync_read_  = 0;
  
  dxl_ctrl_mode_ = dxl_ctrl_mode;

  dxl_ID_list_.resize(dxl_ID_list.size());
  std::copy(dxl_ID_list.begin(), dxl_ID_list.end(), dxl_ID_list_.begin());
}

CtrlMX28::~CtrlMX28()
{
  dxl_ID_list_.clear();
}


bool CtrlMX28::initializeCommDXL(const char* port_name, int baud_rate)
{
  dxl_port_ = dynamixel::PortHandler::getPortHandler(port_name);
  dxl_packet_ = dynamixel::PacketHandler::getPacketHandler(2.0);

  if (dxl_port_->openPort())
  {
    printf("Succeeded in opening the port\n");
  }
  else
  {
    printf("Failed to open the port\n");
    return false;
  }

  if (dxl_port_->setBaudRate(baud_rate))
  {
    printf("Succeeded in changing the baudrate\n");
  }
  else
  {
    printf("Failed to change the baudrate\n");
    return false;
  }

  return true;
}

bool CtrlMX28::initializeDXLParam(void)
{
  if (initializeDXLIndirectAddr() == false)
    return false;

  int dxl_result;
  uint8_t dxl_error = 0;

  // change operating mode of test motor

  for (unsigned int id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
  {
    dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_ID_list_[id_idx], MX28::ADDR_OPERATING_MODE, dxl_ctrl_mode_, &dxl_error);
    if (dxl_result == COMM_SUCCESS)
    {
      std::cout << "Succeeded in changing the Operating Mode of ID " << (int)dxl_ID_list_[id_idx] <<  std::endl;
      Sleep(200);
    }
    else
    {
      std::cout << "Failed to change the Operating Mode of ID " << (int)dxl_ID_list_[id_idx] << std::endl;
      return false;
    }
  }

  // Initialize dxl_sync_read;
  dxl_sync_read_ = new dynamixel::GroupSyncRead(dxl_port_, dxl_packet_, MX28::ADDR_INDIRECT_DATA_1, 11);
  for (unsigned int id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
    dxl_sync_read_->addParam(dxl_ID_list_[id_idx]);

  // Initialize dxl_sync_write
  if (dxl_ctrl_mode_ == 1)
    dxl_sync_write_ = new dynamixel::GroupSyncWrite(dxl_port_, dxl_packet_, MX28::ADDR_GOAL_VELOCITY, 4);
  else if (dxl_ctrl_mode_ == 4)
    dxl_sync_write_ = new dynamixel::GroupSyncWrite(dxl_port_, dxl_packet_, MX28::ADDR_GOAL_POSITION, 4);
  else if (dxl_ctrl_mode_ == 16)
    dxl_sync_write_ = new dynamixel::GroupSyncWrite(dxl_port_, dxl_packet_, MX28::ADDR_GOAL_VELOCITY, 2);
  else
  {
    std::cout << "Failed to initialize bulk write : the invalid control mode of the test" << std::endl;
    return false;
  }

  uint8_t initial_data[4] = { 0, 0, 0, 0 };
  for (unsigned int id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
    dxl_sync_write_->addParam(dxl_ID_list_[id_idx], initial_data);

  return true;
}

bool CtrlMX28::initializeDXLIndirectAddr(void)
{
  uint8_t num_indirect_addr_set = 22;
  uint8_t indirect_addr_array[22];

  int dxl_result = 0;
  uint8_t dxl_error = 0;

  indirect_addr_array[0] = DXL_LOBYTE(MX28::ADDR_PRESENT_PWM + 0);
  indirect_addr_array[1] = DXL_HIBYTE(MX28::ADDR_PRESENT_PWM + 0);
  indirect_addr_array[2] = DXL_LOBYTE(MX28::ADDR_PRESENT_PWM + 1);
  indirect_addr_array[3] = DXL_HIBYTE(MX28::ADDR_PRESENT_PWM + 1);

  indirect_addr_array[4] = DXL_LOBYTE(MX28::ADDR_PRESENT_VELOCITY + 0);
  indirect_addr_array[5] = DXL_HIBYTE(MX28::ADDR_PRESENT_VELOCITY + 0);
  indirect_addr_array[6] = DXL_LOBYTE(MX28::ADDR_PRESENT_VELOCITY + 1);
  indirect_addr_array[7] = DXL_HIBYTE(MX28::ADDR_PRESENT_VELOCITY + 1);
  indirect_addr_array[8] = DXL_LOBYTE(MX28::ADDR_PRESENT_VELOCITY + 2);
  indirect_addr_array[9] = DXL_HIBYTE(MX28::ADDR_PRESENT_VELOCITY + 2);
  indirect_addr_array[10] = DXL_LOBYTE(MX28::ADDR_PRESENT_VELOCITY + 3);
  indirect_addr_array[11] = DXL_HIBYTE(MX28::ADDR_PRESENT_VELOCITY + 3);

  indirect_addr_array[12] = DXL_LOBYTE(MX28::ADDR_PRESENT_POSITION + 0);
  indirect_addr_array[13] = DXL_HIBYTE(MX28::ADDR_PRESENT_POSITION + 0);
  indirect_addr_array[14] = DXL_LOBYTE(MX28::ADDR_PRESENT_POSITION + 1);
  indirect_addr_array[15] = DXL_HIBYTE(MX28::ADDR_PRESENT_POSITION + 1);
  indirect_addr_array[16] = DXL_LOBYTE(MX28::ADDR_PRESENT_POSITION + 2);
  indirect_addr_array[17] = DXL_HIBYTE(MX28::ADDR_PRESENT_POSITION + 2);
  indirect_addr_array[18] = DXL_LOBYTE(MX28::ADDR_PRESENT_POSITION + 3);
  indirect_addr_array[19] = DXL_HIBYTE(MX28::ADDR_PRESENT_POSITION + 3);

  indirect_addr_array[20] = DXL_LOBYTE(MX28::ADDR_PRESENT_TEMPERATURE + 0);
  indirect_addr_array[21] = DXL_HIBYTE(MX28::ADDR_PRESENT_TEMPERATURE + 0);

  for (unsigned int id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
  {
    dxl_result = dxl_packet_->writeTxRx(dxl_port_, dxl_ID_list_[id_idx], MX28::ADDR_INDIRECT_ADDR_1, num_indirect_addr_set, indirect_addr_array, &dxl_error);
    if (dxl_result == COMM_SUCCESS)
    {
      std::cout << "Succeeded in changing Indirect Address of ID " << dxl_ID_list_[id_idx] << std::endl;
      Sleep(200);
    }
    else
    {
      std::cout << "Failed to change Indirect Address of ID " << dxl_ID_list_[id_idx] << std::endl;
      return false;
    }

    if (dxl_error != 0)
      std::cout << "[ID:" << (int) dxl_ID_list_[id_idx] << "] " << dxl_packet_->getRxPacketError(dxl_error) << std::endl;
  }

  return true;
}

bool CtrlMX28::turnTorqueOnDXL(bool on_off)
{
  uint8_t dxl_error = 0;
  int dxl_result = 0;
  for (unsigned int id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
  {
    if (on_off)
      dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_ID_list_[id_idx], MX28::ADDR_TORQUE_ENABLE, 1, &dxl_error);
    else
      dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_ID_list_[id_idx], MX28::ADDR_TORQUE_ENABLE, 0, &dxl_error);

    if (dxl_result == COMM_SUCCESS)
    {
      std::cout << "Succeeded in turning the torque state of ID " << (int)dxl_ID_list_[id_idx] << " to " << on_off << std::endl;
      Sleep(200);
    }
    else
    {
      std::cout << "Failed to turn the torque state of ID " << (int)dxl_ID_list_[id_idx] << " to " << on_off << std::endl;
      return false;
    }
  }

  return true;
}

void CtrlMX28::changeGoalValues(std::vector<ResultDataMX28>& dxl_data)
{
  int dxl_result = 0;

  for (unsigned int id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
  {
    //if (driving_dxl_ctrl_mode_ == 0)
    //  dxl_bulk_write_->changeParam(dxl_id_driving_, XM430::ADDR_GOAL_CURRENT, 2, goal_curr_xm430_.bytes);
    if (dxl_ctrl_mode_ == 1)
      dxl_sync_write_->changeParam(dxl_ID_list_[id_idx], dxl_data[id_idx].goal_velocity_mx28_.bytes);
    else if (dxl_ctrl_mode_ == 4)
      dxl_sync_write_->changeParam(dxl_ID_list_[id_idx], dxl_data[id_idx].goal_position_mx28_with_pid_.bytes);
    else if (dxl_ctrl_mode_ == 16)
      dxl_sync_write_->changeParam(dxl_ID_list_[id_idx], dxl_data[id_idx].goal_pwm_mx28_.bytes);
    else
      std::cout << "Invalid the Control Mode of the Driving Motor" << std::endl;
  }
}

bool CtrlMX28::readValuesFromDXLsTx(void)
{
  int dxl_result = dxl_sync_read_->txPacket();
  if (dxl_result != COMM_SUCCESS)
  {
    std::cout << dxl_result <<"  Failed to send the tx packet for sync read" << std::endl;
    return false;
  }
  return true;
}

bool CtrlMX28::readValuesFromDXLsRx(std::vector<ResultDataMX28>& dxl_data)
{
  int dxl_result = dxl_sync_read_->rxPacket();
  if (dxl_result != COMM_SUCCESS)
  {
    std::cout << dxl_result << "  Failed to receive the rx packet for sync read" << std::endl;
    return false;
  }

  for (unsigned int id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
  {
    dxl_data[id_idx].present_pwm_mx28_.i16_value      = dxl_sync_read_->getData(dxl_ID_list_[id_idx], MX28::ADDR_INDIRECT_DATA_1+0, 2);
    dxl_data[id_idx].present_velocity_mx28_.i32_value = dxl_sync_read_->getData(dxl_ID_list_[id_idx], MX28::ADDR_INDIRECT_DATA_1+2, 4);
    dxl_data[id_idx].present_position_mx28_.i32_value = dxl_sync_read_->getData(dxl_ID_list_[id_idx], MX28::ADDR_INDIRECT_DATA_1+6, 4);
    dxl_data[id_idx].present_temperature_mx28_        = dxl_sync_read_->getData(dxl_ID_list_[id_idx], MX28::ADDR_INDIRECT_DATA_1+10, 1);
  }

  return true;
}

bool CtrlMX28::readValuesFromDXLsTxRx(std::vector<ResultDataMX28>& dxl_data)
{
  if (readValuesFromDXLsTx() == false)
    return false;

  if (readValuesFromDXLsRx(dxl_data) == false)
    return false;

  return true;
}
