
// ADOL_motor_testDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ADOL_motor_test.h"
#include "ADOL_motor_testDlg.h"
#include "afxdialogex.h"

#include <iostream>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define DEFAULT_CALIB_FACTOR1 (1048659.47918428)
#define DEFAULT_CALIB_FACTOR2 (-1838.691939)

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
  CAboutDlg();

  // Dialog Data
#ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_ABOUTBOX };
#endif

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
  DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CADOL_motor_testDlg dialog



CADOL_motor_testDlg::CADOL_motor_testDlg(CWnd* pParent /*=NULL*/)
  : CDialogEx(IDD_ADOL_MOTOR_TEST_DIALOG, pParent)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CADOL_motor_testDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_COMPORT_COMBO, comport_combo_);
  DDX_Control(pDX, IDC_BAUD_COMBO, baud_combo_);
  DDX_Control(pDX, IDC_DXL_VEL, dxl_velocity_edit_);
  DDX_Control(pDX, IDC_CONNECT, connect_btn_);
  DDX_Control(pDX, IDC_CTRL_MODE_COMBO, ctrl_mode_combo_);
  DDX_Control(pDX, IDC_FORCE_CHART, force_chart_);
  DDX_Control(pDX, IDC_CHANNEL, phidget_channel_combo_);
}

BEGIN_MESSAGE_MAP(CADOL_motor_testDlg, CDialogEx)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED(IDC_SET, &CADOL_motor_testDlg::OnBnClickedSet)
  ON_BN_CLICKED(IDC_CONNECT, &CADOL_motor_testDlg::OnBnClickedConnect)
  ON_CBN_SELCHANGE(IDC_COMPORT_COMBO, &CADOL_motor_testDlg::OnCbnSelchangeComportCombo)
  ON_CBN_SELCHANGE(IDC_BAUD_COMBO, &CADOL_motor_testDlg::OnCbnSelchangeBaudCombo)
  ON_BN_CLICKED(IDOK, &CADOL_motor_testDlg::OnBnClickedOk)
  ON_BN_CLICKED(IDC_START, &CADOL_motor_testDlg::OnBnClickedStart)
  ON_BN_CLICKED(IDC_STOP, &CADOL_motor_testDlg::OnBnClickedStop)
  ON_BN_CLICKED(IDC_SET_CALIB, &CADOL_motor_testDlg::OnBnClickedSetCalib)
  ON_BN_CLICKED(IDC_CLEAR, &CADOL_motor_testDlg::OnBnClickedClear)
  ON_BN_CLICKED(IDC_SET_PWM, &CADOL_motor_testDlg::OnBnClickedSetPwm)
  ON_BN_CLICKED(IDC_DISCONNECT, &CADOL_motor_testDlg::OnBnClickedDisconnect)
  ON_BN_CLICKED(IDC_SET_CURR, &CADOL_motor_testDlg::OnBnClickedSetCurr)
  ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SAVE, &CADOL_motor_testDlg::OnBnClickedSave)
END_MESSAGE_MAP()


// CADOL_motor_testDlg message handlers

