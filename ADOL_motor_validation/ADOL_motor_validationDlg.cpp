
// ADOL_motor_validationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ADOL_motor_validation.h"
#include "ADOL_motor_validationDlg.h"
#include "afxdialogex.h"

#include <iostream>
#include <fstream>
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEFAULT_CALIB_FACTOR1 (1048659.47918428)
#define DEFAULT_CALIB_FACTOR2 (-1838.691939)

CMutex g_mutex(FALSE, NULL);
void CALLBACK procArduinoCurrent(UINT m_nTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

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


// CADOL_motor_validationDlg dialog



CADOL_motor_validationDlg::CADOL_motor_validationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ADOL_MOTOR_VALIDATION_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CADOL_motor_validationDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_COMPORT_COMBO, comport_combo_);
  DDX_Control(pDX, IDC_BAUD_COMBO, baud_combo_);
  DDX_Control(pDX, IDC_CTRL_MODE_COMBO, driving_ctrl_mode_combo_);
  DDX_Control(pDX, IDC_CTRL_MODE_COMBO2, test_ctrl_mode_combo_);
  DDX_Control(pDX, IDC_CHANNEL, phidget_channel_combo_);
  DDX_Control(pDX, IDC_ARDUINO_PORT_COMBO, arduino_combo_);
}

BEGIN_MESSAGE_MAP(CADOL_motor_validationDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CADOL_motor_validationDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_STOP, &CADOL_motor_validationDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_CLEAR, &CADOL_motor_validationDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDC_SAVE, &CADOL_motor_validationDlg::OnBnClickedSave)
	ON_BN_CLICKED(IDC_SET_CALIB, &CADOL_motor_validationDlg::OnBnClickedSetCalib)
	ON_BN_CLICKED(IDOK2, &CADOL_motor_validationDlg::OnBnClickedOk2)
	ON_BN_CLICKED(IDC_CONNECT, &CADOL_motor_validationDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_DISCONNECT, &CADOL_motor_validationDlg::OnBnClickedDisconnect)
	ON_BN_CLICKED(IDC_CTRL_START, &CADOL_motor_validationDlg::OnBnClickedCtrlStart)
END_MESSAGE_MAP()


// CADOL_motor_validationDlg message handlers

BOOL CADOL_motor_validationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
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
  for (int port_num = 1; port_num < 253; port_num++)
  {
    CString port_string;
    port_string.Format(L"COM%d", port_num);

    comport_combo_.AddString(port_string);
    arduino_combo_.AddString(port_string);
  }

  //Initianlizing baud combo
  baud_combo_.AddString(L"57600");
  baud_combo_.AddString(L"1000000");

  //Initializing control mode combo
  driving_ctrl_mode_combo_.AddString(L" 0: Current");
  driving_ctrl_mode_combo_.AddString(L" 1: Velcoity");
  driving_ctrl_mode_combo_.AddString(L" 3: Position");
  driving_ctrl_mode_combo_.AddString(L"16: PWM");

  test_ctrl_mode_combo_.AddString(L" 0: Invalid");
  test_ctrl_mode_combo_.AddString(L" 1: Velcoity");
  test_ctrl_mode_combo_.AddString(L" 3: Position");
  test_ctrl_mode_combo_.AddString(L"16: PWM");
  //test_ctrl_mode_combo_.AddString(L"100: torque off");

  phidget_channel_combo_.AddString(L"CH 0");
  phidget_channel_combo_.AddString(L"CH 1");
  phidget_channel_combo_.AddString(L"CH 2");
  phidget_channel_combo_.AddString(L"CH 3");

  map_idx_to_ctrl_mode_[0] = 0;
  map_idx_to_ctrl_mode_[1] = 1;
  map_idx_to_ctrl_mode_[2] = 3;
  map_idx_to_ctrl_mode_[3] = 16;

  //disable disconnect btn
  GetDlgItem(IDC_DISCONNECT)->EnableWindow(false);

  //Initializing dxl paramater
  dxl_port_ = 0;
  driving_dxl_ctrl_mode_ = 1;
  test_dxl_ctrl_mode_ = 1;

  dxl_bulk_read_ = 0;

  dxl_id_test_ = 1;
  dxl_id_driving_ = 2;

  goal_velocity_xm430_.i32_value = 0;
  goal_pwm_xm430_.i16_value = 0;
  goal_curr_xm430_.i16_value = 0;

  goal_velocity_mx28_.i32_value = 0;
  goal_pwm_mx28_.i16_value = 0;

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

  // cmd print enable
  print_enable_ = false;

  // initialize load cell out put variables 
  errorDetailLen_ = 100;
  voltage_output_ = 0;
  measured_weight_g_ = 0;

  // load cell calibration factor
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
    calib_factor1_ = DEFAULT_CALIB_FACTOR1;
    calib_factor2_ = DEFAULT_CALIB_FACTOR2;
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
    calib_factor1_ = calib_factor;

    calib_file_handler >> a;
    calib_file_handler >> calib_factor;
    std::cout << "calib_factor2: " << calib_factor << std::endl;
    calib_factor2_str.Format(_T("%.8f"), calib_factor);
    calib_factor2_ = calib_factor;
  }

  calib_file_handler.close();

  GetDlgItem(IDC_CALIB_FACOR1)->SetWindowTextW(calib_factor1_str);
  GetDlgItem(IDC_CALIB_FACOR2)->SetWindowTextW(calib_factor2_str);

  comm_ = false;

  ////chart related.
  //m_extBgColor = getDefaultBgColor();
  //drawChart(&force_chart_);
  //chart_drawing_ = false;


  data_idx_ = 0;

  ctrl_flag_ = false;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CADOL_motor_validationDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
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

