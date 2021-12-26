#include <Windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <time.h>

#include <phidget22.h>
#include "dynamixel_sdk\dynamixel_sdk.h"
#include "control_table_XM430.h"
#include "control_table_MX28.h"



int comport_num_ = 0;
int baud_num_ = 0;

dynamixel::PortHandler *dxl_port_;
dynamixel::PacketHandler *dxl_packet_;
dynamixel::GroupBulkRead* dxl_bulk_read_;

uint8_t dxl_id_test_ = 1;
uint8_t dxl_id_brake_ = 2;

int32_t goal_position_xm430_;

uint8_t present_temperature_xm430_;
int16_t present_current_xm430_;
int32_t present_velocity_xm430_;
int32_t present_position_xm430_;

uint8_t present_temperature_MX28_;
int32_t present_velocity_MX28_;
int32_t present_position_MX28_;

bool initializeCommDXL(void);
bool initializeDXLParam(void);
bool turnDXLTorqueOnandOff(bool on_off);
bool readValuesFromDXLs(void);
bool changeGoalPosition(void);

int channel_num_ = 0;
PhidgetVoltageRatioInputHandle voltage_ratio_Input;


const char* error_msg;
char error_detail[100];
size_t error_detail_length;
bool initializePhidget(void);

void __stdcall onVoltageRatioChange(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio);
void __stdcall attachVoltageRatioInput(PhidgetHandle ch, void * ctx);
void __stdcall detachVoltageRatioInput(PhidgetHandle ch, void * ctx);
void __stdcall onVoltageRatioInputError(PhidgetHandle ch, void * ctx, Phidget_ErrorEventCode code, const char * description);

void waitForMeasuringEnd();
void calculateVoltageAvgStddev(double *avg, double *stddev);

int position_interval_ = 10;
int nrotation_ = 3;



int max_voltage_measure_count = 250;
double voltage_output[250];
int voltage_measure_count = max_voltage_measure_count;
double calib1 = 0, calib2 = 0;

bool getCalibFactor(std::string file_name);
void saveLog(void);


struct measured_data
{
  int xm430_position_;
  int mx28_position_;
  double load_cell_voltage_;
  double load_cell_stddev_;
  double load_cell_g_;
  int position_interval_;
};

std::vector<measured_data> measured_data_arr;


int main(void)
{
  // Initialize DXL Comm Para
  std::cout << "COM Port Num: ";
  std::cin >> comport_num_;

  std::cout << "Baud Rate: ";
  std::cin >> baud_num_;

  std::stringstream ss;
  ss << "COM" << comport_num_;

  dxl_port_ = dynamixel::PortHandler::getPortHandler(ss.str().c_str());
  dxl_packet_ = dynamixel::PacketHandler::getPacketHandler(2.0);

  //Initialize Phidget Para
  std::cout << "Phidget Channel Num: ";
  std::cin >> channel_num_;

  PhidgetLog_enable(PHIDGET_LOG_INFO, "phidgetlog.log");
  PhidgetVoltageRatioInput_create(&voltage_ratio_Input);
  PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltage_ratio_Input, onVoltageRatioChange, 0);
  Phidget_setOnAttachHandler((PhidgetHandle)voltage_ratio_Input, attachVoltageRatioInput, NULL);
  Phidget_setOnDetachHandler((PhidgetHandle)voltage_ratio_Input, detachVoltageRatioInput, NULL);
  Phidget_setOnErrorHandler((PhidgetHandle)voltage_ratio_Input, onVoltageRatioInputError, NULL);
  Phidget_setChannel((PhidgetHandle)voltage_ratio_Input, channel_num_);


  // Initialize Test Para
  std::cout << "Position Interval: ";
  std::cin >> position_interval_;

  std::cout << "Num of Rotation: ";
  std::cin >> nrotation_;


  // Connect to DXLs and set parameters
  int dxl_comm_result = COMM_TX_FAIL;
  uint8_t dxl_error = 0;

  if (initializeCommDXL() == false)
    return 1;

  if (turnDXLTorqueOnandOff(false) == false)
    return 1;

  if (initializeDXLParam() == false)
    return 1;

  if (turnDXLTorqueOnandOff(true) == false)
    return 1;

  //Connect to Phidget
  if (initializePhidget() == false)
    return 1;

  if (getCalibFactor("calib.txt") == false)
    return 1;

  // Initialize test para
  std::cout << "read current DXL values." << std::endl;
  while (readValuesFromDXLs() == false)
    Sleep(5);

  goal_position_xm430_ = present_position_xm430_;

  int rotation_count = 0;

  measured_data_arr.clear();
  measured_data _data;
  while (true)
  {
    while (changeGoalPosition() == false)
      Sleep(5);

    Sleep(1000);

    while (readValuesFromDXLs() == false)
      Sleep(5);

    voltage_measure_count = 0;
    waitForMeasuringEnd();
    double vol_avg, vol_stddev;
    calculateVoltageAvgStddev(&vol_avg, &vol_stddev);

    _data.xm430_position_ = present_position_xm430_;
    _data.mx28_position_ = present_position_MX28_;
    _data.load_cell_voltage_ = vol_avg;
    _data.load_cell_stddev_ = vol_stddev;
    _data.load_cell_g_ = vol_avg*calib1 + calib2;
    _data.position_interval_ = position_interval_;

    measured_data_arr.push_back(_data);

    std::cout << "MX28pos: " << present_position_MX28_ << " vol_avg: " << vol_avg << " vol_stddev: " << vol_stddev << " position_interval_: " << position_interval_ << std::endl;

    //if (goal_position_xm430_ > 4095)
    //{
    //  goal_position_xm430_ = 4095;
    //  position_interval_ = position_interval_*(-1);

    //  rotation_count += 1;
    //}

    //if (goal_position_xm430_ < 0)
    //{
    //  goal_position_xm430_ = 0;
    //  position_interval_ = position_interval_*(-1);
    //}

    //if (present_position_MX28_ < 0)
    //{
    //  position_interval_ = position_interval_*(-1);
    //  goal_position_xm430_ += present_position_MX28_;
    //}

    //if (present_position_MX28_ > 4095)
    //{
    //  position_interval_ = position_interval_*(-1);
    //  goal_position_xm430_ += (present_position_MX28_ - 4095);

    //  rotation_count += 1;
    //}

    goal_position_xm430_ += position_interval_;

    rotation_count = abs( (int)(present_position_MX28_ / 4095));

    if (rotation_count > nrotation_)
    {
      std::cout << "rotation_count: " << rotation_count << " nrot: " << nrotation_ << std::endl;
      break;
    }
  }

  saveLog();

  return 0;
}


