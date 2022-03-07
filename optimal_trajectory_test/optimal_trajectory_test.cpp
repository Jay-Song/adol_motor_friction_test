#include <string>
#include <fstream>
#include <sstream>
#include "optimal_trajectory_test.h"

using namespace adol;


OptiamlTrajectoryResultDataMX28::OptiamlTrajectoryResultDataMX28()
{
  elapsed_time_sec_ = 0;
  voltage_output_v_ = 0;
  measured_weight_g_ = 0;
  arduino_curr_mx28_mA_ = 0;
}

OptiamlTrajectoryResultDataMX28::~OptiamlTrajectoryResultDataMX28()
{
  dxl_data_.clear();
}


OptimalTrajectoryTest::OptimalTrajectoryTest()
{
  dxl_ctrl_ = 0;
  arduino_ = 0;
  load_cell_ = 0;
  goal_data_idx_ = 0;

  print_flag_ = false;
  ctrl_flag_ = false;
}

OptimalTrajectoryTest::~OptimalTrajectoryTest()
{
  if (dxl_ctrl_ != 0)
    delete dxl_ctrl_;

  if (arduino_ != 0)
    delete arduino_;

  if (load_cell_ != 0)
    delete load_cell_;

  results_array_.clear();

  for (uint32_t i = 0; i < goal_value_list_.size(); i++)
    goal_value_list_[i].clear();

  goal_value_list_.clear();
}

bool OptimalTrajectoryTest::initialize(std::string dxl_port_name, int dxl_baud_rate, int dxl_ctrl_mode, std::vector<uint8_t> dxl_ID_list,
  int phidget_channel_num, std::string calibration_file_name,
  std::string arduino_port_name, int arduino_baud_rate)
{
  dxl_ID_list_.clear();
  dxl_ID_list_.resize(dxl_ID_list.size());

  std::copy(dxl_ID_list.begin(), dxl_ID_list.end(), dxl_ID_list_.begin());

  dxl_ctrl_ = new CtrlMX28(dxl_ID_list, dxl_ctrl_mode);
  arduino_ = new ArduinoCurrentReader();
  load_cell_ = new PhidgetLoadCell(phidget_channel_num);

  if (dxl_ctrl_->initializeCommDXL(dxl_port_name.c_str(), dxl_baud_rate) == false)
    return false;

  if (dxl_ctrl_->turnTorqueOnDXL(false) == false)
    return false;

  if (dxl_ctrl_->initializeDXLParam() == false)
    return false;

  if (dxl_ctrl_->initializeDXLIndirectAddr() == false)
    return false;

  if (dxl_ctrl_->turnTorqueOnDXL(true) == false)
    return false;



  load_cell_->ctx_ = this;
  load_cell_->getCalibFactor(calibration_file_name);
  load_cell_->onVoltageRatioChangeCallback = onVoltageRatioChange;

  QueryPerformanceCounter(&BeginTime);
  if (load_cell_->initializePhidget() == false)
    return false;

  if (arduino_->connect(arduino_port_name, arduino_baud_rate) == false)
    return false;

  arduino_->startTimer();
}

void OptimalTrajectoryTest::onVoltageRatioChange(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio)
{
  OptimalTrajectoryTest* test = (OptimalTrajectoryTest*)ctx;

  QueryPerformanceCounter(&(test->Endtime));
  uint64_t elapsed = test->Endtime.QuadPart - test->BeginTime.QuadPart;
  test->results_.elapsed_time_sec_ = (double)elapsed / (double)test->Frequency.QuadPart;

  test->results_.voltage_output_v_ = voltageRatio;
  test->load_cell_->measured_weight_g_ = voltageRatio*test->load_cell_->calib1_ + test->load_cell_->calib2_;
  test->results_.arduino_curr_mx28_mA_ = test->arduino_->getCurrent();
  
  if (test->ctrl_flag_ == true)
  {
    test->updatdGoalValues();
    test->dxl_ctrl_->changeGoalValues(test->results_.dxl_data_);
    test->dxl_ctrl_->dxl_sync_write_->txPacket();
  }

  test->dxl_ctrl_->dxl_sync_read_->txRxPacket();
  test->dxl_ctrl_->readValuesFromDXLsRx(test->results_.dxl_data_);

  if (test->print_flag_ == true)
  {
    printf("t: %lf vol*10^8: %lf f: %lf gp : %d cp: %d ct: %d curr: %f\n",
      test->results_.elapsed_time_sec_,
      voltageRatio*100000000.0,
      test->load_cell_->measured_weight_g_,
      test->results_.dxl_data_[0].goal_position_mx28_.i32_value,
      test->results_.dxl_data_[0].goal_position_mx28_.i32_value,
      test->results_.dxl_data_[0].present_temperature_mx28_,
      test->results_.arduino_curr_mx28_mA_);

    test->results_array_.push_back(test->results_);
  }
}


bool OptimalTrajectoryTest::loadOptimalTrajectory(std::string file_name)
{
  std::ifstream file;
  file.open(file_name);

  goal_value_list_.clear();
  
  std::vector<int32_t> single_values;

  std::string str;
  int32_t value;
  
  while (!file.eof())
  {
    single_values.clear();
    std::getline(file, str);
    std::stringstream ss(str);

    while (ss >> value)
      single_values.push_back(value);

    if (single_values.size() != dxl_ID_list_.size())
    {
      std::cout << "incorrect data size" << std::endl;
      return false;
    }
    else
    {
      goal_value_list_.push_back(single_values);
    }
  }
  file.close();

  goal_data_idx_ = 0;
}

void OptimalTrajectoryTest::updatdGoalValues()
{
  if (ctrl_flag_ == false)
    return;

  if (goal_data_idx_ >= goal_value_list_.size())
  {
    if(dxl_ctrl_->dxl_ctrl_mode_ != 4)
      goal_data_idx_ = 0;
    else 
      goal_data_idx_ = goal_value_list_.size() - 1;
  }

  //if (dxl_ctrl_->dxl_ctrl_mode_ == 0)
  //  goal_curr_xm430_.i16_value = goal_xm430_list_[data_idx_];

  
  if (dxl_ctrl_->dxl_ctrl_mode_ == 1)
  {
    for (uint32_t id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
      results_.dxl_data_[id_idx].goal_velocity_mx28_.i32_value = goal_value_list_[goal_data_idx_][id_idx];
  }
  else if (dxl_ctrl_->dxl_ctrl_mode_ == 4)
  {
    for (uint32_t id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
      results_.dxl_data_[id_idx].goal_position_mx28_.i32_value = goal_value_list_[goal_data_idx_][id_idx];
  }
  else if (dxl_ctrl_->dxl_ctrl_mode_ == 16)
  {
    for (uint32_t id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
      results_.dxl_data_[id_idx].goal_pwm_mx28_.i16_value = goal_value_list_[goal_data_idx_][id_idx];
  }
  else
    std::cout << "Invalid the Control Mode of the Driving Motor" << std::endl;

  goal_data_idx_++;
}
