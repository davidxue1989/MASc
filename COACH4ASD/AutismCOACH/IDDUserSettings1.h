#pragma once
#include <deque>

// IDDUserSettings1 dialog

class IDDUserSettings1 : public CDialog
{
	DECLARE_DYNAMIC(IDDUserSettings1)

public:
	IDDUserSettings1(CWnd* pParent = NULL);   // standard constructor
	virtual ~IDDUserSettings1();
	
	int levelOnePrompt;
	int levelTwoPrompt;
	int levelThreePrompt;
	int levelFourPrompt;
	int levelFivePrompt;

	int nPromptsFromSettings2;

	virtual BOOL OnInitDialog();
	virtual BOOL OnClose();

// Dialog Data
	enum { IDD = IDD_USERSETTINGS1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonPrev();
	afx_msg void OnBnClickedButtonNext();
	CFont font;
	
	afx_msg void OnBnClickedRadio2();
	CFont *pfont_notBold;
	CFont *pfont_Bold;
	afx_msg void OnBnClickedRadio3();

	void setItemsBold(int IDs[], int nIDs, bool Bolded[], int nBolded);
	afx_msg void OnBnClickedRadio4();
	afx_msg void OnBnClickedRadio5();
	afx_msg void OnBnClickedRadio6();
	afx_msg void OnBnClickedRadio7();
	afx_msg void OnBnClickedRadio8();
	afx_msg void OnBnClickedRadio13();
	afx_msg void OnBnClickedRadio14();
	afx_msg void OnBnClickedRadio15();
	afx_msg void OnBnClickedRadio16();
	afx_msg void OnBnClickedRadio17();
	afx_msg void OnBnClickedRadio18();
	afx_msg void OnBnClickedRadio50();
	afx_msg void OnBnClickedRadio51();
	afx_msg void OnBnClickedRadio52();
	afx_msg void OnBnClickedRadio53();
	afx_msg void OnBnClickedRadio54();
	afx_msg void OnBnClickedRadio55();
	afx_msg void OnBnClickedRadio56();
	afx_msg void OnBnClickedRadio57();
	afx_msg void OnBnClickedRadio58();
	afx_msg void OnBnClickedRadio59();
	afx_msg void OnBnClickedRadio60();
	afx_msg void OnBnClickedRadio61();
};
