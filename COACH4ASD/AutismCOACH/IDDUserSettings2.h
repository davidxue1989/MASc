#pragma once

// IDDUserSettings2 dialog

class IDDUserSettings2 : public CDialog
{
	DECLARE_DYNAMIC(IDDUserSettings2)

public:
	IDDUserSettings2(CWnd* pParent = NULL);   // standard constructor
	virtual ~IDDUserSettings2();

	int nPrompts;
	int firstPromptTime;
	int nextPromptTime;
	int priming;
	int use_attention_grabbers;
	int rewardallsteps;	

	CSpinButtonCtrl m_SpinQ2;
	CSpinButtonCtrl m_SpinQ3;
	CSpinButtonCtrl m_SpinQ4;

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_USERSETTINGS2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonPrev();
	afx_msg void OnBnClickedButtonNext();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton32();
	afx_msg void OnBnClickedButton33();
	afx_msg void OnBnClickedButton34();
	afx_msg void OnBnClickedButton35();
	afx_msg void OnBnClickedButton36();
	
	CFont font;
};