bool initializeCommDXL(void)
{
  int dxl_comm_result = COMM_TX_FAIL;
  uint8_t dxl_error = 0;

  if (dxl_port_->openPort())
  {
    printf("Succeeded in opening the port\n");
  }
  else
  {
    printf("Failed to open the port\n");
    return false;
  }


  if (dxl_port_->setBaudRate(baud_num_))
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

bool initializeDXLParam(void)
{
  //indirect address of brake motor
  uint8_t num_indirect_addr_set = 22;
  uint8_t indirect_addr_array[22];
  indirect_addr_array[0] = DXL_LOBYTE(XM430::ADDR_PRESENT_CURRENT + 0);
  indirect_addr_array[1] = DXL_HIBYTE(XM430::ADDR_PRESENT_CURRENT + 0);
  indirect_addr_array[2] = DXL_LOBYTE(XM430::ADDR_PRESENT_CURRENT + 1);
  indirect_addr_array[3] = DXL_HIBYTE(XM430::ADDR_PRESENT_CURRENT + 1);

  indirect_addr_array[4] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 0);
  indirect_addr_array[5] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 0);
  indirect_addr_array[6] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 1);
  indirect_addr_array[7] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 1);
  indirect_addr_array[8] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 2);
  indirect_addr_array[9] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 2);
  indirect_addr_array[10] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 3);
  indirect_addr_array[11] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 3);

  indirect_addr_array[12] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 0);
  indirect_addr_array[13] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 0);
  indirect_addr_array[14] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 1);
  indirect_addr_array[15] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 1);
  indirect_addr_array[16] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 2);
  indirect_addr_array[17] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 2);
  indirect_addr_array[18] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 3);
  indirect_addr_array[19] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 3);

  indirect_addr_array[20] = DXL_LOBYTE(XM430::ADDR_PRESENT_TEMPERATURE + 0);
  indirect_addr_array[21] = DXL_HIBYTE(XM430::ADDR_PRESENT_TEMPERATURE + 0);

  uint8_t dxl_error = 0;
  int dxl_result = dxl_packet_->writeTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_INDIRECT_ADDR_1, num_indirect_addr_set, indirect_addr_array, &dxl_error);
  if (dxl_result == COMM_SUCCESS)
  {
    printf("Succeeded in changing Indirect Address\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to change Indirect Address\n");
    return false;
  }

  if (dxl_error != 0)
    printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));

  //indirec address for MX28
  num_indirect_addr_set = 18;
  indirect_addr_array[0] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 0);
  indirect_addr_array[1] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 0);
  indirect_addr_array[2] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 1);
  indirect_addr_array[3] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 1);
  indirect_addr_array[4] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 2);
  indirect_addr_array[5] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 2);
  indirect_addr_array[6] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 3);
  indirect_addr_array[7] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 3);

  indirect_addr_array[8] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 0);
  indirect_addr_array[9] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 0);
  indirect_addr_array[10] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 1);
  indirect_addr_array[11] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 1);
  indirect_addr_array[12] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 2);
  indirect_addr_array[13] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 2);
  indirect_addr_array[14] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 3);
  indirect_addr_array[15] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 3);

  indirect_addr_array[16] = DXL_LOBYTE(XM430::ADDR_PRESENT_TEMPERATURE + 0);
  indirect_addr_array[17] = DXL_HIBYTE(XM430::ADDR_PRESENT_TEMPERATURE + 0);

  dxl_error = 0;
  dxl_result = dxl_packet_->writeTxRx(dxl_port_, dxl_id_test_, MX28::ADDR_INDIRECT_ADDR_1, num_indirect_addr_set, indirect_addr_array, &dxl_error);
  if (dxl_result == COMM_SUCCESS)
  {
    printf("Succeeded in changing Indirect Address of MX28\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to change Indirect Address of MX28\n");
    return false;
  }

  if (dxl_error != 0)
    printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));


  // change operating mode of brake motor
  dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_OPERATING_MODE, 4, &dxl_error);
  if (dxl_result == COMM_SUCCESS)
  {
    printf("Succeeded in changing Operating Mode\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to change Operating Mode\n");
    return false;
  }

  //Initialize dxl_bulk_read;
  dxl_bulk_read_ = new dynamixel::GroupBulkRead(dxl_port_, dxl_packet_);
  dxl_bulk_read_->addParam(dxl_id_test_, XM430::ADDR_INDIRECT_DATA_1, 9);
  dxl_bulk_read_->addParam(dxl_id_brake_, MX28::ADDR_INDIRECT_DATA_1, 11);

  return true;
}

