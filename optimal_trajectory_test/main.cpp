#include <iostream>
#include "optimal_trajectory_test.h"

int main(void)
{
  adol::OptimalTrajectoryTest optimal_test;

  std::vector<uint8_t> id_list;
  id_list.push_back(1);

  optimal_test.loadOptimalTrajectory("traj.txt");
  optimal_test.initialize("COM4", 1000000, 4, id_list, 
    1, "calib.txt", 
    "COM3", 1000000,
    0,0,0);

  Sleep(1000);

  optimal_test.startCtrl();

  while (true)
  {
    Sleep(999);
    if (optimal_test.ctrl_flag_ == false)
      break;
  }

  optimal_test.saveTestResult();
  
  return 0;
}