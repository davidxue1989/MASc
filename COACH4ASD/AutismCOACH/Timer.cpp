#include "StdAfx.h"
#include "Timer.h"

#define        ROUND(A)        ((A)<0.0?-((int)(-(A)+0.5)):((int)((A)+0.5)))

//nov 04, 2012:
Timer::Timer(CString timerName, Logging *p_dLog_decision)
{
	this->timerName = timerName;
	this->p_dLog_decision = p_dLog_decision;

	unitTime = 0;
	unitTimerDone = false;
	remainingUnitTimeLastChecked = 0;

	totalTime = 0;
	totalTimerDone = false;
	remainingTotalTime = 0;

	dontInit = false;
	activityDetected_ThisFrame = activityDetected_LastFrame = false;
}

Timer::~Timer()
{
	
}

//initializes variables for both unit and total timers to the starting state
void Timer::initTimer(int totalTime, int unitTime)
{
	this->unitTime = unitTime;
	resetUnitTimer();

	this->totalTime = totalTime;
	resetTotalTimer();

	activityDetected_ThisFrame = activityDetected_LastFrame = false;
}

void Timer::resetTotalTimer()
{
	totalTimerDone = false;
	remainingTotalTime = totalTime;
}

//reset unitTimerDone to false, updates currentTimeUnitTimer, sets startTimeUnitTimer to it, then (for logging purpose) updates remainingUnitTimeLastChecked to unitTime 
void Timer::resetUnitTimer()
{
	unitTimerDone = false;
	currentTimeUnitTimer = startTimeUnitTimer = (double) GetTickCount() / 1000;
	assert(currentTimeUnitTimer == startTimeUnitTimer);
	remainingUnitTimeLastChecked = unitTime;
}

//updates currentTimeUnitTimer and calculates time remaining till unitTime and sets remainingUnitTimeLastChecked to it
bool Timer::checkUnitTimer()
{
	assert(!unitTimerDone); //must have called resetTimer before checkTimer, making unitTimerDone always false at entry

	currentTimeUnitTimer = (double) GetTickCount() / 1000;
	remainingUnitTimeLastChecked = unitTime - (currentTimeUnitTimer - startTimeUnitTimer);
	if (remainingUnitTimeLastChecked <= 0)
	{
		unitTimerDone = true;
		p_dLog_decision->Write(toStruct(), "done unit timer");//dLogDecisionMaking.log(wetTimer);
		decrementTotalTimer();
		resetUnitTimer(); //dxnote nov 08, 2012: must resetUnitTimer after decrementTotalTimer, since decrementTotalTimer needs remainingUnitTimeLastChecked's value
	}
	return unitTimerDone; //returns bool flag for logging purposes
}

void Timer::decrementTotalTimer()
{
	assert(!totalTimerDone); //shouldn't be decrementing if totalTimer is already timed out; if assertion failed, must mean the program using the Timer isn't checking getTotalTimerDone() regularly or haven't initTimer() after last use
	assert(ROUND(remainingUnitTimeLastChecked) <= 0);
	//dx nov 08, 2012: sometimes we don't get to call checkTimer within unitTime, thus we shouldn't assume we can just decrement unitTime each time, but at least unitTime
	//remainingTotalTime -= unitTime;
	remainingTotalTime += ROUND(remainingUnitTimeLastChecked - unitTime);
	totalTimerDone = (remainingTotalTime <= 0);
}

//given activityDetected_ThisFrame from vision module, it manipulates the timer accordingly so that totalTimerDone will be set to true once the total time set by user is reached
void Timer::checkTimer(bool activityDetected_ThisFrame)
{
	this->activityDetected_ThisFrame = activityDetected_ThisFrame;

	if (this->activityDetected_ThisFrame)
	{
		if (!activityDetected_LastFrame)
		{
			resetUnitTimer();
			assert(remainingUnitTimeLastChecked == unitTime);
			p_dLog_decision->Write(toStruct(), "restart unit timer");//dLogDecisionMaking.log(wetTimer);
		}
		else if (activityDetected_LastFrame)
			checkUnitTimer();
	}
	else if (!this->activityDetected_ThisFrame)
	{
		if (!activityDetected_LastFrame)
		{
			//do nothing
		}
		else if (activityDetected_LastFrame)
		{
			assert(remainingUnitTimeLastChecked >= 0); //since wetting detected in last frame, the checkUnitTimer should have been called and thus remainingUnitTimeLastChecked cannot be <= 0
			p_dLog_decision->Write(toStruct(), "stop unit timer");//dLogDecisionMaking.log(wetTimer);
		}
	}
	activityDetected_LastFrame = activityDetected_ThisFrame;
}

Logging::TIMER Timer::toStruct() //put timer's contents to be logged into a struct so Logging doesn't need to include Timer.h
{
	Logging::TIMER	timerStruct;
	timerStruct.activityDetected_LastFrame = activityDetected_LastFrame;
	timerStruct.activityDetected_ThisFrame = activityDetected_ThisFrame;
	timerStruct.remainingTotalTime = remainingTotalTime;
	timerStruct.remainingUnitTimeLastChecked = remainingUnitTimeLastChecked;
	timerStruct.timerName = timerName;
	timerStruct.totalTime = totalTime;
	timerStruct.totalTimerDone = totalTimerDone;
	timerStruct.unitTime = unitTime;
	timerStruct.unitTimerDone = unitTimerDone;
	return timerStruct;
}

//dx nov 25, 2012:
bool Timer::checkRemainingTotalTimeLastChecked() 
{//returns true if remainingTotalTimeLastChecked is the same as remainingTotalTime
	int temp = remainingTotalTimeLastChecked;
	remainingTotalTimeLastChecked = remainingTotalTime;
	return (temp != remainingTotalTime);
}