bool turnDXLTorqueOnandOff(bool on_off)
{
  uint8_t dxl_error = 0;
  int dxl_result = 0;
  if (on_off)
    dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_TORQUE_ENABLE, 1, &dxl_error);
  else
    dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_TORQUE_ENABLE, 0, &dxl_error);

  if (dxl_error == COMM_SUCCESS)
  {
    printf("Succeeded in turning torque on or off\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to turn torque on or off\n");
    return false;
  }

  if (dxl_error != 0)
    printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));

  return true;
}

bool readValuesFromDXLs(void)
{
  int dxl_result = dxl_bulk_read_->txRxPacket();
  if (dxl_result != COMM_SUCCESS)
  {
    printf("%s\n", dxl_packet_->getTxRxResult(dxl_result));
    return false;
  }

  present_current_xm430_ = dxl_bulk_read_->getData(dxl_id_brake_, XM430::ADDR_INDIRECT_DATA_1 + 0, 2);
  present_velocity_xm430_ = dxl_bulk_read_->getData(dxl_id_brake_, XM430::ADDR_INDIRECT_DATA_1 + 2, 4);
  present_position_xm430_ = dxl_bulk_read_->getData(dxl_id_brake_, XM430::ADDR_INDIRECT_DATA_1 + 6, 4);
  present_temperature_xm430_ = dxl_bulk_read_->getData(dxl_id_brake_, XM430::ADDR_INDIRECT_DATA_1 + 10, 1);

  present_velocity_MX28_ = dxl_bulk_read_->getData(dxl_id_test_, MX28::ADDR_INDIRECT_DATA_1 + 0, 4);
  present_position_MX28_ = dxl_bulk_read_->getData(dxl_id_test_, MX28::ADDR_INDIRECT_DATA_1 + 4, 4);
  present_temperature_MX28_ = dxl_bulk_read_->getData(dxl_id_test_, MX28::ADDR_INDIRECT_DATA_1 + 8, 1);


  return true;
}

bool initializePhidget(void)
{
  PhidgetReturnCode phidget_return;
  PhidgetReturnCode error_code;

  // Open your Phidgets and wait for attachment
  phidget_return = Phidget_openWaitForAttachment((PhidgetHandle)voltage_ratio_Input, 5000);
  if (phidget_return != EPHIDGET_OK) {
    Phidget_getLastError(&error_code, &error_msg, error_detail, &error_detail_length);
    printf("Error (%d): %s", error_code, error_msg);
    return false;
  }


  //Set the Data Interval of the Device to 8 ms
  phidget_return = Phidget_setDataInterval((PhidgetHandle)voltage_ratio_Input, 8);
  if (phidget_return != EPHIDGET_OK) {
    Phidget_getLastError(&error_code, &error_msg, error_detail, &error_detail_length);
    printf("Error (%d): %s", error_code, error_msg);
    return false;
  }

  return true;
}

