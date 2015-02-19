#pragma once
#include "afxwin.h"

#include <iostream>
#include <fstream>
using namespace std;

// CHandPositionsDlg dialog

class CHandPositionsDlg : public CDialog
{
	DECLARE_DYNAMIC(CHandPositionsDlg)

public:
	CHandPositionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHandPositionsDlg();

// Dialog Data
	enum { IDD = IDD_HANDPOSITIONS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	ofstream logFile_h;

public:
	
	int buttoninput;

	CEdit leftHandPos;
	CEdit rightHandPos;
	CEdit waterAlert;
	afx_msg void OnBnClicked_away_left();
	afx_msg void OnBnClickedButton_away_right();
	afx_msg void OnBnClickedButton_sink_left();
	afx_msg void OnBnClickedButton_sink_right();
	afx_msg void OnBnClickedButton_water_left();
	afx_msg void OnBnClickedButton_water_right();
	afx_msg void OnBnClickedButton_tap_left();
	afx_msg void OnBnClickedButton_tap_right();
	afx_msg void OnBnClickedButton_soap_left();
	afx_msg void OnBnClickedButton_soap_right();
	afx_msg void OnBnClickedButton_towel_left();
	afx_msg void OnBnClickedButton_towel_right();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	short GetButtonInput();
	CEdit caregiverStatus;
	afx_msg void OnBnClickedButton18();
	afx_msg void OnBnClickedButton19();
	afx_msg void OnBnClickedButton20();
	afx_msg void OnBnClickedButton21();
	afx_msg void OnBnClickedButton22();
	CEdit updatedState;
	afx_msg void StopPromptingMusic();
	afx_msg void PausePromptingMusic();
	afx_msg void ResumePromptingMusic();
	//afx_msg void RestartPromptingMusic();//Jan 28
	afx_msg void OnBnClickedButtonWaterOn();
	CEdit waterStatus;
	afx_msg void OnBnClickedButtonWaterOff();
	afx_msg void OnBnClickedButton29();
	afx_msg void OnBnClickedButton30();
	afx_msg void OnBnClickedButton31();
	CEdit ncctowel;
	CEdit nccsoap;
	CEdit ncctap;
	CEdit nccwater;
	CEdit nccsink;
};
