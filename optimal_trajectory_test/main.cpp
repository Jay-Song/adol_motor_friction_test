#include <iostream>
#include <conio.h>
#include "optimal_trajectory_test.h"

int main(void)
{
  adol::OptimalTrajectoryTest optimal_test;

  std::vector<uint8_t> id_list;
  id_list.push_back(1);

  optimal_test.initialize("COM4", 1000000, 4, id_list, 
    1, "calib.txt", 
    "COM3", 1000000,
    1.0,0,0);

  optimal_test.loadOptimalTrajectory("tau_sq_optimal_rediscretized_test.txt");

  Sleep(1000);

  optimal_test.startCtrl();

  while (true)
  {
    Sleep(999);
    if (optimal_test.ctrl_flag_ == false)
      break;

    if (_kbhit())
      if (_getch() == 27)
        break;
  }

  optimal_test.saveTestResult();
  
  return 0;
}