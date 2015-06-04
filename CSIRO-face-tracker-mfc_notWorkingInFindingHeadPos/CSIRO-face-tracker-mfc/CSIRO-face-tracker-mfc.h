
// CSIRO-face-tracker-mfc.h : main header file for the CSIRO-face-tracker-mfc application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CCSIROfacetrackermfcApp:
// See CSIRO-face-tracker-mfc.cpp for the implementation of this class
//

class CCSIROfacetrackermfcApp : public CWinApp
{
public:
	CCSIROfacetrackermfcApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CCSIROfacetrackermfcApp theApp;
