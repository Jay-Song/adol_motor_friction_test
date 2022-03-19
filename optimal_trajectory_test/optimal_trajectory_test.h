#pragma once

#include "dxl_ctrl.h"
#include "arduino_current_reader.h"
#include "phidget_load_cell.h"
#include "pid_control.h"

namespace adol
{
  class OptiamlTrajectoryResultDataMX28
  {
  public:
    OptiamlTrajectoryResultDataMX28();
    ~OptiamlTrajectoryResultDataMX28();

    double  elapsed_time_sec_;

    std::vector<ResultDataMX28> dxl_data_;

    double  voltage_output_v_;
    double  measured_weight_g_;
    float   arduino_curr_mx28_mA_;
  };

  class OptimalTrajectoryTest
  {
  public:
    OptimalTrajectoryTest();
    ~OptimalTrajectoryTest();

    OptiamlTrajectoryResultDataMX28 results_;

    int32_t goal_data_idx_;
    std::vector<std::vector<int32_t> > goal_value_list_;

    std::vector<OptiamlTrajectoryResultDataMX28> results_array_;

    CtrlMX28* dxl_ctrl_;
    ArduinoCurrentReader* arduino_;
    PhidgetLoadCell* load_cell_;

    LARGE_INTEGER cpu_frequency_;
    LARGE_INTEGER ctrl_begin_time_;
    LARGE_INTEGER ctrl_end_time_;

    std::vector<uint8_t> dxl_ID_list_;
    std::vector<PIDControl> pid_;

    bool print_flag_;
    bool ctrl_flag_;

    void startCtrl();

    bool initialize(std::string dxl_port_name, int dxl_baud_rate, int dxl_ctrl_mode, std::vector<uint8_t> dxl_ID_list,
      int phidget_channel_num, std::string calibration_file_name, 
      std::string arduino_port_name, int arduino_baud_rate,
      double p_gain, double i_gain, double d_gain);

    bool loadOptimalTrajectory(std::string file_name);
    void updatdGoalValues();

    void clearResult();
    void saveTestResult();
    
    static void CALLBACK onVoltageRatioChange(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio);

  };
}