BOOL CADOL_motor_testDlg::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  // Add "About..." menu item to system menu.

  // IDM_ABOUTBOX must be in the system command range.
  ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
  ASSERT(IDM_ABOUTBOX < 0xF000);

  CMenu* pSysMenu = GetSystemMenu(FALSE);
  if(pSysMenu != NULL)
  {
    BOOL bNameValid;
    CString strAboutMenu;
    bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
    ASSERT(bNameValid);
    if(!strAboutMenu.IsEmpty())
    {
      pSysMenu->AppendMenu(MF_SEPARATOR);
      pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
    }
  }

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);			// Set big icon
  SetIcon(m_hIcon, FALSE);		// Set small icon

  //Initializing port combo
  for(int port_num = 1; port_num < 253; port_num++)
  {
    CString port_string;
    port_string.Format(L"COM%d", port_num);
    comport_combo_.AddString(port_string);
  }

  //Initianlizing baud combo
  baud_combo_.AddString(L"57600");
  baud_combo_.AddString(L"1000000");

  //Initializing control mode combo
  ctrl_mode_combo_.AddString(L" 0: Current");
  ctrl_mode_combo_.AddString(L" 1: Velcoity");
  ctrl_mode_combo_.AddString(L"16: PWM");

  phidget_channel_combo_.AddString(L"CH 0");
  phidget_channel_combo_.AddString(L"CH 1");
  phidget_channel_combo_.AddString(L"CH 2");
  phidget_channel_combo_.AddString(L"CH 3");


  map_idx_to_ctrl_mode_[0] = 0;
  map_idx_to_ctrl_mode_[1] = 1;
  map_idx_to_ctrl_mode_[2] = 16;

  //disable disconnect btn
  GetDlgItem(IDC_DISCONNECT)->EnableWindow(false);

  //Initializing dxl paramater
  dxl_port_ = 0;
  dxl_ctrl_mode_ = 1;

  dxl_bulk_read_ = 0;

  dxl_id_test_ = 1;
  dxl_id_brake_ = 2;

  goal_velocity_xm430_ = 0;
  goal_pwm_xm430_ = 0;
  goal_curr_xm430_ = 0;

  present_temperature_xm430_ = 0;
  present_current_xm430_ = 0;
  present_velocity_xm430_ = 0;
  present_position_xm430_ = 0;

  present_temperature_MX28_ = 0;
  present_velocity_MX28_ = 0;
  present_position_MX28_ = 0;

  //For measuring the time
  QueryPerformanceFrequency(&Frequency);

  //Initialize Two Value for measuring time
  QueryPerformanceCounter(&BeginTime);
  QueryPerformanceCounter(&Endtime);
  elapsed_time = 0;

  print_enable_ = false;

  errorDetailLen_ = 100;
  voltage_output_ = 0;
  measured_force_N_ = 0;
  calib_factor1_ = DEFAULT_CALIB_FACTOR1;
  calib_factor2_ = DEFAULT_CALIB_FACTOR2;

  CString calib_factor1_str;
  CString calib_factor2_str;

  std::ifstream calib_file_handler;

  calib_file_handler.open("calib.txt");

  if (!calib_file_handler.is_open())
  {
	  std::cout << "failed to open the calibration file" << std::endl;
	  calib_factor1_str.Format(_T("%.8f"), DEFAULT_CALIB_FACTOR1);
	  calib_factor2_str.Format(_T("%.8f"), DEFAULT_CALIB_FACTOR2);
  }
  else
  {
    std::cout << "succeeded in opening the calibration file" << std::endl;

    std::string a;
    double calib_factor = 0;
    calib_file_handler >> a;
    calib_file_handler >> calib_factor;
    std::cout << "calib_factor1: " << calib_factor << std::endl;
    calib_factor1_str.Format(_T("%.8f"), calib_factor);

    calib_file_handler >> a;
    calib_file_handler >> calib_factor;
    std::cout << "calib_factor2: " << calib_factor << std::endl;
    calib_factor2_str.Format(_T("%.8f"), calib_factor);
  }

  calib_file_handler.close();

  GetDlgItem(IDC_CALIB_FACOR1)->SetWindowTextW(calib_factor1_str);
  GetDlgItem(IDC_CALIB_FACOR2)->SetWindowTextW(calib_factor2_str);

  comm_ = false;

  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CADOL_motor_testDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
  if((nID & 0xFFF0) == IDM_ABOUTBOX)
  {
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
  }
  else
  {
    CDialogEx::OnSysCommand(nID, lParam);
  }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CADOL_motor_testDlg::OnPaint()
{
  if(IsIconic())
  {
    CPaintDC dc(this); // device context for painting

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  }
  else
  {
    CDialogEx::OnPaint();
  }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CADOL_motor_testDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}


bool CADOL_motor_testDlg::initializeCommDXL(void)
{
  CString port_string;
  this->comport_combo_.GetLBText(this->comport_combo_.GetCurSel(), port_string);

  CString baud_string;
  this->baud_combo_.GetLBText(this->baud_combo_.GetCurSel(), baud_string);

  dxl_port_ = dynamixel::PortHandler::getPortHandler((CStringA)port_string);
  dxl_packet_ = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

  int dxl_comm_result = COMM_TX_FAIL;
  uint8_t dxl_error = 0;

  if(dxl_port_->openPort())
  {
    printf("Succeeded in opening the port\n");
  }
  else
  {
    printf("Failed to open the port\n");
    return false;
  }


  if(dxl_port_->setBaudRate(_ttoi(baud_string)))
  {
    printf("Succeeded in changing the baudrate\n");
  }
  else
  {
    printf("Failed to change the baudrate\n");
    return false;
  }

  return true;
}

