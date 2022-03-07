#pragma once
#include <Windows.h>
#include <phidget22.h>


class PhidgetLoadCell
{
public:
  PhidgetLoadCell(int channel_num);
  ~PhidgetLoadCell();

  int channel_num_ = 0;
  double calib1_, calib2_;
  PhidgetVoltageRatioInputHandle voltage_ratio_Input_;

  double  voltage_output_v_;
  double  measured_weight_g_;

  bool initializePhidget(void);
  bool getCalibFactor(std::string file_name);

  //static void CALLBACK onVoltageRatioChange(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio);
  void *ctx_;
  PhidgetVoltageRatioInput_OnVoltageRatioChangeCallback onVoltageRatioChangeCallback;
  static void CALLBACK attachVoltageRatioInput(PhidgetHandle ch, void * ctx);
  static void CALLBACK detachVoltageRatioInput(PhidgetHandle ch, void * ctx);
  static void CALLBACK onVoltageRatioInputError(PhidgetHandle ch, void * ctx, Phidget_ErrorEventCode code, const char * description);
  bool terminatePhidget(void);

};

