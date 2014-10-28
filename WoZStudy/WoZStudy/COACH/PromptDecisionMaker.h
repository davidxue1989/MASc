
#pragma once
#include "../Logging.h"
#include "../Timer.h"
#include "../Detector.h"
#include "../TaskProgress.h"
#include "../PromptStateSwitcher.h"
#include "../COACH/Prompts.h"

#include "../COACH/defines.h"
#include <deque>

typedef void(*PromptDecisionMakerCallBack)(PVOID callBackClassPtr, CString msg);

class PromptDecisionMaker
{
public:
	PromptDecisionMaker();
	~PromptDecisionMaker();

	enum PROMPTSTATE {
		Wait2StartPrompt,
		AttentionGrabber,
		Wait4User2Look,
		CaregiverInterventionAttention,
		TaskPrompt,
		Wait4User2Start,
		Wait4User2End,
		Reward,
		CaregiverInterventionTask,
		LetUsContinue,
		NextTask
	};

	enum PROMPTS { //this is used by tick2() only
		Intro1,
		TurnOnWater1,
		WetYourHands1,
		GetSomeSoap1,
		ScrubYourHands1,
		RinseYourHands1,
		TurnOffWater1,
		DryYourHands1,
		AllDone1,
		AttentionGrabber1,
		Reward1,
		LetUsContinue1,
	};


	void start(Detector *gazeDetector
		, Detector *CGAttentionDetector
		, Detector *taskStartDetector
		, Detector *taskEndDetector
		, Detector *CGTaskDetector
		, TaskProgress *taskProgress
		, PromptStateSwitcher *promptStateSwitcher
		, PromptDecisionMakerCallBack callBack
		, PVOID callBackClassPtr
		, Prompts *pPrompts
		);
	void end();
	void tick();
	
	void tick2(); //this is for the version where we don't increment state automatically, instead every prompt is controlled by the wizard, and actions pile up in a queue
	std::deque<PROMPTS> promptQueue;
	bool rested;
	std::wstring getTaskString(PROMPTS prompt);
	std::wstring getPromptQueueString();

	void clearCountersAndFlags();
	//void changePromptState(PROMPTSTATE nextState);
	void prompt();
	void resetStates();
	void tickPromptStateSwitcher();

	Detector					*gazeDetector;;
	Detector					*CGAttentionDetector;
	Detector					*taskStartDetector;
	Detector					*taskEndDetector;
	Detector					*CGTaskDetector;
	TaskProgress				*taskProgress;
	PromptStateSwitcher			*promptStateSwitcher;

	Prompts *pPrompts;

	Logging	log;

private:

	PROMPTSTATE promptState;
	int nPrompted;
	int nAGed;

	Timer timer;

	PromptDecisionMakerCallBack m_CallBack;
	PVOID m_CallBackClassPtr;
};
