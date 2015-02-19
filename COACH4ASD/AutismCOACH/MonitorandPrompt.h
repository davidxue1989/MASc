#pragma once
#include "ShowImageDlg.h"
#include "UserSettingsDlg.h"
//#include "UserSettings.h"
#include "IDDUserSettings1.h"
#include "IDDUserSettings2.h"
#include "IDDUserSettings3.h"

#include "PauseSystemDlg.h"
#include "ShowImageDlg.h"
#include <time.h>

#include <iostream>
#include <fstream>
#include "Logging.h"

#include "Timer.h" //dx nov 05, 2012

using namespace std;

class CAutismCOACHDoc;

class CMonitorandPrompt
{
public:
	CMonitorandPrompt(void);
	~CMonitorandPrompt(void);

//Member variables
public:
	CAutismCOACHDoc *pDoc;

	Logging dLog;
	//dx nov 06, 2012:
	Logging dLog_decision;

	//dx oct 25, 2012:
	bool wateron;
	bool faucetOccluded;
	bool towelLifted;


	bool skipscrub_n_rinsetracking;
	bool gettingsoap; //Jan 3
	bool hands_were_at_soap_dispenser_in_last_frame;
	bool hands_were_at_towel_in_last_frame;
	bool initiate_attention_grabber;
	bool waiting_for_attention_grabber_to_finish;
	int imnumber;
	//bool open_and_play_priming_video;
	
	bool juststartingPriming;
	bool waitForInitiatePriming;
	bool initiate_priming_user;
	bool user_being_primed_now;
	bool free_to_prompt;
	
	ofstream datafile;
	double dif;
	time_t logtime,
		lastprompttime,
		nowtime,
		lastcounttime,
		drytime,
		lighttime,
		attention_grabber_time,
		trialstartingtime;
	double attentionGrabberTime; //dx nov 25, 2012


	/*lastprompttime is recorded when:
	-prompting timer starts
	-priming video stops and closes
	-prompting video stops and closes
	-Picture prompt is shown
	-Verbal prompt is issued (measured from start of prompt)*/

	clock_t scrubstarttime, rinsestarttime;
	int userState;
	int lastState;
	int lastStateBeforeIntervention;
	int startingPromptLevels[5];
	int promptLevel;//promptLevel generally ranges from 0 to 5 - ie. if
	int numPromptsGivenThisState;
	int numPromptsGivenForEachState[5];

	//int scrub_timer;
	//int rinse_timer;
	bool RUBBINGHANDSDETECTEDBYOPTICALFLOW;//Dec. 21
	//bool SCRUBBINGROUTINEHASSTARTED;
	//bool RINSINGROUTINEHASSTARTED;
	bool state_has_changed;
	bool newstate;
	bool juststarting;
	//bool taskdone;
	bool updated;
	bool caregiverNeeded;
	//bool button_transition; //dx may 13, 2012: not used anywhere
	//bool fwd_state_transition; //dx may 13, 2012: not used anywhere
	bool hands_away;
	bool startover;
	//bool transitioning; //dx may 13, 2012: not used any where
	//bool ready2prime; //dx may 13, 2012: not used any where
	//bool dried;
	//bool verbalPromptSound//dx may 13, 2012: not used any where
	//bool StartOverSound //dx may 13, 2012: not used any where
	bool GoodJobSound; //dxnote dec 04, 2012: true when goodJobSound is playing and is not finished
	bool careGiverInterventionJustOccurred;
	bool initiate_reward;
	
	CShowImageDlg showPicSchedDlg, showPicPromptDlg, showTimerDlg;
	//dx may 23, 2012: prompt views on CG side
	CShowImageDlg showPicSchedDlg_CG, showPicPromptDlg_CG, showTimerDlg_CG;
	bool showPromptInCG;
	CString Timerpictures[21];

//Dialog box classes
public:
	CUserSettingsDlg userSettingsDlg;
	IDDUserSettings1 settings1;
	IDDUserSettings2 settings2;
	IDDUserSettings3 settings3;
	int doModalUserSettings(int index);
	CPauseSystemDlg pauseSystemDlg;

public:
	ofstream logFile;
	CString logFileName;
	double done_getting_soap_time, done_drying_time;//Feb. 18
	
//Operations
public:
	void UseAttentionGrabbers();
	void ShowPictureScheduleImage(CString path);
	void ShowPicturePromptImage(CString path);
	//bool HandsUNKNOWN(CString left, CString right);
	bool HandsUNKNOWN(int ntowel, int nsoap, int ntap, int nwater, int nsink);
	void GiveNextPrompt();
	bool Time_for_NextPrompt();

