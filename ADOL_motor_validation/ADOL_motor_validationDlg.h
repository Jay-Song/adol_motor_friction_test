
// ADOL_motor_validationDlg.h : header file
//

#pragma once
#include <phidget22.h>
#include "afxwin.h"
#include "dynamixel_sdk\dynamixel_sdk.h"
#include "control_table_XM430.h"
#include "control_table_MX28.h"
#include <Windows.h>
#include "ChartHandler.h"
#include "arduino_current_reader.h"

#define PROTOCOL_VERSION 2.0

union value16
{
  int16_t i16_value;
  uint16_t ui16_value;
  uint8_t bytes[2];
};

union value32
{
  int32_t i32_value;
  uint32_t ui32_value;
  uint8_t bytes[4];
  float f32_value;
};

struct ResultData
{
  double  elapsed_time_sec_;
  int32_t goal_velocity_xm430_;
  int16_t goal_pwm_xm430_;
  int16_t goal_curr_xm430_;

  uint8_t present_temperature_xm430_;
  int16_t present_current_xm430_;
  int32_t present_velocity_xm430_;
  int32_t present_position_xm430_;

  int32_t goal_velocity_mx28_;
  int16_t goal_pwm_mx28_;

  uint8_t present_temperature_mx28_;
  int32_t present_velocity_mx28_;
  int32_t present_position_mx28_;

  double  voltage_output_v_;
  double  measured_weight_g_;
  float   arduino_curr_mx28_mA_;
};


// CADOL_motor_validationDlg dialog
class CADOL_motor_validationDlg : public CDialogEx
{
  // Construction
public:
  CADOL_motor_validationDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_ADOL_MOTOR_VALIDATION_DIALOG };
#endif

protected:
  virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
  HICON m_hIcon;

  // Generated message map functions
  virtual BOOL OnInitDialog();
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  DECLARE_MESSAGE_MAP()
public:
  // for gui
  afx_msg void OnBnClickedStart();
  afx_msg void OnBnClickedStop();
  afx_msg void OnBnClickedClear();
  afx_msg void OnBnClickedSave();
  afx_msg void OnBnClickedSetCalib();
  afx_msg void OnBnClickedConnect();
  afx_msg void OnBnClickedDisconnect();
  afx_msg void OnBnClickedCtrlStart();
  afx_msg void OnBnClickedCtrlReboot();
  afx_msg void OnBnClickedExit();

  CComboBox comport_combo_;
  CComboBox baud_combo_;
  CComboBox driving_ctrl_mode_combo_;
  CComboBox test_ctrl_mode_combo_;

  CComboBox arduino_combo_;

  CComboBox phidget_channel_combo_;

  //for dxl
  dynamixel::PortHandler*   dxl_port_;
  dynamixel::PacketHandler* dxl_packet_;
  dynamixel::GroupBulkWrite* dxl_bulk_write_;
  dynamixel::GroupBulkRead* dxl_bulk_read_;
  bool initializeCommDXL(void);
  bool initializeDXLParam(void);
  bool initializeDXLIndirectAddr(void);
  bool turnTorqueOnDXL(bool on_off);

  void updateGoalValues(void);
  void changeGoalValues(void);

  bool readValuesFromDXLsTx(void);
  bool readValuesFromDXLsRx(void);
  bool readValuesFromDXLsTxRx(void);

  uint8_t driving_dxl_ctrl_mode_;
  uint8_t test_dxl_ctrl_mode_;
  std::map<int, int> map_idx_to_ctrl_mode_;
  // 0 - 0: Current Control Mode
  // 1 - 1: Velcoity Control Mode
  // 2 - 3: Position Control Mode(0 ~360[?)
  //     4: Extended Position Control Mode(Multi - turn)
  //     5: Current - based Position Control Mode
  // 3 - 16: PWM Control Mode(Voltage Control Mode)

  uint8_t dxl_id_test_;
  uint8_t dxl_id_driving_;

  LARGE_INTEGER Frequency;
  LARGE_INTEGER BeginTime;
  LARGE_INTEGER Endtime;
  double elapsed_time;

  bool print_enable_; // cmd print enable

  //for plot
  //CChartViewer force_chart_;
  std::vector<double> time_stamps;
  std::vector<double> scailed_force_raw_;
  //std::vector<double> scailed_force_filetered_;
  //std::vector<double> dxl_test_pos_raw_;
  //std::vector<double> dxl_brake_pos_raw_;
  //std::vector<double> dxl_test_vel_raw_;
  //std::vector<double> dxl_brake_vel_raw_;

  // input for dxl
  std::vector<int32_t> goal_vel_xm430_list_;
  std::vector<int16_t> goal_pwm_mx28_list_;
  value32 goal_velocity_xm430_;
  value16 goal_pwm_xm430_;
  value16 goal_curr_xm430_;

  value32 goal_velocity_mx28_;
  value16 goal_pwm_mx28_;
  //int16_t goal_curr_mx28_;

  int data_idx_;
  void loadData(void);

  //for Arduino
  ArduinoCurrentReader arduino_;
  UINT m_nTimerID; // for timer


  // output from dxl
  ResultData curr_result_;
  std::vector<ResultData> arr_result_data_;

  void terminateCommDXL(void);

  bool comm_; //phiget communication

  //for load cell
  PhidgetVoltageRatioInputHandle voltageRatioInput0_;
  PhidgetReturnCode ret_;
  PhidgetReturnCode errorCode_;
  const char * errorString_;
  char errorDetail_[100];
  size_t errorDetailLen_;

  bool initializePhidget(void);
  bool terminatePhidget(void);

  double calib_factor1_;
  double calib_factor2_;

  bool ctrl_flag_;
  bool dxl_comm_flag_;

};
