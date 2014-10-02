
#pragma once
#include "../Logging.h"
#include "../Timer.h"
#include "../Detector.h"
#include "../TaskProgress.h"
#include "../PromptStateSwitcher.h"
#include "../COACH/Prompts.h"

#include "../COACH/defines.h"

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
