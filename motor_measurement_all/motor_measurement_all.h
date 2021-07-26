
// motor_measurement_all.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Cmotor_measurement_allApp:
// See motor_measurement_all.cpp for the implementation of this class
//

class Cmotor_measurement_allApp : public CWinApp
{
public:
	Cmotor_measurement_allApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Cmotor_measurement_allApp theApp;