
// NAOAGDemo.h : main header file for the NAOAGDemo application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CNAOAGDemoApp:
// See NAOAGDemo.cpp for the implementation of this class
//

class CNAOAGDemoApp : public CWinApp
{
public:
	CNAOAGDemoApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CNAOAGDemoApp theApp;