void CADOL_motor_testDlg::terminateCommDXL(void)
{
  if(dxl_port_ != 0)
  {
    dxl_port_->closePort();
    dxl_port_ = 0;
  }

  if(dxl_bulk_read_ != 0)
  {
    dxl_bulk_read_->clearParam();
    delete dxl_bulk_read_;
    dxl_bulk_read_ = 0;
  }
}

bool CADOL_motor_testDlg::initializeDXLParam(void)
{
  //indirect address of brake motor
  uint8_t num_indirect_addr_set = 22;
  uint8_t indirect_addr_array[22];
  indirect_addr_array[0] = DXL_LOBYTE(XM430::ADDR_PRESENT_CURRENT + 0);
  indirect_addr_array[1] = DXL_HIBYTE(XM430::ADDR_PRESENT_CURRENT + 0);
  indirect_addr_array[2] = DXL_LOBYTE(XM430::ADDR_PRESENT_CURRENT + 1);
  indirect_addr_array[3] = DXL_HIBYTE(XM430::ADDR_PRESENT_CURRENT + 1);

  indirect_addr_array[4] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 0);
  indirect_addr_array[5] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 0);
  indirect_addr_array[6] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 1);
  indirect_addr_array[7] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 1);
  indirect_addr_array[8] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 2);
  indirect_addr_array[9] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 2);
  indirect_addr_array[10] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 3);
  indirect_addr_array[11] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 3);

  indirect_addr_array[12] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 0);
  indirect_addr_array[13] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 0);
  indirect_addr_array[14] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 1);
  indirect_addr_array[15] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 1);
  indirect_addr_array[16] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 2);
  indirect_addr_array[17] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 2);
  indirect_addr_array[18] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 3);
  indirect_addr_array[19] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 3);

  indirect_addr_array[20] = DXL_LOBYTE(XM430::ADDR_PRESENT_TEMPERATURE + 0);
  indirect_addr_array[21] = DXL_HIBYTE(XM430::ADDR_PRESENT_TEMPERATURE + 0);

  uint8_t dxl_error = 0;
  int dxl_result = dxl_packet_->writeTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_INDIRECT_ADDR_1, num_indirect_addr_set, indirect_addr_array, &dxl_error);
  if(dxl_result == COMM_SUCCESS)
  {
    printf("Succeeded in changing Indirect Address\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to change Indirect Address\n");
    return false;
  }

  if(dxl_error != 0)
    printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));

  //indirec address for MX28
  num_indirect_addr_set = 18;
  indirect_addr_array[0] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 0);
  indirect_addr_array[1] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 0);
  indirect_addr_array[2] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 1);
  indirect_addr_array[3] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 1);
  indirect_addr_array[4] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 2);
  indirect_addr_array[5] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 2);
  indirect_addr_array[6] = DXL_LOBYTE(XM430::ADDR_PRESENT_VELOCITY + 3);
  indirect_addr_array[7] = DXL_HIBYTE(XM430::ADDR_PRESENT_VELOCITY + 3);

  indirect_addr_array[8] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 0);
  indirect_addr_array[9] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 0);
  indirect_addr_array[10] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 1);
  indirect_addr_array[11] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 1);
  indirect_addr_array[12] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 2);
  indirect_addr_array[13] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 2);
  indirect_addr_array[14] = DXL_LOBYTE(XM430::ADDR_PRESENT_POSITION + 3);
  indirect_addr_array[15] = DXL_HIBYTE(XM430::ADDR_PRESENT_POSITION + 3);

  indirect_addr_array[16] = DXL_LOBYTE(XM430::ADDR_PRESENT_TEMPERATURE + 0);
  indirect_addr_array[17] = DXL_HIBYTE(XM430::ADDR_PRESENT_TEMPERATURE + 0);

  dxl_error = 0;
  dxl_result = dxl_packet_->writeTxRx(dxl_port_, dxl_id_test_, MX28::ADDR_INDIRECT_ADDR_1, num_indirect_addr_set, indirect_addr_array, &dxl_error);
  if(dxl_result == COMM_SUCCESS)
  {
    printf("Succeeded in changing Indirect Address of MX28\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to change Indirect Address of MX28\n");
    return false;
  }

  if(dxl_error != 0)
    printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));


  // change operating mode of brake motor
  dxl_ctrl_mode_ = map_idx_to_ctrl_mode_[ctrl_mode_combo_.GetCurSel()];
  dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_OPERATING_MODE, dxl_ctrl_mode_, &dxl_error);
  if(dxl_result == COMM_SUCCESS)
  {
    printf("Succeeded in changing Operating Mode\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to change Operating Mode\n");
    return false;
  }

  //Initialize dxl_bulk_read;
  dxl_bulk_read_ = new dynamixel::GroupBulkRead(dxl_port_, dxl_packet_);
  dxl_bulk_read_->addParam(dxl_id_test_, XM430::ADDR_INDIRECT_DATA_1, 9);
  dxl_bulk_read_->addParam(dxl_id_brake_, MX28::ADDR_INDIRECT_DATA_1, 11);

  return true;
}