void CADOL_motor_validationDlg::OnPaint()
{
	if (IsIconic())
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
HCURSOR CADOL_motor_validationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool CADOL_motor_validationDlg::initializeCommDXL(void)
{
  CString port_string;
  this->comport_combo_.GetLBText(this->comport_combo_.GetCurSel(), port_string);

  CString baud_string;
  this->baud_combo_.GetLBText(this->baud_combo_.GetCurSel(), baud_string);

  dxl_port_ = dynamixel::PortHandler::getPortHandler((CStringA)port_string);
  dxl_packet_ = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

  int dxl_comm_result = COMM_TX_FAIL;
  uint8_t dxl_error = 0;

  if (dxl_port_->openPort())
  {
    printf("Succeeded in opening the port\n");
  }
  else
  {
    printf("Failed to open the port\n");
    return false;
  }

  if (dxl_port_->setBaudRate(_ttoi(baud_string)))
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

void CADOL_motor_validationDlg::terminateCommDXL(void)
{
  if (dxl_port_ != 0)
  {
    dxl_port_->closePort();
    dxl_port_ = 0;
  }

  if (dxl_bulk_read_ != 0)
  {
    dxl_bulk_read_->clearParam();
    delete dxl_bulk_read_;
    dxl_bulk_read_ = 0;
  }
}

bool CADOL_motor_validationDlg::initializeDXLParam(void)
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
  int dxl_result = dxl_packet_->writeTxRx(dxl_port_, dxl_id_driving_, XM430::ADDR_INDIRECT_ADDR_1, num_indirect_addr_set, indirect_addr_array, &dxl_error);
  if (dxl_result == COMM_SUCCESS)
  {
    printf("Succeeded in changing Indirect Address\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to change Indirect Address\n");
    return false;
  }

  if (dxl_error != 0)
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
  if (dxl_result == COMM_SUCCESS)
  {
    printf("Succeeded in changing Indirect Address of MX28\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to change Indirect Address of MX28\n");
    return false;
  }

  if (dxl_error != 0)
    printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));


  // change operating mode of driving motor
  driving_dxl_ctrl_mode_ = map_idx_to_ctrl_mode_[driving_ctrl_mode_combo_.GetCurSel()];
  dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_id_driving_, XM430::ADDR_OPERATING_MODE, driving_dxl_ctrl_mode_, &dxl_error);
  if (dxl_result == COMM_SUCCESS)
  {
    printf("Succeeded in changing the Operating Mode of the driving\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to change the Operating Mode of the driving\n");
    return false;
  }

  // change operating mode of test motor
  test_dxl_ctrl_mode_ = map_idx_to_ctrl_mode_[test_ctrl_mode_combo_.GetCurSel()];
  dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_id_test_, MX28::ADDR_OPERATING_MODE, test_dxl_ctrl_mode_, &dxl_error);
  if (dxl_result == COMM_SUCCESS)
  {
    printf("Succeeded in changing the Operating Mode of the test\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to change the Operating Mode of the test\n");
    return false;
  }

  //Initialize dxl_bulk_read;
  dxl_bulk_read_ = new dynamixel::GroupBulkRead(dxl_port_, dxl_packet_);
  dxl_bulk_read_->addParam(dxl_id_test_, XM430::ADDR_INDIRECT_DATA_1, 9);
  dxl_bulk_read_->addParam(dxl_id_driving_, MX28::ADDR_INDIRECT_DATA_1, 11);

  // Initialize dxl_bulk_write
  dxl_bulk_write_ = new dynamixel::GroupBulkWrite(dxl_port_, dxl_packet_);

  if (test_dxl_ctrl_mode_ == 1)
    dxl_bulk_write_->addParam(dxl_id_test_, MX28::ADDR_GOAL_VELOCITY, 4, goal_velocity_mx28_.bytes);
  else if (test_dxl_ctrl_mode_== 16)
    dxl_bulk_write_->addParam(dxl_id_test_, MX28::ADDR_GOAL_PWM, 2, goal_pwm_mx28_.bytes);
  else
  {
    printf("Failed to initialize bulk write : the invalid control mode of the test");
    return false;
  }

  if (driving_dxl_ctrl_mode_ == 0)
    dxl_bulk_write_->addParam(dxl_id_driving_, XM430::ADDR_GOAL_CURRENT, 2, goal_curr_xm430_.bytes);
  else if (driving_dxl_ctrl_mode_ == 1)
    dxl_bulk_write_->addParam(dxl_id_driving_, XM430::ADDR_GOAL_VELOCITY, 4, goal_velocity_xm430_.bytes);
  else if (driving_dxl_ctrl_mode_ == 16)
    dxl_bulk_write_->addParam(dxl_id_driving_, XM430::ADDR_GOAL_PWM, 2, goal_pwm_xm430_.bytes);
  else
  {
    printf("Failed to initialize bulk write : the invalid control mode of the driving");
    return false;
  }

  return true;
}

