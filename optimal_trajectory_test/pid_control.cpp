#include "pid_control.h"

using namespace adol;


PIDControl::PIDControl()
{
  ctrl_time_sec_ = 0.008;

  present_error_ = 0;
  previous_error_ = 0;

  desired_ = 0;
  present_ = 0;

  error_sum_ = 0;

  p_gain_ = 0;
  i_gain_ = 0;
  d_gain_ = 0;

  output_ = 0;
}

PIDControl::~PIDControl()
{

}

void PIDControl::initialize(double ctrl_time_sec, double p_gain, double i_gain, double d_gain)
{
  ctrl_time_sec_ = ctrl_time_sec;

  p_gain_ = p_gain;
  d_gain_ = d_gain;
  i_gain_ = i_gain;

  reinitialize();
}

void PIDControl::reinitialize()
{
  present_error_ = 0;
  previous_error_ = 0;

  desired_ = 0;
  present_ = 0;

  error_sum_ = 0;

  output_ = 0;
}

double PIDControl::getOutput(double desired_value, double present_value)
{
  desired_ = desired_value;
  return getOutput(present_value);
}

double PIDControl::getOutput(double present_value_)
{
  present_ = present_value_;
  previous_error_ = present_error_;
  present_error_ = desired_ - present_;

  error_sum_ += present_error_*ctrl_time_sec_;

  output_ = p_gain_*present_error_
    + d_gain_ *(present_error_ - previous_error_) / ctrl_time_sec_
    + i_gain_ * error_sum_;

  return output_;
}