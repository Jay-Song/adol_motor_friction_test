#include "stdafx.h"
#include "ChartHandler.h"


ChartHandler::ChartHandler()
{
  chart_ = 0;
  num_of_data_set_ = 1;
  data_size_ = 1000;
  ext_bg_color_ = 0;
  chart_title_ = "";
}


ChartHandler::~ChartHandler()
{
}

void ChartHandler::initialize(CChartViewer *chart, std::string chart_title, unsigned int num_of_data_set, unsigned int data_size)
{
  chart_ = chart;
  num_of_data_set_ = num_of_data_set;
  data_size_ = data_size;

  data_.resize(num_of_data_set_);
  for(unsigned int data_set_idx = 0; data_set_idx < num_of_data_set_; data_set_idx++)
  {
    for(unsigned int data_idx = 0; data_idx < data_size_; data_idx++)
      data_[data_set_idx].push_back(Chart::NoValue);
  }
  chart_title_ = chart_title;
}

void ChartHandler::addData(unsigned int data_set_idx, double data)
{
  data_[data_set_idx].pop_front();
  data_[data_set_idx].push_back(data);
}

void ChartHandler::drawChart()
{
  //// Create an XYChart object 600 x 270 pixels in size, with light grey (f4f4f4) 
  //// background, black (000000) border, 1 pixel raised effect, and with a rounded frame.
  //XYChart *c = new XYChart(600, 270, 0xf4f4f4, 0x000000, 1);
  //c->setRoundedFrame(15790320); //15790320 comes from examples

  //// Set the plotarea at (55, 62) and of size 520 x 175 pixels. Use white (ffffff) 
  //// background. Enable both horizontal and vertical grids by setting their colors to 
  //// grey (cccccc). Set clipping mode to clip the data lines to the plot area.
  //c->setPlotArea(55, 62, 520, 175, 0xffffff, -1, -1, 0xcccccc, 0xcccccc);
  //c->setClipping();

  //// Add a title to the chart using 15 pts Times New Roman Bold Italic font, with a light
  //// grey (dddddd) background, black (000000) border, and a glass like raised effect.
  //c->addTitle(chart_title_.c_str(), "timesbi.ttf", 15 )->setBackground(0xdddddd, 0x000000, Chart::glassEffect());

  //// Add a legend box at the top of the plot area with 9pts Arial Bold font. We set the 
  //// legend box to the same width as the plot area and use grid layout (as opposed to 
  //// flow or top/down layout). This distributes the 3 legend icons evenly on top of the 
  //// plot area.
  //LegendBox *b = c->addLegend2(55, 33, 3, "arialbd.ttf", 9);
  //b->setBackground(Chart::Transparent, Chart::Transparent);
  //b->setWidth(520);

  //// Configure the y-axis with a 10pts Arial Bold axis title
  //c->yAxis()->setTitle("Intensity (V/m)", "arialbd.ttf", 10);

  //// Configure the x-axis to auto-scale with at least 75 pixels between major tick and 
  //// 15  pixels between minor ticks. This shows more minor grid lines on the chart.
  //c->xAxis()->setTickDensity(75, 15);

  //// Now we add the data to the chart. 
  //double lastTime = m_timeStamps[sampleSize - 1];
  //if(lastTime != Chart::NoValue)
  //{
  //  // Set up the x-axis to show the time range in the data buffer
  //  c->xAxis()->setDateScale(lastTime - DataInterval * sampleSize / 1000, lastTime);

  //  // Set the x-axis label format
  //  c->xAxis()->setLabelFormat("{value|hh:nn:ss}");

  //  // Create a line layer to plot the lines
  //  LineLayer *layer = c->addLineLayer();

  //  // The x-coordinates are the timeStamps.
  //  layer->setXData(DoubleArray(m_timeStamps, sampleSize));

  //  // The 3 data series are used to draw 3 lines. Here we put the latest data values
  //  // as part of the data set name, so you can see them updated in the legend box.
  //  char buffer[1024];

  //  sprintf(buffer, "Alpha: <*bgColor=FFCCCC*> %.2f ", m_dataSeriesA[sampleSize - 1]);
  //  layer->addDataSet(DoubleArray(m_dataSeriesA, sampleSize), 0xff0000, buffer);

  //  sprintf(buffer, "Beta: <*bgColor=CCFFCC*> %.2f ", m_dataSeriesB[sampleSize - 1]);
  //  layer->addDataSet(DoubleArray(m_dataSeriesB, sampleSize), 0x00cc00, buffer);

  //  sprintf(buffer, "Gamma: <*bgColor=CCCCFF*> %.2f ", m_dataSeriesC[sampleSize - 1]);
  //  layer->addDataSet(DoubleArray(m_dataSeriesC, sampleSize), 0x0000ff, buffer);
  //}

  //// Set the chart image to the WinChartViewer
  //viewer->setChart(c);
  //delete c;
}