bool CADOL_motor_testDlg::turnTorqueOnDXL(bool on_off)
{
  uint8_t dxl_error = 0;
  int dxl_result = 0;
  if(on_off)
    dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_TORQUE_ENABLE, 1, &dxl_error);
  else
    dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_TORQUE_ENABLE, 0, &dxl_error);

  if(dxl_error == COMM_SUCCESS)
  {
    printf("Succeeded in turning torque on\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to turn  torque on\n");
    return false;
  }

  if(dxl_error != 0)
    printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));

  return true;
}

void CADOL_motor_testDlg::changeGoalCurrent(void)
{
  if((goal_curr_xm430_ >= -1193) || (goal_curr_xm430_ <= 1193))
  {
    uint8_t dxl_error = 0;
    int dxl_result = dxl_packet_->write2ByteTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_GOAL_CURRENT, goal_curr_xm430_, &dxl_error);
    if (dxl_result != COMM_SUCCESS)
    {
      printf("%s\n", dxl_packet_->getTxRxResult(dxl_result));
      return;
    }

    if (dxl_error != 0)
      printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));

  }
}

void CADOL_motor_testDlg::changeGoalVelocity(void)
{
  if((goal_velocity_xm430_ >= -200) || (goal_velocity_xm430_ <= 200))
  {
    uint8_t dxl_error = 0;
    int dxl_result = dxl_packet_->write4ByteTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_GOAL_VELOCITY, goal_velocity_xm430_, &dxl_error);
    if (dxl_result != COMM_SUCCESS)
    {
      printf("%s\n", dxl_packet_->getTxRxResult(dxl_result));
      return;
    }

    if (dxl_error != 0)
      printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));
  }
}

void CADOL_motor_testDlg::changeGoalPWM(void)
{
  if((goal_pwm_xm430_ >= -885) || (goal_pwm_xm430_ <= 885))
  {
    uint8_t dxl_error = 0;
    int dxl_result = dxl_packet_->write2ByteTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_GOAL_PWM, goal_pwm_xm430_, &dxl_error);

  if (dxl_result != COMM_SUCCESS)
  {
    printf("%s\n", dxl_packet_->getTxRxResult(dxl_result));
    return;
  }

    if(dxl_error != 0)
      printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));

  }
}

bool CADOL_motor_testDlg::readValuesFromDXLs(void)
{
  //uint8_t dxl_error = 0;
  //int dxl_result = dxl_packet_->readTxRx(dxl_port_, dxl_id_brake_, XM430::ADDR_INDIRECT_DATA_1, 11, dxl_data_array_, &dxl_error);
  //present_current_xm430_ = DXL_MAKEWORD(dxl_data_array_[0], dxl_data_array_[1]);
  //present_velocity_xm430_ = DXL_MAKEDWORD(DXL_MAKEWORD(dxl_data_array_[2], dxl_data_array_[3]), DXL_MAKEWORD(dxl_data_array_[4], dxl_data_array_[5]));
  //present_position_xm430_ = DXL_MAKEDWORD(DXL_MAKEWORD(dxl_data_array_[6], dxl_data_array_[7]), DXL_MAKEWORD(dxl_data_array_[8], dxl_data_array_[9]));
  //present_temperature_xm430_ = dxl_data_array_[10];

  //if(dxl_error != 0)
  //  printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));


  int dxl_result = dxl_bulk_read_->txRxPacket();
  if(dxl_result != COMM_SUCCESS)
  {
    printf("%s\n", dxl_packet_->getTxRxResult(dxl_result));
    return false;
  }

  present_current_xm430_     = dxl_bulk_read_->getData(dxl_id_brake_, XM430::ADDR_INDIRECT_DATA_1 + 0, 2);
  present_velocity_xm430_    = dxl_bulk_read_->getData(dxl_id_brake_, XM430::ADDR_INDIRECT_DATA_1 + 2, 4);
  present_position_xm430_    = dxl_bulk_read_->getData(dxl_id_brake_, XM430::ADDR_INDIRECT_DATA_1 + 6, 4);
  present_temperature_xm430_ = dxl_bulk_read_->getData(dxl_id_brake_, XM430::ADDR_INDIRECT_DATA_1 + 10, 1);
  
  present_velocity_MX28_    = dxl_bulk_read_->getData(dxl_id_test_, MX28::ADDR_INDIRECT_DATA_1 + 0, 4);
  present_position_MX28_    = dxl_bulk_read_->getData(dxl_id_test_, MX28::ADDR_INDIRECT_DATA_1 + 4, 4);
  present_temperature_MX28_ = dxl_bulk_read_->getData(dxl_id_test_, MX28::ADDR_INDIRECT_DATA_1 + 8, 1);
  

  return true;
}

