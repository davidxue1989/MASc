#pragma once
#include "afxwin.h"


// CPauseSystemDlg dialog

class CPauseSystemDlg : public CDialog
{
	DECLARE_DYNAMIC(CPauseSystemDlg)

public:
	CPauseSystemDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPauseSystemDlg();

// Dialog Data
	enum { IDD = IDD_PAUSESYSTEM_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	//afx_msg void OnBnClickedButton11();
	//afx_msg void OnBnClickedButton12();
	//afx_msg void OnBnClickedButton13();
	//afx_msg void OnBnClickedButton14();
	//afx_msg void OnBnClickedButton15();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	//afx_msg void OnBnClickedButton1();
	//CEdit reasonForPause;
	//CEdit updateToState;
	void Close();

	bool systemResume;

	//dx oct 28, 2012
	BOOL virtual OnInitDialog();
	BOOL virtual OnClose();
	CStatic *pTextCtrl_CurrentUserState;
	CFont m_font;
	//CString currentUserState;
	enum USER_STATE
	{
		BEGINNING,
		HANDS_WET,
		GOT_SOAP,
		DONE_SCRUBBING,
		DONE_RINSING,
		HANDS_DRIED,
		ALL_DONE_LEFT_THE_SINK
	} userState;
	afx_msg void OnBnClickedButton8();
};
