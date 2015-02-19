#pragma once


// PrematureStopDlg dialog

class PrematureStopDlg : public CDialog
{
	DECLARE_DYNAMIC(PrematureStopDlg)

public:
	PrematureStopDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~PrematureStopDlg();

// Dialog Data
	enum { IDD = IDD_PREMATURESTOP_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int reason;
	afx_msg void OnBnClickedButtonSubmit();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	//dx oct 28, 2012
	enum REASON_PREMATURE
	{	// object regions
		SYSTEM_MALFUNCTIONING = 0,
		CHILD_NEEDS_ASSISTANCE,
		CHILD_IN_DISTRESS,
		CHILD_IS_NOT_RESPONDING,
		EMERGENCY,
		OTHERS_IMPORTANT,
		OTHERS_NOT_IMPORTANT
	} reasonPremature;

};
