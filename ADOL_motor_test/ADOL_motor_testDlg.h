
// ADOL_motor_testDlg.h : header file
//

#pragma once
#include <phidget22.h>
#include "afxwin.h"
#include "dynamixel_sdk\dynamixel_sdk.h"
#include "control_table_XM430.h"
#include "control_table_MX28.h"
#include <Windows.h>
#include "ChartHandler.h"

#define PROTOCOL_VERSION 2.0


// CADOL_motor_testDlg dialog
class CADOL_motor_testDlg : public CDialogEx
{
// Construction
public:
	CADOL_motor_testDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADOL_MOTOR_TEST_DIALOG };
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
  
  afx_msg void OnBnClickedExit();
  afx_msg void OnBnClickedConnect();
  afx_msg void OnBnClickedDisconnect();
  afx_msg void OnCbnSelchangeComportCombo();
  afx_msg void OnCbnSelchangeBaudCombo();
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedStart();
  afx_msg void OnBnClickedStop();
  afx_msg void OnBnClickedClear();
  afx_msg void OnBnClickedSetCalib();
  afx_msg void OnBnClickedSet();
  afx_msg void OnBnClickedSetPwm();
  afx_msg void OnBnClickedSetCurr();

  afx_msg void OnTimer(UINT_PTR nIDEvent);

  CButton connect_btn_;
  CComboBox comport_combo_;
  CComboBox baud_combo_;
  CComboBox ctrl_mode_combo_;
  CEdit dxl_velocity_edit_;



  //for plot
  CChartViewer force_chart_;
  std::vector<double> time_stamps;
  std::vector<double> scailed_force_raw_;
  std::vector<double> scailed_force_filetered_;
  std::vector<double> dxl_test_pos_raw_;
  std::vector<double> dxl_brake_pos_raw_;
  std::vector<double> dxl_test_vel_raw_;
  std::vector<double> dxl_brake_vel_raw_;


  //for dxl
  dynamixel::PortHandler*   dxl_port_;
  dynamixel::PacketHandler* dxl_packet_;
  dynamixel::GroupBulkRead* dxl_bulk_read_;

  bool initializeCommDXL(void);
  bool initializeDXLParam(void);
  bool turnTorqueOnDXL(bool on_off);
  void changeGoalVelocity(void);
  void changeGoalPWM(void);
  void changeGoalCurrent(void);
  bool readValuesFromDXLs(void);

  uint8_t dxl_ctrl_mode_; 
  // 0: Current Control Mode
  // 1: Velcoity Control Mode
  // 3: Position Control Mode(0 ~360[?)
  // 4: Extended Position Control Mode(Multi - turn)
  // 5: Current - based Position Control Mode
  //16: PWM Control Mode(Voltage Control Mode)

  std::map<int, int> map_idx_to_ctrl_mode_;

  uint8_t dxl_id_test_;
  uint8_t dxl_id_brake_;

  LARGE_INTEGER Frequency;
  LARGE_INTEGER BeginTime;
  LARGE_INTEGER Endtime;
  double elapsed_time;

  bool print_enable_;


  uint8_t dxl_data_array_[11];

  // output from dxl
  int32_t goal_velocity_xm430_;
  int16_t goal_pwm_xm430_;
  int16_t goal_curr_xm430_;

  uint8_t present_temperature_xm430_;
  int16_t present_current_xm430_;
  int32_t present_velocity_xm430_;
  int32_t present_position_xm430_;

  uint8_t present_temperature_MX28_;
  int32_t present_velocity_MX28_;
  int32_t present_position_MX28_;

  std::vector<double> arr_elapsed_time_;
  std::vector<int32_t> arr_goal_velocity_xm430_;
  std::vector<int16_t> arr_goal_pwm_xm430_;
  std::vector<int16_t> arr_goal_curr_xm430_;
  
  std::vector<uint8_t> arr_present_temperature_xm430_;
  std::vector<int16_t> arr_present_current_xm430_;
  std::vector<int32_t> arr_present_velocity_xm430_;
  std::vector<int32_t> arr_present_position_xm430_;
  
  std::vector<uint8_t> arr_present_temperature_MX28_;
  std::vector<int32_t> arr_present_velocity_MX28_;
  std::vector<int32_t> arr_present_position_MX28_;


  void terminateCommDXL(void);

  bool comm_;

  //for load cell
  PhidgetVoltageRatioInputHandle voltageRatioInput0_;
  PhidgetReturnCode ret_;
  PhidgetReturnCode errorCode_;
  const char * errorString_;
  char errorDetail_[100];
  size_t errorDetailLen_;

  bool initializePhidget(void);
  bool terminatePhidget(void);


  double voltage_output_;
  double measured_force_N_;
  double calib_factor1_;
  double calib_factor2_;

  std::vector<double> arr_voltage_output_;
  std::vector<double> arr_measured_force_N_;


  virtual BOOL PreTranslateMessage(MSG* pMsg);
  afx_msg void OnBnClickedSave();
  CComboBox phidget_channel_combo_;
};
