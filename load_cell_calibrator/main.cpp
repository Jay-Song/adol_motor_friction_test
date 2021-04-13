#include <phidget22.h>
#include <iostream>
#include <fstream>

PhidgetVoltageRatioInputHandle voltage_ratio_Input;
PhidgetReturnCode phidget_return;
PhidgetReturnCode error_code;

const char* error_msg;
char error_detail[100];
size_t error_detail_length;

void __stdcall onVoltageRatioChange(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio);
void __stdcall attachVoltageRatioInput(PhidgetHandle ch, void * ctx);
void __stdcall detachVoltageRatioInput(PhidgetHandle ch, void * ctx);
void __stdcall onVoltageRatioInputError(PhidgetHandle ch, void * ctx, Phidget_ErrorEventCode code, const char * description);

int max_voltage_measure_count = 625;
double voltage_output[625];
int voltage_measure_count = 625;

void waitForMeasuringEnd();
void calculateVoltageAvgStddev(double *avg, double *stddev);

double lever_arm_on_mass = 209.0;

int main(void)
{
  // Initialize and Set Phidget parameters
  int channel = 0;
  std::cout << "Channel Num: ";
  std::cin >> channel;
  std::cout << "Channel No." << channel << " will be used." << std::endl;

  PhidgetLog_enable(PHIDGET_LOG_INFO, "phidgetlog.log");
  PhidgetVoltageRatioInput_create(&voltage_ratio_Input);
  PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltage_ratio_Input, onVoltageRatioChange, 0);
  Phidget_setOnAttachHandler((PhidgetHandle)voltage_ratio_Input, attachVoltageRatioInput, NULL);
  Phidget_setOnDetachHandler((PhidgetHandle)voltage_ratio_Input, detachVoltageRatioInput, NULL);
  Phidget_setOnErrorHandler((PhidgetHandle)voltage_ratio_Input, onVoltageRatioInputError, NULL);
  Phidget_setChannel((PhidgetHandle)voltage_ratio_Input, channel);

  // Open your Phidgets and wait for attachment
  phidget_return = Phidget_openWaitForAttachment((PhidgetHandle)voltage_ratio_Input, 5000);
  if (phidget_return != EPHIDGET_OK) {
    Phidget_getLastError(&error_code, &error_msg, error_detail, &error_detail_length);
    printf("Error (%d): %s", error_code, error_msg);
    return 1;
  }

  
  //Set the Data Interval of the Device to 8 ms
  phidget_return = Phidget_setDataInterval((PhidgetHandle)voltage_ratio_Input, 8);
  if (phidget_return != EPHIDGET_OK) {
    Phidget_getLastError(&error_code, &error_msg, error_detail, &error_detail_length);
    printf("Error (%d): %s", error_code, error_msg);
    return 1;
  }

  std::cout << "1: Measuring the load when lever arm is off." << std::endl;
  std::cout << "Make sure that the lever arm is off (not pressing the load cell)." << std::endl;
  std::cout << "Press Enter key to start to measure." << std::endl; 
  std::cin.clear();
  std::cin.ignore(INT_MAX, '\n');
  std::cin.ignore(INT_MAX, '\n');
  //std::cin.get();
  voltage_measure_count = 0;

  waitForMeasuringEnd();

  double voltage_avg1 = 0, voltage_stddev1 = 0;
  calculateVoltageAvgStddev(&voltage_avg1, &voltage_stddev1);
    
  std::cout << "2: Measuring the load when lever arm is on." << std::endl;
  std::cout << "Make sure that the lever arm is on (pressing the load cell)." << std::endl;
  std::cout << "Press Enter key to start to measure." << std::endl;
  std::cin.clear();
  std::cin.ignore(INT_MAX, '\n');
  //std::cin.get();
  voltage_measure_count = 0;

  waitForMeasuringEnd();

  double voltage_avg2 = 0, voltage_stddev2 = 0;
  calculateVoltageAvgStddev(&voltage_avg2, &voltage_stddev2);

  std::cout << "3: Check the values." << std::endl;
  std::cout << "Lever arm off" << std::endl;
  std::cout << "average: " << voltage_avg1 << " " << "stddev: " << voltage_stddev1 << std::endl;

  std::cout << "Lever arm on" << std::endl;
  std::cout << "average: " << voltage_avg2 << " " << "stddev: " << voltage_stddev2 << std::endl;

  // error check
  if (voltage_avg1 > voltage_avg2)
  {
    std::cout << "[ERROR] Off value is higher than On value." << std::endl;
    std::cout << "Press Enter key to finish." << std::endl;
    std::cin.clear();
    std::cin.ignore(INT_MAX, '\n');
    std::cin.get();
    return 1;
  }

  double calibration_factor1 = 0, calibration_factor2 = 0;
  // mass = calibration_factor1 * voltage + calibration_factor2;
  calibration_factor1 = lever_arm_on_mass / (voltage_avg2 - voltage_avg1);
  calibration_factor2 = -calibration_factor1*voltage_avg1;

  std::cout << "4: Save the values." << std::endl;
  std::ofstream calib_file_handler;
  calib_file_handler.open("calib.txt");
  calib_file_handler << "calibration_factor1: " << calibration_factor1 << std::endl;
  calib_file_handler << "calibration_factor2: " << calibration_factor2 << std::endl;
  calib_file_handler.close();
  
  std::cout << "Completed saving the file." << std::endl;
  std::cout << "Press Enter key to finish." << std::endl;
  std::cin.clear();
  std::cin.ignore(INT_MAX, '\n');
  //std::cin.get();

  return 0;
}


//For Phidget
void __stdcall onVoltageRatioChange(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio)
{
  if (voltage_measure_count < max_voltage_measure_count)
  {
    voltage_output[voltage_measure_count++] = voltageRatio;
  }
  else
    return;
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

void waitForMeasuringEnd()
{
  while (true)
  {
    if (voltage_measure_count == max_voltage_measure_count)
      break;

    _sleep(1000);
  }
}