#pragma once
#include <string>
// TaskProgress dialog

enum USERTASK {
	SystemPause = 0,
	Intro = 1,
	TurnOnWater = 2,
	WetYourHands = 3,
	GetSomeSoap = 4,
	ScrubYourHands = 5,
	RinseYourHands = 6,
	TurnOffWater = 7,
	DryYourHands = 8,
	AllDone = 9,
	EmergencyStop = 110,
	ImmediatePause = 111
};

class TaskProgress : public CDialogEx
{
	DECLARE_DYNAMIC(TaskProgress)

public:
	TaskProgress(CWnd* pParent = NULL);   // standard constructor
	virtual ~TaskProgress();
	void start();
	//void stop();
	void updateDisplay();
	std::wstring getCurrentTaskString();
	void incrementTask();

	// Dialog Data
	enum { IDD = IDD_TASK_PROGRESS };

	bool isTaskChanged() { return taskChanged; };
	USERTASK getCurrentTask() { return currentTask; };
	USERTASK getLastTask() { return lastTask; };
	void clearTaskChanged() { taskChanged = false; };
	void revert2LastTask() { currentTask = lastTask; updateDisplay(); };
	
private:
	USERTASK currentTask;
	USERTASK lastTask;
	bool taskChanged;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonIntro();
	afx_msg void OnBnClickedButtonTurnonwater();
	afx_msg void OnBnClickedButtonWethands();
	afx_msg void OnBnClickedButtonSqueezeoutsoap();
	afx_msg void OnBnClickedButtonScrubhands();
	afx_msg void OnBnClickedButtonRinsehands();
	afx_msg void OnBnClickedButtonTurnoffwater();
	afx_msg void OnBnClickedButtonDryhands();
	afx_msg void OnBnClickedButtonAlldone();
	afx_msg void OnBnClickedButtonSystempause();
	afx_msg void OnBnClickedButtonImmediatepause();
	afx_msg void OnBnClickedButtonEmergencystop();
};