//For Phidget
void __stdcall onVoltageRatioInput0_VoltageRatioChange(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio) {

  CADOL_motor_testDlg* test_dlg = (CADOL_motor_testDlg*)ctx;

  if(test_dlg->comm_ == false)
    return;

  QueryPerformanceCounter(&(test_dlg->Endtime));
  uint64_t elapsed = test_dlg->Endtime.QuadPart - test_dlg->BeginTime.QuadPart;
  test_dlg->elapsed_time = (double)elapsed / (double)test_dlg->Frequency.QuadPart;

  if(test_dlg->dxl_ctrl_mode_ == 0)
    test_dlg->changeGoalCurrent();
  else if(test_dlg->dxl_ctrl_mode_ == 1)
    test_dlg->changeGoalVelocity();
  else
    test_dlg->changeGoalPWM();

  test_dlg->readValuesFromDXLs();
  //printf("t:%lf vol:%lf\n", duringtime, voltageRatio*100000000.0);

  test_dlg->voltage_output_ = voltageRatio;
  test_dlg->measured_force_N_ = voltageRatio*test_dlg->calib_factor1_ + test_dlg->calib_factor2_;

  if(test_dlg->dxl_ctrl_mode_ == 0)
  {
    if(test_dlg->print_enable_ == true)
      printf("t: %lf vol*10^8: %lf f: %lf gc : %d cv: %d cp: %d ct: %d cc: %d mcv: %d mcp: %d mct: %d\n",
             test_dlg->elapsed_time,
             voltageRatio*100000000.0,
             test_dlg->measured_force_N_,
             test_dlg->goal_curr_xm430_,
             test_dlg->present_velocity_xm430_,
             test_dlg->present_position_xm430_,
             test_dlg->present_temperature_xm430_,
             test_dlg->present_current_xm430_,
             test_dlg->present_velocity_MX28_,
             test_dlg->present_position_MX28_,
             test_dlg->present_temperature_MX28_);
  }
  else if(test_dlg->dxl_ctrl_mode_ == 1)
  {
    if(test_dlg->print_enable_ == true)
      printf("t: %lf vol*10^8: %lf f: %lf gv : %d cv: %d cp: %d ct: %d cc: %d mcv: %d mcp: %d mct: %d\n",
             test_dlg->elapsed_time,
             voltageRatio*100000000.0,
             test_dlg->measured_force_N_,
             test_dlg->goal_velocity_xm430_,
             test_dlg->present_velocity_xm430_,
             test_dlg->present_position_xm430_,
             test_dlg->present_temperature_xm430_,
             test_dlg->present_current_xm430_,
             test_dlg->present_velocity_MX28_,
             test_dlg->present_position_MX28_,
             test_dlg->present_temperature_MX28_);
  }
  else
  {
    if(test_dlg->print_enable_ == true)
      printf("t: %lf vol*10^8: %lf f: %lf gpwm : %d cv: %d cp: %d ct: %d cc: %d mcv: %d mcp: %d mct: %d\n",
             test_dlg->elapsed_time,
             voltageRatio*100000000.0,
             test_dlg->measured_force_N_,
             test_dlg->goal_pwm_xm430_,
             test_dlg->present_velocity_xm430_,
             test_dlg->present_position_xm430_,
             test_dlg->present_temperature_xm430_,
             test_dlg->present_current_xm430_,
             test_dlg->present_velocity_MX28_,
             test_dlg->present_position_MX28_,
             test_dlg->present_temperature_MX28_);
  }

  if (test_dlg->print_enable_ == true)
  {
    test_dlg->arr_elapsed_time_.push_back(test_dlg->elapsed_time);
    test_dlg->arr_voltage_output_.push_back(test_dlg->voltage_output_);
    test_dlg->arr_goal_velocity_xm430_.push_back(test_dlg->goal_velocity_xm430_);
    test_dlg->arr_goal_pwm_xm430_.push_back(test_dlg->goal_pwm_xm430_);
    test_dlg->arr_goal_curr_xm430_.push_back(test_dlg->goal_curr_xm430_);
    test_dlg->arr_present_temperature_xm430_.push_back(test_dlg->present_temperature_xm430_);
    test_dlg->arr_present_current_xm430_.push_back(test_dlg->present_current_xm430_);
    test_dlg->arr_present_velocity_xm430_.push_back(test_dlg->present_velocity_xm430_);
    test_dlg->arr_present_position_xm430_.push_back(test_dlg->present_position_xm430_);
    test_dlg->arr_present_temperature_MX28_.push_back(test_dlg->present_temperature_MX28_);
    test_dlg->arr_present_velocity_MX28_.push_back(test_dlg->present_velocity_MX28_);
    test_dlg->arr_present_position_MX28_.push_back(test_dlg->present_position_MX28_);
  }
}

