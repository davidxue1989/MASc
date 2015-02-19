#pragma once
#include "IDDUserSettings1.h"
#include "IDDUserSettings2.h"


// IDDUserSettings3 dialog

class IDDUserSettings3 : public CDialog
{
	DECLARE_DYNAMIC(IDDUserSettings3)

public:
	IDDUserSettings3(CWnd* pParent = NULL);   // standard constructor
	virtual ~IDDUserSettings3();

	int useCountdown;
	int scrubTime;
	int rinseTime;

	int soapTime; //dx oct 28, 2012
	//dx nov 09, 2012
	int wetTime;
	int dryTime;
	
	int volume;

	//CSpinButtonCtrl m_SpinQ8b;
	//CSpinButtonCtrl m_SpinQ8c;

	virtual BOOL OnInitDialog();
	BOOL virtual OnClose();

// Dialog Data
	enum { IDD = IDD_USERSETTINGS3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonPrev();
	afx_msg void OnBnClickedButtonNext();
	afx_msg void OnBnClickedButton33();
	afx_msg void OnBnClickedButton34();
	afx_msg void OnBnClickedButton35();
	afx_msg void OnBnClickedButton36();
	
	CFont font;
	afx_msg void OnBnClickedButton37();
	afx_msg void OnBnClickedButton38();

	//dx oct 30, 2012:
	CSliderCtrl m_SliderCtrl;
	afx_msg void OnBnClickedButton39();
	afx_msg void OnBnClickedButton40();
	afx_msg void OnBnClickedButton41();
	afx_msg void OnBnClickedButton42();
	
	afx_msg void OnSliderChangeMouseRelease(NMHDR *pNMHDR, LRESULT *pResult);
	bool ChangeVolume(double nVolume,bool bScalar);	
	CStatic *pVolumeText;
	CFont m_font;
	CString convertVolume2StaticText();
	afx_msg void OnBnClickedButton1();

	IDDUserSettings1 *pSettingDlg1;
	IDDUserSettings2 *pSettingDlg2;
};
