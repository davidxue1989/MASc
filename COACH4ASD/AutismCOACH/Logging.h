#pragma once

class Logging
{
public:
	Logging(CString logPath);
	//Logging(CString path);
	Logging(CString logPath, CString logTag);
	~Logging(void);
	void Write(CString message);
	struct TIMER //since Timer class needs to include Logging for logging purposes, we cannot include Timer.h here, thus we can only pass timer's contents to be logged by a struct
	{	
		CString timerName;
		bool activityDetected_ThisFrame;
		bool activityDetected_LastFrame;
		int totalTime;
		int remainingTotalTime;
		bool totalTimerDone;
		int unitTime;
		double remainingUnitTimeLastChecked;
		bool unitTimerDone;
	};
	void Write(TIMER timer, CString msg); //dx nov 06, 2012
	void Write(TIMER timer, int userState);

	CString logPath;
	CStdioFile logFile;
	CString readBuffer;
	CString lastLogMessage;
	CTime timeStamp;
	//int year, month, day, hour, minute, second;
	DWORD timeStamp_msec;
	bool nonVerbose;

	void Open(CString path);
	CString GetCurrentTimeStamp(void);
	CString GetTimeStamp(void);
	CString FormatTimeStamp(void);
	//void SetTimeStamp(void);
	void SetTimeStamp(CTime timeStamp, DWORD timeStamp_msec) { this->timeStamp=timeStamp; this->timeStamp_msec=timeStamp_msec; }


	void WriteNewLine(int nLine);
	int GetLastSessionNumber();

	//dx nov 03, 2012:
	CString logTag;
	struct NOBSNs
	{		
		bool wateron;
		bool faucetOccluded;
		int nwater_obs;

		int nsoap_obs;
		int nsoapspout_obs;
				
		bool RUBBINGHANDSDETECTEDBYOPTICALFLOW;
		int ntapl_obs;
		int ntapr_obs;
		
		int ntowel_obs;
		bool towelLifted;	

		int nsink_obs;

		void setVars(bool wateron, bool faucetOccluded, int nwater_obs, int nsoap_obs, int nsoapspout_obs, bool RUBBINGHANDSDETECTEDBYOPTICALFLOW, 
			int ntapl_obs, int ntapr_obs, int ntowel_obs, bool towelLifted, int nsink_obs)
		{ this->wateron = wateron; this->faucetOccluded = faucetOccluded; this->nwater_obs = nwater_obs; this->nsoap_obs = nsoap_obs;
		  this->nsoapspout_obs = nsoapspout_obs; this->RUBBINGHANDSDETECTEDBYOPTICALFLOW = RUBBINGHANDSDETECTEDBYOPTICALFLOW; 
		  this->ntapl_obs = ntapl_obs; this->ntapr_obs = ntapr_obs; this->ntowel_obs = ntowel_obs; this->towelLifted = towelLifted;
		  this->nsink_obs = nsink_obs; }
	} nObsns;
};
