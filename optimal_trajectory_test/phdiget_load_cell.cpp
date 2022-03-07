#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include "phidget_load_cell.h"

PhidgetLoadCell::PhidgetLoadCell(int channel_num)
{
  channel_num_ = channel_num;
  calib1_ = calib2_ = 0;
  voltage_ratio_Input_ = 0;

  onVoltageRatioChangeCallback = 0;
  
  voltage_output_v_ = 0;
  measured_weight_g_ = 0;

  ctx_ = 0;
}

PhidgetLoadCell::~PhidgetLoadCell()
{ }



bool PhidgetLoadCell::initializePhidget(void)
{
  PhidgetReturnCode phidget_return;
  PhidgetReturnCode error_code;
  char error_detail[100];
  size_t error_detail_length;
  const char* error_msg;

  PhidgetLog_enable(PHIDGET_LOG_INFO, "phidgetlog.log");
  PhidgetVoltageRatioInput_create(&voltage_ratio_Input_);
  PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltage_ratio_Input_, onVoltageRatioChangeCallback, ctx_);
  Phidget_setOnAttachHandler((PhidgetHandle)voltage_ratio_Input_, attachVoltageRatioInput, NULL);
  Phidget_setOnDetachHandler((PhidgetHandle)voltage_ratio_Input_, detachVoltageRatioInput, NULL);
  Phidget_setOnErrorHandler((PhidgetHandle)voltage_ratio_Input_, onVoltageRatioInputError, NULL);
  Phidget_setChannel((PhidgetHandle)voltage_ratio_Input_, channel_num_);

  // Open your Phidgets and wait for attachment
  phidget_return = Phidget_openWaitForAttachment((PhidgetHandle)voltage_ratio_Input_, 5000);
  if (phidget_return != EPHIDGET_OK) {
    Phidget_getLastError(&error_code, &error_msg, error_detail, &error_detail_length);
    printf("Error (%d): %s", error_code, error_msg);
    return false;
  }

  //Set the Data Interval of the Device to 8 ms
  phidget_return = Phidget_setDataInterval((PhidgetHandle)voltage_ratio_Input_, 8);
  if (phidget_return != EPHIDGET_OK) {
    Phidget_getLastError(&error_code, &error_msg, error_detail, &error_detail_length);
    printf("Error (%d): %s", error_code, error_msg);
    return false;
  }

  return true;
}

bool PhidgetLoadCell::terminatePhidget(void)
{
  PhidgetReturnCode ret;
  PhidgetReturnCode error_code;

  char error_detail[100];
  size_t error_detail_length;
  const char* error_msg;

  ret = Phidget_close((PhidgetHandle)voltage_ratio_Input_);
  if (ret != EPHIDGET_OK) {
    Phidget_getLastError(&error_code, &error_msg, error_detail, &error_detail_length);
    printf("Error (%d): %s", error_code, error_msg);
    //exit(1);
  }
  PhidgetVoltageRatioInput_delete(&voltage_ratio_Input_);

  return true;
}


void PhidgetLoadCell::attachVoltageRatioInput(PhidgetHandle ch, void * ctx)
{
  printf("Attach!\n");
}

void PhidgetLoadCell::detachVoltageRatioInput(PhidgetHandle ch, void * ctx)
{
  printf("Detach!\n");
}

void PhidgetLoadCell::onVoltageRatioInputError(PhidgetHandle ch, void * ctx, Phidget_ErrorEventCode code, const char * description)
{
  printf("Description: %s\n", description);
  printf("----------\n");
}

bool PhidgetLoadCell::getCalibFactor(std::string file_name)
{
  std::ifstream calib_file_handler;
  calib_file_handler.open(file_name);

  if (!calib_file_handler.is_open())
  {
    calib_file_handler.close();
    std::cout << "Failed to open the calibration file" << std::endl;
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
    calib1_ = calib_factor;

    calib_file_handler >> a;
    calib_file_handler >> calib_factor;
    std::cout << "calib_factor2: " << calib_factor << std::endl;
    calib2_ = calib_factor;

    calib_file_handler.close();

    return true;
  }
}