bool CADOL_motor_validationDlg::turnTorqueOnDXL(bool on_off)
{
  uint8_t dxl_error = 0;
  int dxl_result = 0;
  if (on_off)
    dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_id_driving_, XM430::ADDR_TORQUE_ENABLE, 1, &dxl_error);
  else
    dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_id_driving_, XM430::ADDR_TORQUE_ENABLE, 0, &dxl_error);

  if (dxl_error == COMM_SUCCESS)
  {
    printf("Succeeded in turning on the torque of the driving\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to turn on the torqueof the driving\n");
    return false;
  }

  if (dxl_error != 0)
    printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));

  if (on_off)
    dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_id_test_, MX28::ADDR_TORQUE_ENABLE, 0, &dxl_error);
  else
    dxl_result = dxl_packet_->write1ByteTxRx(dxl_port_, dxl_id_test_, MX28::ADDR_TORQUE_ENABLE, 0, &dxl_error);

  if (dxl_error == COMM_SUCCESS)
  {
    printf("Succeeded in turning on the torque of the test\n");
    Sleep(200);
  }
  else
  {
    printf("Failed to turn on the torqueof the test\n");
    return false;
  }

  if (dxl_error != 0)
    printf("%s\n", dxl_packet_->getRxPacketError(dxl_error));

  return true;
}

