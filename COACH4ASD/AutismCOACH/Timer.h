/*
dx nov 05, 2012:
- for counting the length of time that a certain activity is performed by the user.
- there are two kinds of timer, totalTimer and unitTimer
	- unitTimer will be resetted if the activity doesn't reach unitTime
	- if an activity reaches unitTime, the time is accumulated in the totalTimer
	- once totalTime is reached in the totalTimer, totalTimerDone is set to true
- activityDetected_ThisFrame is given from vision module
*/


#pragma once
#include "Logging.h"

//nov 04, 2012
class Timer
{
public:
	Timer(CString timerName, Logging *p_dLog_decision);
	~Timer();
	
	void initTimer(int totalTime, int unitTime);
	void checkTimer(bool activityDetected_ThisFrame);

	
	Logging::TIMER toStruct (); //dx nov 05, 2012

	//getters:
	//CString getTimerName() { return timerName; }
	//bool getUnitTimerDone() { return unitTimerDone; }
	//double getRemainingUnitTimeLastChecked() { return remainingUnitTimeLastChecked; }
	bool getTotalTimerDone() { return totalTimerDone; }
	int getRemainingTotalTime() { return remainingTotalTime; }
	bool getActivityDetectedThisFrame() { return activityDetected_ThisFrame; } //dx nov 25, 2012

	bool checkRemainingTotalTimeLastChecked(); //dx nov 25, 2012
	bool activityStarted() { return (remainingTotalTime != totalTime); } //dx nov 25, 2012: returns true if we decremented some time off of totalTime already

	//variables:
	bool dontInit; //set true in during scrubbing to allow rinseTimer not resetted going from scrubbing to rinsing, but resetted if going to rinsing in other ways (e.g. from system pause)

protected:
	//setters:
	//void setUnitTime(int unitTime) { this->unitTime = unitTime; }
	//void setTotalTime(int totalTime) { this->totalTime = totalTime; }

	//helper functions:
	void resetUnitTimer();
	bool checkUnitTimer();
	void resetTotalTimer();
	void decrementTotalTimer();

private:
	CString timerName; //for logging purposes, is this a wetTimer/soapTimer/scrubTimer/rinseTimer/dryTimer/leaveTimer
	
	int unitTime;
	bool unitTimerDone; //whether one unitTime is reached
	double remainingUnitTimeLastChecked; //in seconds = ticCounts(in msec)/1000
	int totalTime;
	bool totalTimerDone;
	int remainingTotalTime; //in seconds = set to totalTime in beginning, decremented by unitTime each time unitTime is reached

	double currentTimeUnitTimer; //in seconds
	double startTimeUnitTimer; //in seconds
	
	bool activityDetected_ThisFrame, activityDetected_LastFrame;

	//nov 06, 2012:
	//friend class Logging;
	Logging *p_dLog_decision;

	int remainingTotalTimeLastChecked; //dx nov 25, 2012
};