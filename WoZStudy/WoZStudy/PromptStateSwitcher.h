#pragma once


// PromptStateSwitcher dialog


class PromptStateSwitcher : public CDialogEx
{
	DECLARE_DYNAMIC(PromptStateSwitcher)

	enum SWITCHABLESTATES {
		FirstPrompt = 0,
		FirstAG = 1,
		SecondAG = 2,
		SecondPrompt = 3,
		Reward = 4,
		LetUsContinue = 5,
		Other = 6,
	};

public:
	PromptStateSwitcher(CWnd* pParent = NULL);   // standard constructor
	virtual ~PromptStateSwitcher();

	void start();
	//void stop();
	void updateDisplay();

	// Dialog Data
	enum { IDD = IDD_PROMPT_STATE };
	SWITCHABLESTATES currentState;
	bool stateChanged;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonFirstprompt();
	afx_msg void OnBnClickedButtonFirstag();
	afx_msg void OnBnClickedButtonSecondag();
	afx_msg void OnBnClickedButtonSecondprompt();
	afx_msg void OnBnClickedButtonReward();
	afx_msg void OnBnClickedButtonLetuscontinue();
};