void CADOL_motor_validationDlg::changeGoalValues(void)
{
  int dxl_result = 0;
  
  if (test_dxl_ctrl_mode_ == 1)
    dxl_bulk_write_->changeParam(dxl_id_test_, MX28::ADDR_GOAL_VELOCITY, 4, goal_velocity_mx28_.bytes);
  else if (test_dxl_ctrl_mode_ == 16)
    dxl_bulk_write_->changeParam(dxl_id_test_, MX28::ADDR_GOAL_PWM, 2, goal_pwm_mx28_.bytes);

  if (driving_dxl_ctrl_mode_ == 0)
    dxl_bulk_write_->changeParam(dxl_id_driving_, XM430::ADDR_GOAL_CURRENT, 2, goal_curr_xm430_.bytes);
  else if (driving_dxl_ctrl_mode_ == 1)
    dxl_bulk_write_->changeParam(dxl_id_driving_, XM430::ADDR_GOAL_VELOCITY, 4, goal_velocity_xm430_.bytes);
  else if (driving_dxl_ctrl_mode_ == 16)
    dxl_bulk_write_->changeParam(dxl_id_driving_, XM430::ADDR_GOAL_PWM, 2, goal_pwm_xm430_.bytes);

  dxl_result = dxl_bulk_write_->txPacket();

  if (dxl_result != COMM_SUCCESS)
  {
    printf("%s\n", dxl_packet_->getTxRxResult(dxl_result));
    return;
  }
}

bool CADOL_motor_validationDlg::readValuesFromDXLsTx(void)
{
  int dxl_result = dxl_bulk_read_->txPacket();
  if (dxl_result != COMM_SUCCESS)
  {
    printf("%s\n", dxl_packet_->getTxRxResult(dxl_result));
    return false;
  }
  return true;
}

bool CADOL_motor_validationDlg::readValuesFromDXLsRx(void)
{
  int dxl_result = dxl_bulk_read_->rxPacket();
  if (dxl_result != COMM_SUCCESS)
  {
    printf("%s\n", dxl_packet_->getTxRxResult(dxl_result));
    return false;
  }

  present_current_xm430_ = dxl_bulk_read_->getData(dxl_id_driving_, XM430::ADDR_INDIRECT_DATA_1 + 0, 2);
  present_velocity_xm430_ = dxl_bulk_read_->getData(dxl_id_driving_, XM430::ADDR_INDIRECT_DATA_1 + 2, 4);
  present_position_xm430_ = dxl_bulk_read_->getData(dxl_id_driving_, XM430::ADDR_INDIRECT_DATA_1 + 6, 4);
  present_temperature_xm430_ = dxl_bulk_read_->getData(dxl_id_driving_, XM430::ADDR_INDIRECT_DATA_1 + 10, 1);

  present_velocity_MX28_ = dxl_bulk_read_->getData(dxl_id_test_, MX28::ADDR_INDIRECT_DATA_1 + 0, 4);
  present_position_MX28_ = dxl_bulk_read_->getData(dxl_id_test_, MX28::ADDR_INDIRECT_DATA_1 + 4, 4);
  present_temperature_MX28_ = dxl_bulk_read_->getData(dxl_id_test_, MX28::ADDR_INDIRECT_DATA_1 + 8, 1);

  return true;
}

