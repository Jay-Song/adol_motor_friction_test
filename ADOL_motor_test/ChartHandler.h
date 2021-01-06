#pragma once

#include "ChartViewer.h"
#include <string>
#include <vector>
#include <deque>

class ChartHandler
{
public:
  ChartHandler();
  ~ChartHandler();

  void initialize(CChartViewer *chart, std::string chart_title, unsigned int num_of_data, unsigned int data_size);
  void addData(unsigned int data_idx, double data);

  void drawChart();

  CChartViewer *chart_;
  unsigned int num_of_data_set_;
  unsigned int data_size_;
  std::vector< std::deque<double> > data_;

  int ext_bg_color_;
  std::string chart_title_;
};