void __stdcall attachVoltageRatioInput(PhidgetHandle ch, void * ctx)
{
  printf("Attach!\n");
}

void __stdcall detachVoltageRatioInput(PhidgetHandle ch, void * ctx)
{
  printf("Detach!\n");
}

void __stdcall onVoltageRatioInputError(PhidgetHandle ch, void * ctx, Phidget_ErrorEventCode code, const char * description)
{
  printf("Description: %s\n", description);
  printf("----------\n");
}

void __stdcall onVoltageRatioChange(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio)
{
  if (voltage_measure_count < max_voltage_measure_count)
  {
    voltage_output[voltage_measure_count++] = voltageRatio;
  }
  else
    return;
}

void waitForMeasuringEnd()
{
  while (true)
  {
    if (voltage_measure_count == max_voltage_measure_count)
      break;

    Sleep(10);
  }
}

bool changeGoalPosition(void)
{
  //if ((goal_position_xm430_ >= -200) || (goal_position_xm430_ <= 200))
  //{
  uint8_t dxl_error = 0;
  int dxl_result = dxl_packet_->write4ByteTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_GOAL_POSITION, goal_position_xm430_, &dxl_error);
  if (dxl_result != COMM_SUCCESS)
  {
    //printf("%s\n", dxl_packet_->getTxRxResult(dxl_result));
    return false;
  }

  //if (dxl_error != 0)
  //  printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));
  //}

  return true;
}

void calculateVoltageAvgStddev(double *avg, double *stddev)
{
  double voltage_sum = 0, voltage_avg = 0, voltage_stddev = 0;
  for (int voltage_idx = 0; voltage_idx < max_voltage_measure_count; voltage_idx++)
  {
    voltage_sum += voltage_output[voltage_idx];
  }

  voltage_avg = voltage_sum / (double)max_voltage_measure_count;

  voltage_sum = 0;
  for (int voltage_idx = 0; voltage_idx < max_voltage_measure_count; voltage_idx++)
  {
    voltage_sum += (voltage_output[voltage_idx] - voltage_avg) * (voltage_output[voltage_idx] - voltage_avg);
  }
  voltage_stddev = sqrt(voltage_sum / (double)max_voltage_measure_count);

  *avg = voltage_avg;
  *stddev = voltage_stddev;
}

const std::string currentDateTime() {
  time_t     now = time(0); //save current time into time_t
  struct tm  tstruct;
  //  char       buf[80];
  tstruct = *localtime(&now);
  //  strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct); // YYYY-MM-DD.HH:mm:ss ?????? ¨ö¨¬¨¡¢ç¢¬?

  std::stringstream ss;
  ss << tstruct.tm_year + 1900 << tstruct.tm_mon
    << tstruct.tm_mday
    << tstruct.tm_hour
    << tstruct.tm_min
    << tstruct.tm_sec << ".txt";

  return ss.str();
}

void saveLog(void)
{
  std::ofstream log_file;
  log_file.open(currentDateTime());
  log_file << std::fixed;
  log_file.precision(10);

  for (unsigned int arr_idx = 0; arr_idx < measured_data_arr.size(); arr_idx++)
  {
    log_file << measured_data_arr[arr_idx].load_cell_stddev_ << "\t"
      << measured_data_arr[arr_idx].load_cell_voltage_ << "\t"
      << measured_data_arr[arr_idx].load_cell_g_ << "\t"
      << measured_data_arr[arr_idx].position_interval_ << "\t"
      << 0 << "\t"
      << 0 << "\t"
      << 0 << "\t"
      << 0 << "\t"
      << measured_data_arr[arr_idx].position_interval_ << "\t"
      << measured_data_arr[arr_idx].xm430_position_ << "\t"
      << 0 << "\t"
      << 0 << "\t"
      << measured_data_arr[arr_idx].mx28_position_ << "\t" << std::endl;
  }

  log_file.close();
}

bool getCalibFactor(std::string file_name)
{
  std::ifstream calib_file_handler;
  calib_file_handler.open(file_name);

  if (!calib_file_handler.is_open())
  {
    calib_file_handler.close();
    return false;
  }
  else
  {
    std::cout << "succeeded in opening the calibration file" << std::endl;

    std::string a;
    double calib_factor = 0;
    calib_file_handler >> a;
    calib_file_handler >> calib_factor;
    std::cout << "calib_factor1: " << calib_factor << std::endl;
    calib1 = calib_factor;

    calib_file_handler >> a;
    calib_file_handler >> calib_factor;
    std::cout << "calib_factor2: " << calib_factor << std::endl;
    calib2 = calib_factor;

    calib_file_handler.close();

    return true;
  }

}