//For Phidget
void __stdcall onVoltageRatioInput0_VoltageRatioChange(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio) {

  CADOL_motor_validationDlg* test_dlg = (CADOL_motor_validationDlg*)ctx;

  if (test_dlg->comm_ == false)
    return;

  QueryPerformanceCounter(&(test_dlg->Endtime));
  uint64_t elapsed = test_dlg->Endtime.QuadPart - test_dlg->BeginTime.QuadPart;
  test_dlg->elapsed_time = (double)elapsed / (double)test_dlg->Frequency.QuadPart;

  test_dlg->updateGoalValues();
  test_dlg->changeGoalValues();
  test_dlg->readValuesFromDXLsTx();

  //test_dlg->arduino_.txRxPacket(); // excute this in the multimedia timer
  float curr = test_dlg->arduino_.getCurrent();

  test_dlg->readValuesFromDXLsRx();

  test_dlg->voltage_output_ = voltageRatio;
  test_dlg->measured_weight_g_ = voltageRatio*test_dlg->calib_factor1_ + test_dlg->calib_factor2_;


  if (test_dlg->print_enable_ == true)
    printf("t: %lf vol*10^8: %lf f: %lf gc : %d cv: %d cp: %d ct: %d cc: %d mcv: %d mcp: %d mct: %d curr: %f\n",
      test_dlg->elapsed_time,
      voltageRatio*100000000.0,
      test_dlg->measured_weight_g_,
      test_dlg->goal_curr_xm430_.i16_value,
      test_dlg->present_velocity_xm430_,
      test_dlg->present_position_xm430_,
      test_dlg->present_temperature_xm430_,
      test_dlg->present_current_xm430_,
      test_dlg->present_velocity_MX28_,
      test_dlg->present_position_MX28_,
      test_dlg->present_temperature_MX28_,
      curr);

  if (test_dlg->print_enable_ == true)
  {
    test_dlg->arr_elapsed_time_.push_back(test_dlg->elapsed_time);
    test_dlg->arr_voltage_output_.push_back(test_dlg->voltage_output_);
    test_dlg->arr_measured_weight_g_.push_back(test_dlg->measured_weight_g_);
    test_dlg->arr_goal_velocity_xm430_.push_back(test_dlg->goal_velocity_xm430_.i32_value);
    test_dlg->arr_goal_pwm_xm430_.push_back(test_dlg->goal_pwm_xm430_.i16_value);
    test_dlg->arr_goal_curr_xm430_.push_back(test_dlg->goal_curr_xm430_.i16_value);
    test_dlg->arr_present_temperature_xm430_.push_back(test_dlg->present_temperature_xm430_);
    test_dlg->arr_present_current_xm430_.push_back(test_dlg->present_current_xm430_);
    test_dlg->arr_present_velocity_xm430_.push_back(test_dlg->present_velocity_xm430_);
    test_dlg->arr_present_position_xm430_.push_back(test_dlg->present_position_xm430_);
    test_dlg->arr_present_temperature_MX28_.push_back(test_dlg->present_temperature_MX28_);
    test_dlg->arr_present_velocity_MX28_.push_back(test_dlg->present_velocity_MX28_);
    test_dlg->arr_present_position_MX28_.push_back(test_dlg->present_position_MX28_);
    test_dlg->arr_arduino_curr_.push_back(curr);

    g_mutex.Lock();
    test_dlg->time_stamps.push_back(test_dlg->time_stamps[test_dlg->time_stamps.size() - 1] + test_dlg->elapsed_time);
    test_dlg->scailed_force_raw_.push_back(test_dlg->measured_weight_g_);
    g_mutex.Unlock();
  }
}

void CADOL_motor_validationDlg::updateGoalValues(void)
{
  if (ctrl_flag_ == false)
    return;

  data_idx_++;

  if (data_idx_ >= goal_vel_xm430_list_.size())
    data_idx_ = 0;

  goal_velocity_xm430_.i32_value = goal_vel_xm430_list_[data_idx_];
  goal_pwm_mx28_.i16_value = goal_pwm_mx28_list_[data_idx_];

}

void CADOL_motor_validationDlg::loadData(void)
{
  std::ifstream file;
  file.open("traj.txt");

  goal_vel_xm430_list_.clear();
  goal_pwm_mx28_list_.clear();

  std::string srt;

  while (!file.eof())
  {
    int32_t a; int16_t b;
    std::getline(file, srt);
    std::stringstream ss(srt);

    
    file >> a;
    file >> b;
    goal_vel_xm430_list_.push_back(a);
    goal_pwm_mx28_list_.push_back(b);
  }
  file.close();
  
  data_idx_ = 0;
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

bool CADOL_motor_validationDlg::initializePhidget(void)
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
  if (ret_ != EPHIDGET_OK) {
    Phidget_getLastError(&errorCode_, &errorString_, errorDetail_, &errorDetailLen_);
    printf("Error (%d): %s", errorCode_, errorString_);
    return false;
  }

  QueryPerformanceCounter(&BeginTime);
  //Set the Data Interval of the Device to 8 ms
  ret_ = Phidget_setDataInterval((PhidgetHandle)voltageRatioInput0_, 8);
  if (ret_ != EPHIDGET_OK) {
    Phidget_getLastError(&errorCode_, &errorString_, errorDetail_, &errorDetailLen_);
    printf("Error (%d): %s", errorCode_, errorString_);
    return false;
  }
  comm_ = true;
  return true;
}

