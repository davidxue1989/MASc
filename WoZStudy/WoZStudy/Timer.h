#pragma once
#include "stdafx.h"
class Timer
{
public:
	Timer() :periodMsec(-1), startTime(-1), started(false) {};
	~Timer() {};
	void start(int sec) { periodMsec = sec * 1000; startTime = GetTickCount(); started = true; };
	void restart() { start(periodMsec); };
	int getPeriod() { return periodMsec/1000; };
	bool timesUp() { return GetTickCount() - startTime >= periodMsec; };
	void stop() { started = false; periodMsec = -1; startTime = -1; }
	bool isStarted() { return started; }

private:
	DWORD periodMsec;
	DWORD startTime;
	bool started;
};

