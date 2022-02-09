#pragma once
#include "dynamixel_sdk\dynamixel_sdk.h"
#include "control_table_XM430.h"
#include "control_table_MX28.h"
#include <Windows.h>


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


struct ResultData
{
  double  elapsed_time_sec_;
  int32_t goal_velocity_xm430_;
  int16_t goal_pwm_xm430_;
  int16_t goal_curr_xm430_;

  uint8_t present_temperature_xm430_;
  int16_t present_current_xm430_;
  int32_t present_velocity_xm430_;
  int32_t present_position_xm430_;

  int32_t goal_velocity_mx28_;
  int16_t goal_pwm_mx28_;

  uint8_t present_temperature_mx28_;
  int32_t present_velocity_mx28_;
  int32_t present_position_mx28_;

  double  voltage_output_v_;
  double  measured_weight_g_;
  float   arduino_curr_mx28_mA_;
};

namespace adol
{
  class DynamixelTestCtrl
  {
    DynamixelTestCtrl();
    ~DynamixelTestCtrl();

    //for dxl
    dynamixel::PortHandler*   dxl_port_;
    dynamixel::PacketHandler* dxl_packet_;
    dynamixel::GroupBulkWrite* dxl_bulk_write_;
    dynamixel::GroupBulkRead* dxl_bulk_read_;
    bool initializeCommDXL(void);
    bool initializeDXLParam(void);
    bool initializeDXLIndirectAddr(void);
    bool turnTorqueOnDXL(bool on_off);

    void updateGoalValues(void);
    void changeGoalValues(void);

    bool readValuesFromDXLsTx(void);
    bool readValuesFromDXLsRx(void);
    bool readValuesFromDXLsTxRx(void);
    
    uint8_t driving_dxl_ctrl_mode_;
    uint8_t test_dxl_ctrl_mode_;

  };
}