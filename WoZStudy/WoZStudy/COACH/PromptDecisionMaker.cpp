
#include "stdafx.h"

#include "PromptDecisionMaker.h"
#include "../WoZStudyDoc.h"

PromptDecisionMaker::PromptDecisionMaker() {
	gazeDetector = NULL;
	CGAttentionDetector = NULL;
	taskStartDetector = NULL;
	taskEndDetector = NULL;
	CGTaskDetector = NULL;
	taskProgress = NULL;
	pPrompts = NULL;
	rested = true;
}
PromptDecisionMaker::~PromptDecisionMaker() {}

void PromptDecisionMaker::start(Detector *gazeDetector
	, Detector *CGAttentionDetector
	, Detector *taskStartDetector
	, Detector *taskEndDetector
	, Detector *CGTaskDetector
	, TaskProgress *taskProgress
	, PromptStateSwitcher *promptStateSwitcher
	, PromptDecisionMakerCallBack callBack
	, PVOID callBackClassPtr
	, Prompts *pPrompts
	) {
	this->gazeDetector = gazeDetector;
	this->CGAttentionDetector = CGAttentionDetector;
	this->taskStartDetector = taskStartDetector;
	this->taskEndDetector = taskEndDetector;
	this->CGTaskDetector = CGTaskDetector;
	this->taskProgress = taskProgress;
	this->promptStateSwitcher = promptStateSwitcher;
	m_CallBack = callBack;
	m_CallBackClassPtr = callBackClassPtr;
	this->pPrompts = pPrompts;
	
	log.openLog(L".\\Log_WoZTrialLog.txt");
	log.writeSessionStart(-1);

	taskProgress->start(); //sets current task to SystemPause and marks task changed flag, which will resetStates()
	promptStateSwitcher->start();
}

void PromptDecisionMaker::end() { //called when calling callback pDoc
	resetStates();

	gazeDetector = NULL;
	CGAttentionDetector = NULL;
	taskStartDetector = NULL;
	taskEndDetector = NULL;
	CGTaskDetector = NULL;

	log.writeSessionEnd();
}

void PromptDecisionMaker::resetStates() {
	//stop the detectors
	if (gazeDetector && gazeDetector->isStarted())
		gazeDetector->stop();
	if (CGAttentionDetector && CGAttentionDetector->isStarted())
		CGAttentionDetector->stop();
	if (taskStartDetector && taskStartDetector->isStarted())
		taskStartDetector->stop();
	if (taskEndDetector && taskEndDetector->isStarted())
		taskEndDetector->stop();
	if (CGTaskDetector && CGTaskDetector->isStarted())
		CGTaskDetector->stop();

	//stop the timer
	if (timer.isStarted())
		timer.stop();

	//reset pPrompt flags
	pPrompts->reset();

	clearCountersAndFlags();
	promptState = Wait2StartPrompt;
}

void PromptDecisionMaker::clearCountersAndFlags() {
	nPrompted = 0;
	nAGed = 0;
	if (taskProgress)
		taskProgress->clearTaskChanged();
	if (pPrompts)
		pPrompts->startedPrompting = false;
}

