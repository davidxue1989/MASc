#pragma once
#include "afxwin.h"
#include <iostream>
#include <fstream>
#include <string>
#include "afxcmn.h"
using namespace std;


// CUserSettingsDlg dialog

class CUserSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CUserSettingsDlg)

public:
	CUserSettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUserSettingsDlg();

// Dialog Data
	enum { IDD = IDD_USERSETTINGS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void SaveUserSettingsForTrial(CString filename); //Jan 12 //dx aug 18, 2012: I don't think we need to use this
	afx_msg void OnBnClickedOk();
	int startOver;
	int resoap;
	int useCountdown;
	int levelOnePrompt;
	int levelTwoPrompt;
	int levelThreePrompt;
	int levelFourPrompt;
	int levelFivePrompt;
	int priming;
	int rewardallsteps;
	int nPrompts;
	int firstPromptTime;
	int nextPromptTime;
	int scrubTime;
	int rinseTime;
	int soapTime; //dx oct 28, 2012
	//dx nov 09, 2012
	int wetTime;
	int dryTime;
	int volume;

	char songstr[256];
	//int prompttypes[6];
	int prompttypes[5]; //dx dec 04, 2012: don't know why they wrote prompttypes to have 6 elements

	int use_attention_grabbers;

//Operations
public:
	void LoadDefaultUserSettings();//April 20, 2010 - Justin
	void SaveUserSettings();
	
//Implementation
protected:
	ofstream userFile; //dx aug 18, 2012: not used?!
public:
	ofstream logFile_u; //dx aug 18, 2012: not used?!
public:
	bool alreadyentered_usersettings;
	CSpinButtonCtrl m_SpinQ6;
	CSpinButtonCtrl m_SpinQ9;
	CSpinButtonCtrl m_SpinQ10;
	CSpinButtonCtrl m_SpinQ11;
	CSpinButtonCtrl m_SpinQ12;
	virtual BOOL OnInitDialog();
	CString logFileName;//Jan 12
};