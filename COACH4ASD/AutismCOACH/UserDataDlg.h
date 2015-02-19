#pragma once
#include <iostream>
#include <fstream>
using namespace std;


// CUserDataDlg dialog

class CUserDataDlg : public CDialog
{
	DECLARE_DYNAMIC(CUserDataDlg)

public:
	CUserDataDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUserDataDlg();

// Dialog Data
	enum { IDD = IDD_USERDATADIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString trialUserSettings;
	CString trialLogFile;
	ofstream mostrecentuserdata;
	CString participantName;
	CString trialDate;
	int trialNumber;
	CString trialStartTime;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CString calibrationFileName;
	afx_msg void OnBnClickedButtonBrowse();
};
