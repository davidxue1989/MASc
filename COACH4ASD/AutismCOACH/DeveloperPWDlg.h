#pragma once


// CDeveloperPWDlg dialog

class CDeveloperPWDlg : public CDialog
{
	DECLARE_DYNAMIC(CDeveloperPWDlg)

public:
	CDeveloperPWDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDeveloperPWDlg();
	CString password;

// Dialog Data
	enum { IDD = IDD_DIALOG_DEVLOPER_PW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