void __stdcall onVoltageRatioInput0_Attach(PhidgetHandle ch, void * ctx) {
  printf("Attach!\n");
}

void __stdcall onVoltageRatioInput0_Detach(PhidgetHandle ch, void * ctx) {
  printf("Detach!\n");
}

void __stdcall onVoltageRatioInput0_Error(PhidgetHandle ch, void * ctx, Phidget_ErrorEventCode code, const char * description) {
  printf("Description: %s\n", description);
  printf("----------\n");
}

bool CADOL_motor_testDlg::initializePhidget(void)
{
  PhidgetLog_enable(PHIDGET_LOG_INFO, "phidgetlog.log");
  PhidgetVoltageRatioInput_create(&voltageRatioInput0_);
  PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput0_, onVoltageRatioInput0_VoltageRatioChange, this);
  Phidget_setOnAttachHandler((PhidgetHandle)voltageRatioInput0_, onVoltageRatioInput0_Attach, NULL);
  Phidget_setOnDetachHandler((PhidgetHandle)voltageRatioInput0_, onVoltageRatioInput0_Detach, NULL);
  Phidget_setOnErrorHandler((PhidgetHandle)voltageRatioInput0_, onVoltageRatioInput0_Error, NULL);

  std::cout << "phidget_channel_combo_.GetCurSel() : " << phidget_channel_combo_.GetCurSel() << std::endl;

  Phidget_setChannel((PhidgetHandle)voltageRatioInput0_, 1);

  //Open your Phidgets and wait for attachment
  ret_ = Phidget_openWaitForAttachment((PhidgetHandle)voltageRatioInput0_, 5000);
  if(ret_ != EPHIDGET_OK) {
    Phidget_getLastError(&errorCode_, &errorString_, errorDetail_, &errorDetailLen_);
    printf("Error (%d): %s", errorCode_, errorString_);
    return false;
  }

  QueryPerformanceCounter(&BeginTime);
  //Set the Data Interval of the Device to 8 ms
  ret_ = Phidget_setDataInterval((PhidgetHandle)voltageRatioInput0_, 8);
  if(ret_ != EPHIDGET_OK) {
    Phidget_getLastError(&errorCode_, &errorString_, errorDetail_, &errorDetailLen_);
    printf("Error (%d): %s", errorCode_, errorString_);
    return false;
  }
  comm_ = true;
  return true;
}

bool CADOL_motor_testDlg::terminatePhidget(void)
{
  comm_ = false;
  Sleep(16);
  ret_ = Phidget_close((PhidgetHandle)voltageRatioInput0_);
  if(ret_ != EPHIDGET_OK) {
    Phidget_getLastError(&errorCode_, &errorString_, errorDetail_, &errorDetailLen_);
    printf("Error (%d): %s", errorCode_, errorString_);
    //exit(1);
  }
  PhidgetVoltageRatioInput_delete(&voltageRatioInput0_);

  return true;
}

