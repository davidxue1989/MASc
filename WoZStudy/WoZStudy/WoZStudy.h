
// WoZStudy.h : main header file for the WoZStudy application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

// CWoZStudyApp:
// See WoZStudy.cpp for the implementation of this class
//

class CWoZStudyApp : public CWinApp
{
public:
	CWoZStudyApp();

	//HACCEL m_haccel;
	//BOOL PreTranslateMessage(MSG* pMsg);

// Overrides
public:
	virtual BOOL InitInstance();

	HACCEL m_haccel;
	BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	CDocument *getDoc();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedButtonIntro();
	afx_msg void OnBnClickedButtonTurnonwater();
	afx_msg void OnBnClickedButtonWethands();
	afx_msg void OnBnClickedButtonSqueezeoutsoap();
	afx_msg void OnBnClickedButtonScrubhands();
	afx_msg void OnBnClickedButtonRinsehands();
	afx_msg void OnBnClickedButtonTurnoffwater();
	afx_msg void OnBnClickedButtonDryhands();
	afx_msg void OnBnClickedButtonAlldone();

	afx_msg void OnBnClickedButtonFirstag();
	afx_msg void OnBnClickedButtonReward();
	afx_msg void OnBnClickedButtonLetuscontinue();

	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnBnClickedButtonStop();
};

extern CWoZStudyApp theApp;