	void PromptUser();
	void PrimeUser();
	//void WaitforButton();
	
	//Prompting functions
	void ShowPicPrompt();
	void ShowPicSchedule();
	void PlayVerbalPrompt(int user_state);
	void PlayVideoPrompt(int user_state, bool verbal);
	void OpenVerbalPrompt(CString path);

	

	void wait(float seconds);
	void UpdateState(int ntowel, int nsoap, int nltap, int nrtap, int nwater, int nsink, int nsoapspout, int numblobsdetected);
	void UpdatePromptingInterface();
	void RewardCompletionofStep();
	//void ShowCountdown(int &imagenumber);
	void ShowCountdown(int imagenumber, bool remainingTimeChanged); //dx nov 05, 2012
	void Setflags();
	void UpdateSystem(CString l_Hnd_Obsrvn, CString r_Hnd_Obsrvn);
	void UpdateSystem(int ntowel, int nsoap, int ntap, int nwater, int nsink);

	//dx may 11, 2012:
	int sessionNumber;
	//dx may 09, 2012: prompt_started and prompt_ended flags
	bool prompt_started, prompt_ended;
	//dx may 07, 2012: nosbervations (the LogVars)
	bool waterOn;
	int ntowel, nsoap, nsoapspout, nltap, nrtap, nwater, nsink;

	//dx may 05, 2012: logging the states
	CStdioFile log_decisionMaking, log_prompting;
	time_t sessionStart_l, sessionEnd_l;
	//decision making log
	bool waterOn_l;
	int ntowel_l, nsoap_l, nsoapspout_l, nltap_l, nrtap_l, nwater_l, nsink_l;
	int thisState_l, nextState_l;
	time_t stateStartTime_l, stateEndTime_l;
	void logState();
	//prompting log
	int thisPromptType_l, promptLevel_l, prompt_userState_l;
	//time_t promptStartTime_l, promptEndTime_l;	
	////priming log
	//time_t primingStartTime_l, primingEndTime_l;
	////attention grabber log
	//time_t attentionStartTime_l, attentionEndTime_l;

	
	void reinitialize(void);
	void sessionEndLog(void);
	IplImage *pictureSchedule; //dx aug 18, 2012

	//dx sep 15, 2012
	bool trainScrubbing_justStarted; 
	bool trainScrubbing_scrubbingStarted;
	bool trainScrubbing_nonScrubbingStarted;
	bool trainScrubbing_finished;

	//dx nov 05, 2012
	Timer wetTimer, soapTimer, scrubTimer, rinseTimer, dryTimer, leaveTimer;

	//dx nov 28, 2012
	bool TOWELINTERACTIONDETECTED; //flag set to true when PERCENTAGE_OF_FRAMES_TOWEL_INTERACTING percent of frames in towelInteractionDectectedQueue are true
	
	//dx nov 30, 2012:
	int bundleCount; //how many new frames have we put into the bundle so far	
	enum ScrubbingLoop_mode
	{
		GRABBING_MODE,
		PROCESSING_MODE
	} scrubbingLoop_mode;
	deque<IplImage *> imagesBundle;
	//dx dec 01, 2012: pDoc.monitorandPrompt's own timestamp - detached videoLog's time mechanism from dLog's time mechanism
	struct TimeStamp
	{
		CTime timeStamp;
		DWORD timeStamp_msec;
	} timeStampStruct;
	CTime timeStamp;
	DWORD timeStamp_msec;
	//deque<TimeStamp> timeStampBundle;
	void setTimeStamp() { timeStampStruct.timeStamp=CTime::GetCurrentTime(); timeStampStruct.timeStamp_msec=GetTickCount()%10000; };
	CString timeStamp2CStr();
	//dx dec 01, 2012
	void initializeScrubbingLoop_GRABBING_MODE();
	bool scrubbingLoop_die;
	bool scrubbingLoop_clearQueues;
};

