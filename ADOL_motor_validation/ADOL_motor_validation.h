
// ADOL_motor_validation.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CADOL_motor_validationApp:
// See ADOL_motor_validation.cpp for the implementation of this class
//

class CADOL_motor_validationApp : public CWinApp
{
public:
	CADOL_motor_validationApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CADOL_motor_validationApp theApp;