bool CADOL_motor_validationDlg::terminatePhidget(void)
{
  comm_ = false;
  Sleep(16);
  ret_ = Phidget_close((PhidgetHandle)voltageRatioInput0_);
  if (ret_ != EPHIDGET_OK) {
    Phidget_getLastError(&errorCode_, &errorString_, errorDetail_, &errorDetailLen_);
    printf("Error (%d): %s", errorCode_, errorString_);
    //exit(1);
  }
  PhidgetVoltageRatioInput_delete(&voltageRatioInput0_);

  return true;
}


void CADOL_motor_validationDlg::OnBnClickedConnect()
{
  // TODO: Add your control notification handler code here
  if (initializeCommDXL() == false)
  {
    AfxMessageBox(L"Failed to connect to USB2DXL");
    return;
  }

  if (turnTorqueOnDXL(false) == false)
  {
    AfxMessageBox(L"Failed to initialize dxl param");
    terminateCommDXL();
    return;
  }

  if (initializeDXLParam() == false)
  {
    AfxMessageBox(L"Failed to initialize dxl param");
    terminateCommDXL();
    return;
  }

  if (turnTorqueOnDXL(true) == false) // to test, torque off
  {
    AfxMessageBox(L"Failed to initialize dxl param");
    terminateCommDXL();
    return;
  }

  if (initializePhidget() == false)
  {
    AfxMessageBox(L"Failed to connect to Phiget");
    terminateCommDXL();
    terminatePhidget();
    return;
  }

  CString port_string;
  this->arduino_combo_.GetLBText(this->arduino_combo_.GetCurSel(), port_string);
  std::string port = std::string(CT2CA(port_string));
  if (arduino_.connect(port, 1000000) != true)
  {
    AfxMessageBox(L"Failed to connect to Arduino");
    return;
  }

  loadData();

  // for timer
  // gettting timer resolution
  TIMECAPS timecaps;
  timeGetDevCaps(&timecaps, sizeof(TIMECAPS));

  std::cout << " timer resoultion : " << timecaps.wPeriodMin << " " << timecaps.wPeriodMax << std::endl;

  m_nTimerID = timeSetEvent(4, timecaps.wPeriodMin, procArduinoCurrent, (DWORD_PTR)this, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
  if (m_nTimerID == 0)
  {
    AfxMessageBox(L"Failed to connect to Arduino");
    return;
  }
  else
  {
    std::cout << "Succeeded in creating timer for arduino" << std::endl;
  }

  //connect_btn_.EnableWindow(false);
  //comport_combo_.EnableWindow(false);
  //baud_combo_.EnableWindow(false);
  //ctrl_mode_combo_.EnableWindow(false);
  //phidget_channel_combo_.EnableWindow(false);
}


void CADOL_motor_validationDlg::OnBnClickedDisconnect()
{
  // TODO: Add your control notification handler code here
  //stop dynamixel
  goal_velocity_xm430_.i32_value = 0;
  goal_pwm_xm430_.i16_value = 0;
  goal_curr_xm430_.i16_value = 0;
  print_enable_ = false;
  Sleep(200);

  //terminate dynamixel and phidget.
  //before termination, terminating phidget should be first becuase terminating communication should be first.
  terminatePhidget();
  terminateCommDXL();

  //enable connect windows.
  //connect_btn_.EnableWindow(true);
  //comport_combo_.EnableWindow(true);
  //baud_combo_.EnableWindow(true);
  //ctrl_mode_combo_.EnableWindow(true);
}

void CADOL_motor_validationDlg::OnBnClickedStart()
{
	// TODO: Add your control notification handler code here
  arr_elapsed_time_.clear();
  arr_voltage_output_.clear();
  arr_measured_weight_g_.clear();
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
  arr_arduino_curr_.clear();

  time_stamps.clear();
  time_stamps.push_back(0);
  scailed_force_raw_.clear();

  print_enable_ = true;
  //chart_drawing_ = true;

  //SetTimer(DRAWING_TIMER, 100, NULL);
}


void CADOL_motor_validationDlg::OnBnClickedStop()
{
  //chart_drawing_ = false;
  print_enable_ = false;
  //KillTimer(DRAWING_TIMER);
}


void CADOL_motor_validationDlg::OnBnClickedClear()
{
  system("cls");
  bool curr_print = print_enable_;
  //bool curr_drawing = chart_drawing_;
  print_enable_ = false;
  //chart_drawing_ = false;

  arr_elapsed_time_.clear();
  arr_voltage_output_.clear();
  arr_measured_weight_g_.clear();
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
  arr_arduino_curr_.clear();

  time_stamps.clear();
  time_stamps.push_back(0);
  scailed_force_raw_.clear();

  //drawChart(&force_chart_);

  print_enable_ = curr_print;
  //chart_drawing_ = curr_drawing;
}


void CADOL_motor_validationDlg::OnBnClickedSave()
{
  ctrl_flag_ = false;
  print_enable_ = false;
  Sleep(8);

  std::ofstream log_file;
  CTime cTime = CTime::GetCurrentTime(); // get current date and time
  CString file_path;

    file_path.Format(_T("log_val_%04d%02d%02d%02d%02d%02d.txt"),
      cTime.GetYear(), cTime.GetMonth(),
      cTime.GetDay(), cTime.GetHour(),
      cTime.GetMinute(),
      cTime.GetSecond());


  //file_path.Format(_T("log_vel%d_pwm%d_cur%d_%04d%02d%02d%02d%02d%02d.txt"),
  //  arr_goal_velocity_xm430_[0], arr_goal_pwm_xm430_[0], arr_goal_curr_xm430_[0],
  //  cTime.GetYear(), cTime.GetMonth(),
  //  cTime.GetDay(), cTime.GetHour(),
  //  cTime.GetMinute(),
  //  cTime.GetSecond());

  log_file.open(file_path);
  log_file << std::fixed;
  log_file.precision(10);
  log_file << "time" << "\t" << "voltage_output" << "\t" << "measured_load" << "\t"
    << "des_driving_vel" << "\t" << "des_driving_pwm" << "\t" << "des_driving_curr" << "\t" << "des_driving_temp" << "\t"
    << "mes_driving_curr" << "\t" << "mes_driving_vel" << "\t" << "mes_driving_pos" << "\t"
    << "mes_test_temp" << "\t" << "mes_test_vel" << "\t" << "mes_test_pos" << "\t" << "mes_test_curr" << std::endl;

  for (unsigned int arr_idx = 0; arr_idx < arr_elapsed_time_.size(); arr_idx++)
  {

    log_file << arr_elapsed_time_[arr_idx] << "\t"
      << arr_voltage_output_[arr_idx] << "\t"
      << arr_measured_weight_g_[arr_idx] << "\t"
      << arr_goal_velocity_xm430_[arr_idx] << "\t"
      << arr_goal_pwm_xm430_[arr_idx] << "\t"
      << arr_goal_curr_xm430_[arr_idx] << "\t"
      << (unsigned int)arr_present_temperature_xm430_[arr_idx] << "\t"
      << arr_present_current_xm430_[arr_idx] << "\t"
      << arr_present_velocity_xm430_[arr_idx] << "\t"
      << arr_present_position_xm430_[arr_idx] << "\t"
      << (unsigned int)arr_present_temperature_MX28_[arr_idx] << "\t"
      << arr_present_velocity_MX28_[arr_idx] << "\t"
      << arr_present_position_MX28_[arr_idx] << "\t" 
      << arr_arduino_curr_[arr_idx]
      << std::endl;
  }

  log_file.close();

  arr_elapsed_time_.clear();
  arr_voltage_output_.clear();
  arr_measured_weight_g_.clear();
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
  arr_arduino_curr_.clear();
}


void CADOL_motor_validationDlg::OnBnClickedSetCalib()
{
	// TODO: Add your control notification handler code here
}


void CADOL_motor_validationDlg::OnBnClickedOk2()
{
	// TODO: Add your control notification handler code here
}


void CADOL_motor_validationDlg::OnBnClickedCtrlStart()
{
	// TODO: Add your control notification handler code here
  ctrl_flag_ = true;
  //print_enable_ = true;
}


void CALLBACK procArduinoCurrent(UINT m_nTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
  CADOL_motor_validationDlg* dlg = (CADOL_motor_validationDlg*)dwUser;
  dlg->arduino_.txRxPacket();
}