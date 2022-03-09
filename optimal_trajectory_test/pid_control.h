#pragma once

namespace adol
{
  class PIDControl
  {
  public:
    PIDControl();
    ~PIDControl();

    double ctrl_time_sec_;
    
    double present_error_;
    double previous_error_;

    double error_sum_;

    double desired_;
    double present_;
    double output_;

    double p_gain_;
    double i_gain_;
    double d_gain_;

    void initialize(double ctrl_time_sec, double p_gain, double i_gain, double d_gain);
    void reinitialize();

    double getOutput(double desired_value, double present_value);
    double getOutput(double present_value);

  };
}