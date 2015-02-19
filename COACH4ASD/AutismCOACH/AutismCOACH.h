// AutismCOACH.h : main header file for the AutismCOACH application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CAutismCOACHApp:
// See AutismCOACH.cpp for the implementation of this class
//

class CAutismCOACHApp : public CWinApp
{
public:
	CAutismCOACHApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CAutismCOACHApp theApp;