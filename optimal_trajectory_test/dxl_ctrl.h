#pragma once
#include "dynamixel_sdk\dynamixel_sdk.h"
#include "control_table_XM430.h"
#include "control_table_MX28.h"
#include <Windows.h>
#include <vector>

union value16
{
  int16_t i16_value;
  uint16_t ui16_value;
  uint8_t bytes[2];
};

union value32
{
  int32_t i32_value;
  uint32_t ui32_value;
  uint8_t bytes[4];
  float f32_value;
};

namespace adol
{
  class ResultDataMX28
  {
  public:
    ResultDataMX28();
    ~ResultDataMX28();

    uint8_t dxl_id_;
    value32 goal_position_mx28_with_pid_;
    value32 goal_position_mx28_;
    value32 goal_velocity_mx28_;
    value16 goal_pwm_mx28_;

    value16 present_pwm_mx28_;
    value32 present_velocity_mx28_;
    value32 present_position_mx28_;
    uint8_t present_temperature_mx28_;
    value16 present_input_voltage_;
  };


  class CtrlMX28
  {

  public:
    uint8_t dxl_ctrl_mode_;
    std::vector<uint8_t> dxl_ID_list_;

    CtrlMX28(std::vector<uint8_t> dxl_ID_list, uint8_t dxl_ctrl_mode);
    ~CtrlMX28();

    //for dxl
    dynamixel::PortHandler*   dxl_port_;
    dynamixel::PacketHandler* dxl_packet_;
    dynamixel::GroupSyncWrite* dxl_sync_write_;
    dynamixel::GroupSyncRead*  dxl_sync_read_;

    bool initializeCommDXL(const char* port_name, int baud_rate);

    bool initializeDXLParam(void);
    bool initializeDXLIndirectAddr(void);
    bool turnTorqueOnDXL(bool on_off);

    void changeGoalValues(std::vector<ResultDataMX28>& dxl_data);

    bool readValuesFromDXLsTx(void);
    bool readValuesFromDXLsRx(std::vector<ResultDataMX28>& dxl_data);
    bool readValuesFromDXLsTxRx(std::vector<ResultDataMX28>& dxl_data);
  };
}