void PromptDecisionMaker::tickPromptStateSwitcher() {
	if (promptStateSwitcher->stateChanged) {
		if (taskProgress->getCurrentTask() == SystemPause || taskProgress->getCurrentTask() == ImmediatePause) {
			taskProgress->revert2LastTask();
		}
		resetStates();
		promptStateSwitcher->stateChanged = false;
		switch (promptStateSwitcher->currentState)
		{
		case PromptStateSwitcher::FirstPrompt:
			promptState = TaskPrompt;
			nPrompted = 0;
			nAGed = 0;
			break;
		case PromptStateSwitcher::SecondPrompt:
			promptState = TaskPrompt;
			nPrompted = 1;
			nAGed = 0;
			break;
		case PromptStateSwitcher::FirstAG:
			promptState = AttentionGrabber;
			nPrompted = 1;
			nAGed = 0;
			break;
		case PromptStateSwitcher::SecondAG:
			promptState = AttentionGrabber;
			nPrompted = 1;
			nAGed = 1;
			break;
		case PromptStateSwitcher::Reward:
			promptState = Reward;
			nPrompted = 0;
			nAGed = 0;
			break;
		case PromptStateSwitcher::LetUsContinue:
			promptState = LetUsContinue;
			nPrompted = 0;
			nAGed = 0;
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	}
	else {
		if (promptState == TaskPrompt) {
			if (nPrompted == 0) {
				if (promptStateSwitcher->currentState != PromptStateSwitcher::FirstPrompt) {
					promptStateSwitcher->currentState = PromptStateSwitcher::FirstPrompt;
					promptStateSwitcher->updateDisplay();
				}
			}
			else if (nPrompted == 1) {
				if (promptStateSwitcher->currentState != PromptStateSwitcher::SecondPrompt) {
					promptStateSwitcher->currentState = PromptStateSwitcher::SecondPrompt;
					promptStateSwitcher->updateDisplay();
				}
			}
		}
		else if (promptState == AttentionGrabber) {
			if (nAGed == 0) {
				if (promptStateSwitcher->currentState != PromptStateSwitcher::FirstAG) {
					promptStateSwitcher->currentState = PromptStateSwitcher::FirstAG;
					promptStateSwitcher->updateDisplay();
				}
			}
			else if (nAGed == 1) {
				if (promptStateSwitcher->currentState != PromptStateSwitcher::SecondAG) {
					promptStateSwitcher->currentState = PromptStateSwitcher::SecondAG;
					promptStateSwitcher->updateDisplay();
				}
			}
		}
		else if (promptState == Reward) {
			if (promptStateSwitcher->currentState != PromptStateSwitcher::Reward) {
				promptStateSwitcher->currentState = PromptStateSwitcher::Reward;
				promptStateSwitcher->updateDisplay();
			}
		}
		else if (promptState == LetUsContinue) {
			if (promptStateSwitcher->currentState != PromptStateSwitcher::LetUsContinue) {
				promptStateSwitcher->currentState = PromptStateSwitcher::LetUsContinue;
				promptStateSwitcher->updateDisplay();
			}
		}
		else {
			if (promptStateSwitcher->currentState != PromptStateSwitcher::Other) {
				promptStateSwitcher->currentState = PromptStateSwitcher::Other;
				promptStateSwitcher->updateDisplay();
			}
		}
	}
}

void PromptDecisionMaker::tick2() {
	log.tick();

	if (!pPrompts->isPrompting()) {
		if (promptQueue.empty()) {
			if (!rested) {
				pPrompts->takeRest();
				rested = true;
			}
		}
		else {
			PROMPTS p = promptQueue.front();
			promptQueue.pop_front();
			pPrompts->prompt(getTaskString(p));
			rested = false;
			CWoZStudyDoc* pDoc = (CWoZStudyDoc*) ((CFrameWnd*) AfxGetMainWnd())->GetActiveDocument();
			pDoc->prompt_queue_dlg.updateDisplay(getPromptQueueString());
		}
	}
}

std::wstring PromptDecisionMaker::getTaskString(PROMPTS prompt) {
	std::wstring taskStr = L"";
	switch (prompt)
	{
	case Intro1:
		taskStr = L"Intro";
		break;
	case TurnOnWater1:
		taskStr = L"TurnOnWater";
		break;
	case WetYourHands1:
		taskStr = L"WetYourHands";
		break;
	case GetSomeSoap1:
		taskStr = L"GetSomeSoap";
		break;
	case ScrubYourHands1:
		taskStr = L"ScrubYourHands";
		break;
	case RinseYourHands1:
		taskStr = L"RinseYourHands";
		break;
	case TurnOffWater1:
		taskStr = L"TurnOffWater";
		break;
	case DryYourHands1:
		taskStr = L"DryYourHands";
		break;
	case AllDone1:
		taskStr = L"AllDone";
		break;
	case AttentionGrabber1:
		taskStr = L"AttentionGrabber";
		break;
	case Reward1:
		taskStr = L"Reward";
		break;
	case LetUsContinue1:
		taskStr = L"LetUsContinue";
		break;
	default:
		ASSERT(FALSE);
		return taskStr;
		break;
	}
	return taskStr;
}

std::wstring PromptDecisionMaker::getPromptQueueString() {
	std::wstring task = L"";
	std::deque<PROMPTS> queue = promptQueue;
	while (!queue.empty()) {
		PROMPTS p = queue.front();
		queue.pop_front();
		task += getTaskString(p) + L", ";
	}
	//task += std::to_wstring((long double)(promptQueue.size()));
	return task;
}

void PromptDecisionMaker::tick() {//is ticked every OnTimer in view

	//tick the log.  make sure this is the first thing in tick() since we want to keep the logs' time here and in View's OnTimer the same
	log.tick();

	if (taskProgress->isTaskChanged()) {
		resetStates();
		log.Write((L"current task: " + taskProgress->getCurrentTaskString()).c_str());
		//return;
	}

	tickPromptStateSwitcher();

	if (taskProgress->getCurrentTask() == EmergencyStop) {
		pPrompts->emergencyStop(); //disable all motor force
	}
	else if (taskProgress->getCurrentTask() == ImmediatePause) {
		pPrompts->immediatePause(1); //cancel current motion and then takeRest
	}
	else if (taskProgress->getCurrentTask() == SystemPause) {
		if (!pPrompts->isPrompting())
			pPrompts->takeRest(); //get into rest position then disable all motor force except head
	}
	else if (taskProgress->getCurrentTask() == Intro) {
		switch (promptState)
		{
		case Wait2StartPrompt:
			promptState = TaskPrompt;
			break;
		case TaskPrompt:
			if (!pPrompts->startedPrompting && !pPrompts->isPrompting()) {
				pPrompts->startedPrompting = true;
				prompt();
			}
			else if (pPrompts->startedPrompting && !pPrompts->isPrompting()) {
				taskProgress->incrementTask();
				promptState = Wait2StartPrompt;
			}
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	}
	else if (taskProgress->getCurrentTask() == AllDone) {
		switch (promptState)
		{
		case Wait2StartPrompt:
			promptState = TaskPrompt;
			break;
		case TaskPrompt:
			if (!pPrompts->startedPrompting && !pPrompts->isPrompting()) {
				pPrompts->startedPrompting = true;
				prompt();
			}
			else if (pPrompts->startedPrompting && !pPrompts->isPrompting()) {
				pPrompts->startedPrompting = false;
				pPrompts->takeRest();
				(*m_CallBack)(m_CallBackClassPtr, L"EndSession");
			}
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	}
	else {
		switch (promptState)
		{
		case Wait2StartPrompt:
			if (!taskStartDetector->isStarted()) {
				taskStartDetector->start();
			}
			else if (!timer.isStarted()) {
				timer.start(TIME_WAIT_PROMPT);
				pPrompts->takeRest();
			}
			else if (taskStartDetector->isDetected()) {
				log.Write(L"user task start detected");
				timer.stop();
				taskStartDetector->stop();
				promptState = Wait4User2End;
			}
			else if (timer.timesUp()) {
				log.Write(L"Wait2StartPrompt times up");
				timer.stop();
				taskStartDetector->stop();
				promptState = TaskPrompt;
			}
			break;
		case AttentionGrabber:
			if (!gazeDetector->isStarted()) {
				gazeDetector->start();
			}
			else if (gazeDetector->isDetected()) {
				if (!pPrompts->isImmediatePauseIssued() && pPrompts->isPrompting()) {
					log.Write(L"attention detected");
					timer.stop();
					pPrompts->immediatePause(0);
				}
				else if (pPrompts->isImmediatePauseIssued() && !pPrompts->isPrompting()) {
					gazeDetector->stop();
					pPrompts->reset();
					ASSERT(!pPrompts->isImmediatePauseIssued());
					pPrompts->startedPrompting = false;
					promptState = TaskPrompt;
				}
			}			
			else if (!pPrompts->startedPrompting && !pPrompts->isPrompting()) {
				pPrompts->startedPrompting = true;
				prompt();
				log.Write(L"AG started");
			}
			else if (pPrompts->startedPrompting && !pPrompts->isPrompting()) {
				pPrompts->startedPrompting = false;
				nAGed++;
				log.Write(L"AG ended");
				promptState = Wait4User2Look;
			}
			break;
		case Wait4User2Look:
			if (!timer.isStarted()) {
				timer.start(TIME_WAIT_PROMPT);
				pPrompts->takeRest();
			}
			else if (gazeDetector->isDetected()) {
				log.Write(L"attention detected");
				timer.stop();
				gazeDetector->stop();
				promptState = TaskPrompt;
			}
			else if (timer.timesUp()) {
				log.Write(L"Wait4User2Look times up");
				timer.stop();
				if (nAGed == 2) {
					gazeDetector->stop();
					promptState = CaregiverInterventionAttention;
				}
				else {
					promptState = AttentionGrabber;
				}
			}
			break;
		case CaregiverInterventionAttention:
			if (!CGAttentionDetector->isStarted()) {
				log.Write(L"CG intervention attention started");
				CGAttentionDetector->start();
				pPrompts->takeRest();
			}
			else if (CGAttentionDetector->isDetected()) {
				log.Write(L"CG intervention attention end detected");
				CGAttentionDetector->stop();
				promptState = TaskPrompt;
			}
			break;
		case TaskPrompt:
			if (!taskStartDetector->isStarted()) {
				taskStartDetector->start();
			}
			else if (taskStartDetector->isDetected()) {
				if (!pPrompts->isImmediatePauseIssued() && pPrompts->isPrompting()) {
					log.Write(L"user task start detected");
					timer.stop();
					pPrompts->immediatePause(1);
				}
				else if (pPrompts->isImmediatePauseIssued() && !pPrompts->isPrompting()) {
					taskStartDetector->stop();
					pPrompts->reset();
					ASSERT(!pPrompts->isImmediatePauseIssued());
					pPrompts->startedPrompting = false;
					promptState = Wait4User2End;
				}
			}
			else if (!pPrompts->startedPrompting && !pPrompts->isPrompting()) {
				pPrompts->startedPrompting = true;
				prompt();
				log.Write(L"task prompt started");
			}
			else if (pPrompts->startedPrompting && !pPrompts->isPrompting()) {
				pPrompts->startedPrompting = false;
				nPrompted++;
				log.Write(L"task prompt ended");
				promptState = Wait4User2Start;
			}
			break;
		case Wait4User2Start:
			if (!timer.isStarted()) {
				timer.start(TIME_WAIT_PROMPT);
				pPrompts->takeRest();
			}
			else if (taskStartDetector->isDetected()) {
				log.Write(L"user task start detected");
				timer.stop();
				taskStartDetector->stop();
				promptState = Wait4User2End;
			}
			else if (timer.timesUp()) {
				log.Write(L"Wait4User2Start times up");
				timer.stop();
				if (nPrompted == 2) {
					promptState = CaregiverInterventionTask;
					taskStartDetector->stop();
				}
				else {
					promptState = AttentionGrabber;
				}
			}
			break;
		case Wait4User2End:
			if (!taskEndDetector->isStarted()) {
				taskEndDetector->start();
			}
			else if (!timer.isStarted())
				timer.start(TIME_WAIT_TASK_FIN);
			else if (taskEndDetector->isDetected()) {
				log.Write(L"user task end detected");
				timer.stop();
				taskEndDetector->stop();
				promptState = Reward;
			}
			else if (timer.timesUp()) {
				log.Write(L"Wait4User2End times up");
				timer.stop();
				taskEndDetector->stop();
				promptState = CaregiverInterventionTask;
			}
			break;
		case Reward:
			if (!pPrompts->startedPrompting && !pPrompts->isPrompting()) {
				pPrompts->startedPrompting = true;
				prompt();
				log.Write(L"reward started");
			}
			else if (pPrompts->startedPrompting && !pPrompts->isPrompting()) {
				pPrompts->startedPrompting = false;
				log.Write(L"reward ended");
				promptState = NextTask;
			}
			break;
		case CaregiverInterventionTask:
			if (!CGTaskDetector->isStarted()) {
				log.Write(L"CG intervention task started");
				CGTaskDetector->start();
				pPrompts->takeRest();
			}
			else if (CGTaskDetector->isDetected()) {
				log.Write(L"CG intervention task end detected");
				CGTaskDetector->stop();
				promptState = LetUsContinue;
			}
			break;
		case LetUsContinue:
			if (!pPrompts->startedPrompting && !pPrompts->isPrompting()) {
				pPrompts->startedPrompting = true;
				log.Write(L"Let us continue started");
				prompt();
			}
			else if (pPrompts->startedPrompting && !pPrompts->isPrompting()) {
				log.Write(L"Let us continue ended");
				promptState = NextTask;
			}
			break;
		case NextTask:
			taskProgress->incrementTask();
			promptState = Wait2StartPrompt;
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	}
}

void PromptDecisionMaker::prompt() {
	switch (promptState)
	{
	case AttentionGrabber:
		pPrompts->prompt(L"AttentionGrabber");
		break;
	case TaskPrompt:
		pPrompts->prompt(taskProgress->getCurrentTaskString());
		break;
	case Reward:
		pPrompts->prompt(L"Reward");
		break;
	case LetUsContinue:
		pPrompts->prompt(L"LetUsContinue");
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}