//For UI
void CADOL_motor_testDlg::OnBnClickedExit()
{
  // TODO: Add your control notification handler code here
}

void CADOL_motor_testDlg::OnBnClickedSet()
{
  // TODO: Add your control notification handler code here
  CString dxl_velocity_str;
  dxl_velocity_edit_.GetWindowTextW(dxl_velocity_str);
  goal_velocity_xm430_ = _ttoi(dxl_velocity_str);
}

void CADOL_motor_testDlg::OnBnClickedConnect()
{
  // TODO: Add your control notification handler code here
  if(initializeCommDXL() == false)
  {
    AfxMessageBox(L"Failed to connect to USB2DXL");
    return;
  }

  if(turnTorqueOnDXL(false) == false)
  {
    AfxMessageBox(L"Failed to initialize dxl param");
    terminateCommDXL();
    return;
  }


  if(initializeDXLParam() == false)
  {
    AfxMessageBox(L"Failed to initialize dxl param");
    terminateCommDXL();
    return;
  }

  if(turnTorqueOnDXL(true) == false) // to test, torque off
  {
    AfxMessageBox(L"Failed to initialize dxl param");
    terminateCommDXL();
    return;
  }

  if(initializePhidget() == false)
  {
    AfxMessageBox(L"Failed to connect to Phiget");
    terminateCommDXL();
    terminatePhidget();
    return;
  }

  connect_btn_.EnableWindow(false);
  comport_combo_.EnableWindow(false);
  baud_combo_.EnableWindow(false);
  ctrl_mode_combo_.EnableWindow(false);
  phidget_channel_combo_.EnableWindow(false);
}

void CADOL_motor_testDlg::OnCbnSelchangeComportCombo()
{
  // TODO: Add your control notification handler code here
}

void CADOL_motor_testDlg::OnCbnSelchangeBaudCombo()
{
  // TODO: Add your control notification handler code here
}

void CADOL_motor_testDlg::OnBnClickedOk()
{
  // TODO: Add your control notification handler code here
  goal_velocity_xm430_ = 0;
  goal_pwm_xm430_ = 0;
  goal_curr_xm430_ = 0;
  Sleep(200);

  if(dxl_port_ != 0)
  {
    terminatePhidget();
    terminateCommDXL();
  }

  CDialogEx::OnOK();
}

void CADOL_motor_testDlg::OnBnClickedSetCalib()
{
  //get factor1
  CString calib_factor1_str;
  GetDlgItem(IDC_CALIB_FACOR1)->GetWindowTextW(calib_factor1_str);
  calib_factor1_ = _ttof(calib_factor1_str);

  //get factor2
  CString calib_factor2_str;
  GetDlgItem(IDC_CALIB_FACOR2)->GetWindowTextW(calib_factor2_str);
  calib_factor2_ = _ttof(calib_factor2_str);

  printf("calib: %.8f %.8f\n", calib_factor1_, calib_factor2_);
}

void CADOL_motor_testDlg::OnBnClickedStart()
{
  print_enable_ = true;
   
  arr_elapsed_time_.clear();
  arr_voltage_output_.clear();
  arr_goal_velocity_xm430_.clear();
  arr_goal_pwm_xm430_.clear();
  arr_goal_curr_xm430_.clear();
  arr_present_temperature_xm430_.clear();
  arr_present_current_xm430_.clear();
  arr_present_velocity_xm430_.clear();
  arr_present_position_xm430_.clear();
  arr_present_temperature_MX28_.clear();
  arr_present_velocity_MX28_.clear();
  arr_present_position_MX28_.clear();

  //SetTimer
}

void CADOL_motor_testDlg::OnBnClickedStop()
{
  print_enable_ = false;
}

void CADOL_motor_testDlg::OnBnClickedClear()
{
  system("cls");
  bool curr_print_ = print_enable_;
  print_enable_ = false;

  arr_elapsed_time_.clear();
  arr_voltage_output_.clear();
  arr_goal_velocity_xm430_.clear();
  arr_goal_pwm_xm430_.clear();
  arr_goal_curr_xm430_.clear();
  arr_present_temperature_xm430_.clear();
  arr_present_current_xm430_.clear();
  arr_present_velocity_xm430_.clear();
  arr_present_position_xm430_.clear();
  arr_present_temperature_MX28_.clear();
  arr_present_velocity_MX28_.clear();
  arr_present_position_MX28_.clear();

  print_enable_ = curr_print_;
}

