#include <string>
#include <fstream>
#include <sstream>
#include <time.h>
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

  clearResult();
}

bool OptimalTrajectoryTest::initialize(std::string dxl_port_name, int dxl_baud_rate, int dxl_ctrl_mode, std::vector<uint8_t> dxl_ID_list,
  int phidget_channel_num, std::string calibration_file_name,
  std::string arduino_port_name, int arduino_baud_rate, 
  double p_gain, double i_gain, double d_gain)
{
  dxl_ID_list_.clear();
  dxl_ID_list_.resize(dxl_ID_list.size());

  std::copy(dxl_ID_list.begin(), dxl_ID_list.end(), dxl_ID_list_.begin());

  results_.dxl_data_.resize(dxl_ID_list.size());
  pid_.resize(dxl_ID_list.size());

  for (uint32_t id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
  {
    pid_[id_idx].initialize(0.008, p_gain, i_gain, d_gain);
  }

  dxl_ctrl_ = new CtrlMX28(dxl_ID_list, dxl_ctrl_mode);
  arduino_ = new ArduinoCurrentReader();
  load_cell_ = new PhidgetLoadCell(phidget_channel_num);

  if (dxl_ctrl_->initializeCommDXL(dxl_port_name.c_str(), dxl_baud_rate) == false)
    return false;

  if (dxl_ctrl_->turnTorqueOnDXL(false) == false)
    return false;

  if (dxl_ctrl_->initializeDXLParam() == false)
    return false;

  if (dxl_ctrl_->turnTorqueOnDXL(true) == false)
    return false;

  load_cell_->ctx_ = this;
  load_cell_->getCalibFactor(calibration_file_name);
  load_cell_->onVoltageRatioChangeCallback = onVoltageRatioChange;

  if (arduino_->connect(arduino_port_name, arduino_baud_rate) == false)
    return false;

  arduino_->txRxPacket();
  Sleep(1);
  arduino_->txRxPacket();
  Sleep(1);


  arduino_->startTimer();

  dxl_ctrl_->readValuesFromDXLsTxRx(results_.dxl_data_);
  dxl_ctrl_->readValuesFromDXLsTxRx(results_.dxl_data_);
  dxl_ctrl_->readValuesFromDXLsTx();
  Sleep(1);


  QueryPerformanceFrequency(&cpu_frequency_);
  QueryPerformanceCounter(&ctrl_begin_time_);
  if (load_cell_->initializePhidget() == false)
    return false;



}

void OptimalTrajectoryTest::onVoltageRatioChange(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio)
{
  OptimalTrajectoryTest* test = (OptimalTrajectoryTest*)ctx;

  QueryPerformanceCounter(&(test->ctrl_end_time_));
  uint64_t elapsed = test->ctrl_end_time_.QuadPart - test->ctrl_begin_time_.QuadPart;
  test->results_.elapsed_time_sec_ = (double) elapsed / (double) test->cpu_frequency_.QuadPart;

  test->results_.voltage_output_v_ = voltageRatio;
  test->load_cell_->measured_weight_g_ = voltageRatio*test->load_cell_->calib1_ + test->load_cell_->calib2_;
  test->results_.measured_weight_g_ = test->load_cell_->measured_weight_g_;
  test->results_.arduino_curr_mx28_mA_ = test->arduino_->getCurrent();

  test->dxl_ctrl_->readValuesFromDXLsRx(test->results_.dxl_data_);

  if (test->ctrl_flag_ == true)
  {
    test->updatdGoalValues();
    test->dxl_ctrl_->changeGoalValues(test->results_.dxl_data_);
    test->dxl_ctrl_->dxl_sync_write_->txPacket();
  }

  if (test->print_flag_ == true)
  {
    printf("t: %lf vol*10^8: %lf f: %lf gp : %d cp: %d ct: %d cc: %f cv: %d\n",
      test->results_.elapsed_time_sec_,
      voltageRatio*100000000.0,
      test->load_cell_->measured_weight_g_,
      test->results_.dxl_data_[0].goal_position_mx28_.i32_value,
      test->results_.dxl_data_[0].present_position_mx28_.i32_value,
      test->results_.dxl_data_[0].present_temperature_mx28_,
      test->results_.arduino_curr_mx28_mA_,
      test->results_.dxl_data_[0].present_input_voltage_.i16_value);

    test->results_array_.push_back(test->results_);
  }
  test->dxl_ctrl_->readValuesFromDXLsTx();
}

bool OptimalTrajectoryTest::loadOptimalTrajectory(std::string file_name)
{
  std::ifstream file;
  file.open(file_name);

  for (unsigned int i = 0; i < goal_value_list_.size(); i++)
    goal_value_list_[i].clear();

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
    {
      //std::cout << value << std::endl;
      single_values.push_back(value);
    }

    if (single_values.size() != dxl_ID_list_.size())
    {
      //std::cout << "incorrect data size" << single_values.size()  << "   " << dxl_ID_list_.size() << std::endl;
      file.close();

      goal_data_idx_ = 0;
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

void OptimalTrajectoryTest::startCtrl()
{
  ctrl_flag_ = true;
  print_flag_ = true;
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
    {
      goal_data_idx_ = goal_value_list_.size() - 1;
      ctrl_flag_ = false;
      print_flag_ = false;
    }
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
    {
      results_.dxl_data_[id_idx].goal_position_mx28_.i32_value = goal_value_list_[goal_data_idx_][id_idx];
      results_.dxl_data_[id_idx].goal_position_mx28_with_pid_.i32_value =
        goal_value_list_[goal_data_idx_][id_idx] + 
        pid_[id_idx].getOutput(goal_value_list_[goal_data_idx_][id_idx],
        results_.dxl_data_[id_idx].present_position_mx28_.i32_value);
    }
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

const std::string getCurrentDateTime() {
  time_t     now = time(0); //save current time into time_t
  struct tm  tstruct;
  localtime_s(&tstruct, &now);

  std::stringstream ss;
  ss << tstruct.tm_year + 1900;

  ss.width(2);
  ss.fill('0');
  ss << (tstruct.tm_mon + 1) // the month is 0 to 11, so we need to add 1.
    << tstruct.tm_mday
    << tstruct.tm_hour
    << tstruct.tm_min
    << tstruct.tm_sec << ".txt";

  return ss.str();
}

void OptimalTrajectoryTest::clearResult()
{
  for (uint32_t i = 0; i < results_array_.size(); i++)
    results_array_[i].dxl_data_.clear();
  
  results_array_.clear();

  for (uint32_t i = 0; i < goal_value_list_.size(); i++)
    goal_value_list_[i].clear();

  goal_value_list_.clear();
}

void OptimalTrajectoryTest::saveTestResult()
{
  ctrl_flag_ = false;
  print_flag_ = false;
  Sleep(8);

  std::ofstream log_file;
  std::string file_path = "log_val_" + getCurrentDateTime();

  log_file.open(file_path);
  log_file << std::fixed;
  log_file.precision(10);
  log_file << "time" << "\t" << "voltage_output" << "\t" << "measured_load" << "\t";
  
  for (unsigned int id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
  {
    log_file << "des_pos_ID_" << (int)dxl_ID_list_[id_idx] << "\t" << "des_pid_pos_ID_" << (int)dxl_ID_list_[id_idx] << "\t"
      << "des_vel_ID_" << (int)dxl_ID_list_[id_idx] << "\t" << "des_pwm_ID_" << (int)dxl_ID_list_[id_idx] << "\t"
      << "mes_pos_ID_" << (int)dxl_ID_list_[id_idx] << "\t" << "mes_vel_ID_" << (int)dxl_ID_list_[id_idx] << "\t"
      << "mes_pwm_ID_" << (int)dxl_ID_list_[id_idx] << "\t" << "mes_temp_ID_" << (int)dxl_ID_list_[id_idx] << "\t"
      << "mes_input_vol_ID_" << (int)dxl_ID_list_[id_idx] << "\t";
  }

  log_file << "mes_curr" << std::endl;

  for (unsigned int arr_idx = 0; arr_idx < results_array_.size(); arr_idx++)
  {
    log_file << results_array_[arr_idx].elapsed_time_sec_ << "\t"
      << results_array_[arr_idx].voltage_output_v_ << "\t"
      << results_array_[arr_idx].measured_weight_g_ << "\t";

    for (unsigned int id_idx = 0; id_idx < dxl_ID_list_.size(); id_idx++)
    {
      log_file << results_array_[arr_idx].dxl_data_[id_idx].goal_position_mx28_.i32_value << "\t"
        << results_array_[arr_idx].dxl_data_[id_idx].goal_position_mx28_with_pid_.i32_value << "\t"
        << results_array_[arr_idx].dxl_data_[id_idx].goal_velocity_mx28_.i32_value << "\t"
        << (int)results_array_[arr_idx].dxl_data_[id_idx].goal_pwm_mx28_.i16_value << "\t"
        << results_array_[arr_idx].dxl_data_[id_idx].present_position_mx28_.i32_value << "\t"
        << results_array_[arr_idx].dxl_data_[id_idx].present_velocity_mx28_.i32_value << "\t"
        << results_array_[arr_idx].dxl_data_[id_idx].present_pwm_mx28_.i16_value << "\t"
        << (unsigned int)results_array_[arr_idx].dxl_data_[id_idx].present_temperature_mx28_ << "\t"
        << (int)results_array_[arr_idx].dxl_data_[id_idx].present_input_voltage_.i16_value << "\t";
    }

    log_file << results_array_[arr_idx].arduino_curr_mx28_mA_ << std::endl;
  }

  log_file.close();

  clearResult();
}