void CADOL_motor_testDlg::OnBnClickedSave()
{
  print_enable_ = false;
  Sleep(8);
  
  std::ofstream log_file;
  CTime cTime = CTime::GetCurrentTime(); // get current date and time
  CString file_path;
  file_path.Format(_T("log_%04d%02d%02d%02d%02d%02d.txt"),
    cTime.GetYear(), cTime.GetMonth(),
    cTime.GetDay(), cTime.GetHour(),
    cTime.GetMinute(),
    cTime.GetSecond());
  
  log_file.open(file_path);
  
  for(unsigned int arr_idx = 0; arr_idx < arr_elapsed_time_.size(); arr_idx++)
  {
  
    log_file << arr_elapsed_time_[arr_idx] << "\t"
      << arr_voltage_output_[arr_idx] << "\t"
      << arr_goal_velocity_xm430_[arr_idx] << "\t"
      << arr_goal_pwm_xm430_[arr_idx] << "\t"
      << arr_goal_curr_xm430_[arr_idx] << "\t"
      << arr_present_temperature_xm430_[arr_idx] << "\t"
      << arr_present_current_xm430_[arr_idx] << "\t"
      << arr_present_velocity_xm430_[arr_idx] << "\t"
      << arr_present_position_xm430_[arr_idx] << "\t"
      << arr_present_temperature_MX28_[arr_idx] << "\t"
      << arr_present_velocity_MX28_[arr_idx] << "\t"
      << arr_present_position_MX28_[arr_idx] << "\t" << std::endl;
  }

  log_file.close();
  
  arr_elapsed_time_.clear();
  arr_voltage_output_.clear();
  arr_goal_velocity_xm430_.clear();
  arr_goal_pwm_xm430_.clear();
  arr_goal_curr_xm430_.clear();
  arr_present_temperature_xm430_.clear();
  arr_present_current_xm430_.clear();
  arr_present_velocity_xm430_.clear();
  arr_present_position_xm430_.clear();
  arr_present_temperature_MX28_.clear();
  arr_present_velocity_MX28_.clear();
  arr_present_position_MX28_.clear();
}


void CADOL_motor_testDlg::OnBnClickedSetPwm()
{
  CString goal_pwm_str;
  GetDlgItem(IDC_PWM_EDIT)->GetWindowTextW(goal_pwm_str);
  goal_pwm_xm430_ = _ttoi(goal_pwm_str);
}

void CADOL_motor_testDlg::OnBnClickedDisconnect()
{
  //stop dynamixel
  goal_velocity_xm430_ = 0;
  goal_pwm_xm430_ = 0;
  goal_curr_xm430_ = 0;
  print_enable_ = false;
  Sleep(200);

  //terminate dynamixel and phidget.
  //before termination, terminating phidget should be first becuase terminating communication should be first.
  terminatePhidget();
  terminateCommDXL();
  
  //enable connect windows.
  connect_btn_.EnableWindow(true);
  comport_combo_.EnableWindow(true);
  baud_combo_.EnableWindow(true);
  ctrl_mode_combo_.EnableWindow(true);
}

void CADOL_motor_testDlg::OnBnClickedSetCurr()
{
  CString goal_curr_str;
  GetDlgItem(IDC_CURR_EDIT)->GetWindowTextW(goal_curr_str);
  goal_curr_xm430_ = _ttoi(goal_curr_str);
}

void CADOL_motor_testDlg::OnTimer(UINT_PTR nIDEvent)
{
  // TODO: Add your message handler code here and/or call default

  CDialogEx::OnTimer(nIDEvent);
}


BOOL CADOL_motor_testDlg::PreTranslateMessage(MSG* pMsg)
{
  // TODO: Add your specialized code here and/or call the base class
  if(pMsg->message == WM_KEYDOWN)
  {
    if(pMsg->wParam == VK_RETURN)
    {
      return false;
      //if(pMsg->hwnd == GetDlgItem(IDC_CALIB_FACOR1)->GetSafeHwnd())
      //{
      //  GetDlgItem(IDC_SET_CALIB)->SetFocus();
      //  return false;
      //}
      //else if(pMsg->hwnd == GetDlgItem(IDC_BAUD_COMBO)->GetSafeHwnd())
      //  return FALSE;
    }
  }
  return CDialogEx::PreTranslateMessage(pMsg);
}



