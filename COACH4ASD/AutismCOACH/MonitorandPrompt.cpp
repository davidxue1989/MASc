//dx april 03, 2012: changing prompt from second monitor to first. done changing mcisendstring. starting schedule pictures
//dx april 27, 2012: changing prompt from second monitor to first. done picture schedules.
//dx april 27, 2012 - 2: changing back the prompt to second monitors (to use splashtop on android as extended monitor, and to have splashtop start in the mainscreen, gotta make monitor 2 the main screen, and put it to the left of monitor 1
#include "StdAfx.h"
#include "MonitorandPrompt.h"
#include "AutismCOACHDoc.h"

#include <MMSystem.h> // for playing a .wav file
#pragma comment(lib, "winmm.lib") // for playing a .wav file
#include <windows.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream> //dx april 03, 2012
#include "Logging.h"


#include <queue>

using namespace std;

#define REWARDALLSTEPS 0
#define PRIME 0

#define SOAPDISPENSER 1
#define TOWELDISPENSER 2
#define STARTOVER 0
#define USEVERBALPROMPT 0
#define DONOTUSEATTENTIONGRABBERS 3
#define USESOUNDANDIMAGE 2
#define USEIMAGEONLY 1
#define USESOUNDONLY 0



#define LOG_PATH "C:\\dxFiles\\COACH4ASDresourceFiles\\testLog\\COACH4ASDLog.txt"

//dx april 03, 2012------
void mciSendString_put_firstScreen (string deviceName){ //dxnote: putting a device on first screen, full screen sized minus schedule picture banner
	//oss.str(""); //these two lines clear the oss and makes it able to be used again http://forums.codeguru.com/showthread.php?t=292719
	//oss.clear();
	ostringstream oss;
	oss << "put " << deviceName << " window at 0 0 " << FIRSTMONITORWIDTH << ' ' << int(FIRSTMONITORHEIGHT*4/5);
	mciSendString(oss.str().c_str(), NULL, 0, 0);//dxnote: .c_str() to convert std string to LPCSTR
}
//-----------------------
//dx april 27, 2012 - 2 ----
void mciSendString_put_secondScreen (string deviceName){ //dxnote: putting a device on first screen, full screen sized minus schedule picture banner
	//oss.str(""); //these two lines clear the oss and makes it able to be used again http://forums.codeguru.com/showthread.php?t=292719
	//oss.clear();
	ostringstream oss;
	oss << "put " << deviceName << " window at " <<FIRSTMONITORWIDTH <<" 0 " << SECONDMONITORWIDTH << ' ' << int(SECONDMONITORHEIGHT*4/5);
	mciSendString(oss.str().c_str(), NULL, 0, 0);//dxnote: .c_str() to convert std string to LPCSTR
}
//---------------------------
void mciSendString_put_CGPromptView (string deviceName){
	ostringstream oss, oss1;
	oss << "put " << deviceName << " window at " <<int(FIRSTMONITORWIDTH-FIRSTMONITORWIDTH*0.2)<< ' ' <<int(FIRSTMONITORHEIGHT-FIRSTMONITORHEIGHT*0.2)<< ' ' << int(FIRSTMONITORWIDTH*0.2) << ' ' << int(FIRSTMONITORHEIGHT*0.2 *4/5);
	//oss1 << "put " << deviceName << " window at " <<819.2<< ' ' <<500<< ' ' <<100<< ' ' << 100;
	//CString a = oss1.str().c_str();
	mciSendString(oss.str().c_str(), NULL, 0, 0);//dxnote: .c_str() to convert std string to LPCSTR
}



CMonitorandPrompt::CMonitorandPrompt(void)
	: pictureSchedule(NULL)
	, dLog(LOG_PATH)
	, dLog_decision(LOG_PATH, "DECISION")/*dx nov 06, 2012*/
	, wetTimer("wetTimer", &dLog_decision), soapTimer("soapTimer", &dLog_decision)
	, scrubTimer("scrubTimer", &dLog_decision), rinseTimer("rinseTimer", &dLog_decision)
	, dryTimer("dryTimer", &dLog_decision) /*dx nov 05, 2012*/
	, leaveTimer("leaveTimer", &dLog_decision) /*dx nov 28, 2012*/
{	
	//open_and_play_priming_video = false;//Jan12
	juststartingPriming = true;
	waitForInitiatePriming = false;
	initiate_priming_user = false;
	user_being_primed_now = false;
	initiate_attention_grabber = false;	
	waiting_for_attention_grabber_to_finish = false;
	free_to_prompt = false;	
	juststarting = false; //dx nov 06, 2012
	//dx may 09, 2012:
	prompt_started=false;
	prompt_ended=false;
	
	initiate_reward = false;
	
	//Initialize userState, lastState to 0, taskCompleted to false

	//dx oct 25, 2012:	
	wateron = false;	
	faucetOccluded = false;
	towelLifted = true; //since we detects whether the background underneath the towel is changed as towel not lifed

	imnumber = 10;
	gettingsoap = false;//Jan 3
	hands_were_at_soap_dispenser_in_last_frame = false;
	hands_were_at_towel_in_last_frame = false;
	
	
	userState = 0;
	lastState = 0;
	lastStateBeforeIntervention = 0;
	//state_has_changed = false;
	state_has_changed = true; //dx nov 05, 2012: for timer.init to be called in UpdatePromptingInterface at start of session
	//taskdone = false;
	updated = false;
	caregiverNeeded = false;
	//fwd_state_transition = false;//dx may 13, 2012: not used any where
	hands_away = false;
	//taskCompleted = false;

	//SCRUBBINGROUTINEHASSTARTED = false;
	//RINSINGROUTINEHASSTARTED = false;
	RUBBINGHANDSDETECTEDBYOPTICALFLOW = false;//Dec. 21
	TOWELINTERACTIONDETECTED = false; //dx nov 28, 2012

	startover = false;
	//transitioning = false; //dx may 13, 2012: not used any where
	//ready2prime = false; //dx may 13, 2012: not used any where

	//dried = false;
	skipscrub_n_rinsetracking = false;
	//verbalPromptSound = false;//dx may 13, 2012: not used any where
	GoodJobSound = false;
	//StartOverSound = false; //dx may 13, 2012: not used any where

	

	////done_getting_soap_time = 1.5;//January 11, 2012
	//done_getting_soap_time = 0.5;//dx may 21, 2012: changed it to a shorter time
	done_drying_time = 1.0;

	showPicSchedDlg.Create(IDD_SHOWIMAGE_DIALOG, NULL);
	showPicPromptDlg.Create(IDD_SHOWIMAGE_DIALOG, NULL);
	showTimerDlg.Create(IDD_SHOWIMAGE_DIALOG, NULL);

	//dx may 23, 2012: prompt views on CG side
	showPicSchedDlg_CG.Create(IDD_SHOWIMAGE_DIALOG, NULL);
	showPicPromptDlg_CG.Create(IDD_SHOWIMAGE_DIALOG, NULL);
	showTimerDlg_CG.Create(IDD_SHOWIMAGE_DIALOG, NULL);
	

	//showPicSchedDlg.ResizeLargeImage(FIRSTMONITORWIDTH, SECONDMONITORHEIGHT*4/5, SECONDMONITORWIDTH, SECONDMONITORHEIGHT/5);
	//showPicPromptDlg.ResizeLargeImage(FIRSTMONITORWIDTH, 0, SECONDMONITORWIDTH, SECONDMONITORHEIGHT*4/5);
	//dx april 27, 2012: switching to first monitor -------------------
	//showPicSchedDlg.ResizeLargeImage(0, FIRSTMONITORHEIGHT*4/5, FIRSTMONITORWIDTH, FIRSTMONITORHEIGHT/5); 
	//showPicPromptDlg.ResizeLargeImage(0, 0, FIRSTMONITORWIDTH, FIRSTMONITORHEIGHT*4/5); 
	//dx april 27, 2012 - 2 (switching back to second monitor?)
	showPicSchedDlg.ResizeLargeImage(FIRSTMONITORWIDTH, SECONDMONITORHEIGHT*4/5, SECONDMONITORWIDTH, SECONDMONITORHEIGHT/5); 
	showPicPromptDlg.ResizeLargeImage(FIRSTMONITORWIDTH, 0, SECONDMONITORWIDTH, SECONDMONITORHEIGHT*4/5); 
	//-----------------------------------------------------------------	
	
	//dx may 23, 2012: prompt views on CG side
	showPicSchedDlg_CG.ResizeLargeImage(FIRSTMONITORWIDTH-FIRSTMONITORWIDTH*0.2, FIRSTMONITORHEIGHT-FIRSTMONITORHEIGHT*0.2*1/5, FIRSTMONITORWIDTH*0.2, FIRSTMONITORHEIGHT*0.2/5); 
	showPicPromptDlg_CG.ResizeLargeImage(FIRSTMONITORWIDTH-FIRSTMONITORWIDTH*0.2, FIRSTMONITORHEIGHT-FIRSTMONITORHEIGHT*0.2, FIRSTMONITORWIDTH*0.2, FIRSTMONITORHEIGHT*0.2*4/5); 
	
	//showTimerDlg.ResizeLargeImage(FIRSTMONITORWIDTH + 2*SECONDMONITORWIDTH/5, SECONDMONITORHEIGHT - 200 - SECONDMONITORHEIGHT/5, 200, 200);//Feb. 7, 2011
	//showTimerDlg.ResizeLargeImage(0, 0, 200, 200);

	//datafile.open ("onpainttimestamps.txt");
	
	pictureSchedule = NULL; //dx aug 18, 2012

	Timerpictures[0] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T0.bmp";
	Timerpictures[1] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T1.bmp";
	Timerpictures[2] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T2.bmp";
	Timerpictures[3] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T3.bmp";
	Timerpictures[4] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T4.bmp";
	Timerpictures[5] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T5.bmp";
	Timerpictures[6] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T6.bmp";
	Timerpictures[7] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T7.bmp";
	Timerpictures[8] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T8.bmp";
	Timerpictures[9] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T9.bmp";
	Timerpictures[10] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T10.bmp";
	Timerpictures[11] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T11.bmp";
	Timerpictures[12] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T12.bmp";
	Timerpictures[13] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T13.bmp";
	Timerpictures[14] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T14.bmp";
	Timerpictures[15] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T15.bmp";
	Timerpictures[16] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T16.bmp";
	Timerpictures[17] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T17.bmp";
	Timerpictures[18] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T18.bmp";
	Timerpictures[19] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T19.bmp";
	Timerpictures[20] = "C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\T20.bmp";

	careGiverInterventionJustOccurred = false;
	numPromptsGivenForEachState[0] = 0;//January 13, 2012
	
	//dx may 11, 2012:
	sessionNumber = 0;
	//dx may 07, 2012: 
	//observations
	waterOn=false;
	ntowel=0;
	nsoap=0;
	nltap=0;
	nrtap=0;
	nwater=0;
	nsink=0;
	//dx may 06, 2012: logging initialization
	//decision making log
	waterOn_l=false;
	ntowel_l=0;
	nsoap_l=0;
	nltap_l=0;
	nrtap_l=0;
	nwater_l=0;
	nsink_l=0;
	thisState_l=0;
	nextState_l=0;
	stateStartTime_l=0;
	stateEndTime_l=0;
	//prompting log
	promptLevel_l=0;
	prompt_userState_l=0;
	//promptStartTime_l=0;
	//promptEndTime_l=0;
	////priming log
	//primingStartTime_l=0;
	//primingEndTime_l=0;
	////attention grabber log
	//attentionStartTime_l=0;
	//attentionEndTime_l=0;
	

	showPromptInCG = TRUE;

	//dx sep 15, 2012:
	trainScrubbing_justStarted = false;
	trainScrubbing_scrubbingStarted = false;
	trainScrubbing_nonScrubbingStarted = false;
	trainScrubbing_finished = false;

	//dx nov 30, 2012:
	bundleCount = 0;
	scrubbingLoop_mode = GRABBING_MODE;
}

CMonitorandPrompt::~CMonitorandPrompt(void)
{
}

//bool CMonitorandPrompt::HandsUNKNOWN(CString left, CString right)
//{
//	if ( (left=="off task") && (right=="off task"))
//		return true;
//	else
//		return false;
//}

bool CMonitorandPrompt::HandsUNKNOWN(int ntowel, int nsoap, int ntap, int nwater, int nsink)
{
	if ( ntowel + nsoap + ntap + nwater + nsink == 0)
		return true;
	else
		return false;
}

void CMonitorandPrompt::ShowPicPrompt()
{
	//If the user has specified to use verbal prompts only for this prompting level...
	if (userSettingsDlg.prompttypes[promptLevel-1] == USEVERBALPROMPT)
	{
		ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\blackscreen.bmp");
		return;
	}


	switch (userState)
	{
	case 0:
		ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\wetyourhands.JPG");
		break;

	case 1:
		ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\getthesoap.JPG");
		break;

	case 2:
		ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\scrubyourhands.JPG");
		break;

	case 3:
		showTimerDlg.ShowWindow(SW_HIDE);
		//dx may 23, 2012: show prompt view on CG side
		showTimerDlg_CG.ShowWindow(SW_HIDE);
		ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\rinseyourhands.JPG");
		break;

	case 4:
		showTimerDlg.ShowWindow(SW_HIDE);
		//dx may 23, 2012: show prompt view on CG side
		showTimerDlg_CG.ShowWindow(SW_HIDE);
		//if (wateron && dried)
		if (wateron && dryTimer.getTotalTimerDone())
			ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\turnoffwater.JPG");
		else
			ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\dryyourhands.JPG");
		break;

	case 5:
		ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\ALLDONE.bmp");
		break;
	}
}

void CMonitorandPrompt::ShowPicSchedule()
{
	switch (userState)
	{
	case 0:
		//cvLoadImage("C:\dxFiles\COACH4asd jul 18, 2012\AutismCOACH\resourceFiles\Media For COACH for ASD\Picture Schedule\picsched0.bmp");
		//cvLoadImage("..\AutismCOACH\resourceFiles\Media For COACH for ASD\Picture Schedule\picsched0.bmp");

		//ShowPictureScheduleImage("../Media For COACH for ASD\\Picture Schedule\\picsched0.bmp");
		//ShowPictureScheduleImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Schedule\\picsched0.bmp");
		ShowPictureScheduleImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Schedule\\picsched0.bmp");
		break;

	case 1:
		ShowPictureScheduleImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Schedule\\picsched1.bmp");
		break;

	case 2:
		ShowPictureScheduleImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Schedule\\picsched2.bmp");
		break;

	case 3:
		ShowPictureScheduleImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Schedule\\picsched3.bmp");
		break;

	case 4:
		ShowPictureScheduleImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Schedule\\picsched4.bmp");
		break;

	case 5:
		ShowPictureScheduleImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Schedule\\picsched5.bmp");
		break;
	}
}

void CMonitorandPrompt::PrimeUser()
{
	//if (careGiverInterventionJustOccurred && userState !=6)
	//{
	//	juststartingPriming = true;
	//	waitForInitiatePriming = false;
	//	initiate_priming_user = false;//January 13, 2012
	//	user_being_primed_now = false;
	//}

	//skip if state_has changed
	if (state_has_changed)
	{	
		//juststartingPriming = false;
		//waitForInitiatePriming = false;
		////reset flag needed for priming to occur
		//initiate_priming_user = false;
		////reset other "state" variables
		//user_being_primed_now = false;
		return;
	}

	if(juststartingPriming)
	{
		//dx aug 29, 2012: make sure when sessions starts, screen is black with picture schedule
		ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\blackscreen.bmp");
		ShowPicSchedule();

		time(&lastprompttime);
		juststartingPriming = false;		
		if (userState == 5) //dx dec 04, 2012: if steps are all done, start immediately the prompting (great, you are all done, you can leave now)
			juststarting = true;
		else
			waitForInitiatePriming = true;
		return;
	}
	if((!juststartingPriming) && waitForInitiatePriming)
	{
		if(Time_for_NextPrompt() == false) //wait until first prompt time
			return;
		else
		{
			waitForInitiatePriming = false;
			initiate_priming_user = true;
		}
	}
	//skip if not proper time for priming
	if ( (!initiate_priming_user)&&(!user_being_primed_now) )
		return;

	//skip if user is not using priming
	if (userSettingsDlg.priming != PRIME)
	{
		//check if the initiate_attention_grabber flag is set
		if (initiate_priming_user)
		{
			//reset
			initiate_priming_user = false;
			//set flag for attantion grabber
			initiate_attention_grabber = true;
		}
		return;
	}

	//Prime the user now if it is the proper time
	else if (userSettingsDlg.priming == PRIME)
	{
		ofstream loopRateFile ("C:\\dxFiles\\COACH4ASDresourceFiles\\loopRate.txt", ios::app);
		//ASSERT(loopRateFile.is_open());
		DWORD loopStartTime, loopNowTime;

		//If it is time to prime the user, then prime the user
		if (initiate_priming_user)
		{			
			loopRateFile << "\tIn PrimeUser: " << endl;
			loopStartTime = GetTickCount();

			//dx may 07, 2012: log priming
			//time(&primingStartTime_l);
			//dx may 09, 2012:
			dLog.Write("prime start");			
			////dxdebug may 13, 2012:
			//CString temp;
			//temp.Format("initiate_priming_user = %d, user_being_primed_now = %d, initiate_attention_grabber = %d, waiting_for_attention_grabber_to_finish = %d, free_to_prompt = %d, juststarting = %d"
			//	,initiate_priming_user,user_being_primed_now,initiate_attention_grabber,waiting_for_attention_grabber_to_finish,free_to_prompt,juststarting);
			//dLog.Write(temp);

			showPicPromptDlg.ShowWindow(SW_HIDE); //dx jul 09, 2012: making sure the blank screen picture doesn't block the priming video
			showTimerDlg.ShowWindow(SW_HIDE); //dx jul 09, 2012: making sure the count down timer picture doesn't block the priming video
			
			loopNowTime = GetTickCount();
			loopRateFile << "\t" << loopNowTime - loopStartTime << ", pre open mPrimingVideo" << endl;
			loopStartTime = GetTickCount();

			mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\Movie.wmv\" type MPEGVideo alias myPrimingVideo style popup", NULL, 0, 0);
			
			loopNowTime = GetTickCount();
			loopRateFile << "\t" << loopNowTime - loopStartTime << ", done open mPrimingVideo" << endl;
			loopStartTime = GetTickCount();
			
			//mciSendString("put myPrimingVideo window at 1024 0 1280 820", NULL, 0, 0);
			//dx april 03, 2012------
			//mciSendString_put_firstScreen("myPrimingVideo");
			//dx april 27, 2012 - 2 ----
			mciSendString_put_secondScreen("myPrimingVideo");

			loopNowTime = GetTickCount();
			loopRateFile << "\t" << loopNowTime - loopStartTime << ", done put to second screen" << endl;
			loopStartTime = GetTickCount();
			
			//-----------------------
			mciSendString("play myPrimingVideo", NULL, 0, 0);
			
			loopNowTime = GetTickCount();
			loopRateFile << "\t" << loopNowTime - loopStartTime << ", done play mPrimingVideo" << endl;
			loopStartTime = GetTickCount();
			
			//dx may 25, 2012: to make sure the blank screen is not blocking the priming video on the CG side
			showPicPromptDlg_CG.ShowWindow(SW_HIDE);
			showTimerDlg_CG.ShowWindow(SW_HIDE);
			//dx may 23, 2012: show prompt view on CG side
			mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\Movie.wmv\" type MPEGVideo alias myPrimingVideo_CG style popup", NULL, 0, 0);

			loopNowTime = GetTickCount();
			loopRateFile << "\t" << loopNowTime - loopStartTime << ", done CG open myPrimingVideo_CG" << endl;
			loopStartTime = GetTickCount();

			mciSendString_put_CGPromptView("myPrimingVideo_CG");

			loopNowTime = GetTickCount();
			loopRateFile << "\t" << loopNowTime - loopStartTime << ", done CG mciSendString_put_CGPromptView" << endl;
			loopStartTime = GetTickCount();

			if (showPromptInCG)
			{
				mciSendString("play myPrimingVideo_CG", NULL, 0, 0);
				
				loopNowTime = GetTickCount();
				loopRateFile << "\t" << loopNowTime - loopStartTime << ", done CG play myPrimingVideo_CG" << endl;
				loopStartTime = GetTickCount();
			}

			mciSendString("set myPrimingVideo_CG audio all off", NULL, 0, 0);

			loopNowTime = GetTickCount();
			loopRateFile << "\t" << loopNowTime - loopStartTime << ", done CG set audio off" << endl;
			loopStartTime = GetTickCount();
			
			
			initiate_priming_user = false;
			user_being_primed_now = true;

		}
		//Close the priming video if it has finished playing
		if (user_being_primed_now)
		{
			char buffer [128];
			mciSendString("status myPrimingVideo mode", buffer, 128, 0);
			
			loopNowTime = GetTickCount();
			loopRateFile << "\t" << loopNowTime - loopStartTime << ", done status myPrimingVideo mode" << endl;
			loopStartTime = GetTickCount();
			
			if (strcmp (buffer, "stopped") == 0 )
			{	
				//dx may 07, 2012: log priming
				//time(&primingEndTime_l);
				//dx may 09, 2012:
				dLog.Write("prime end");

				////dxdebug may 13, 2012:
				//CString temp;
				//temp.Format("initiate_priming_user = %d, user_being_primed_now = %d, initiate_attention_grabber = %d, waiting_for_attention_grabber_to_finish = %d, free_to_prompt = %d, juststarting = %d"
				//	,initiate_priming_user,user_being_primed_now,initiate_attention_grabber,waiting_for_attention_grabber_to_finish,free_to_prompt,juststarting);
				//dLog.Write(temp);

				//showPicPromptDlg.ShowWindow(SW_SHOW);//dx may 23, 2012: commented out because showPicPromptDlg.ShowWindow(SW_SHOW) is called in ShowPicturePromptImage() already
				//show the blank prompting screen
				ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\blackscreen.bmp");				

				loopNowTime = GetTickCount();
				loopRateFile << "\t" << loopNowTime - loopStartTime << ", pre close mPrimingVideo" << endl;
				loopStartTime = GetTickCount();

				mciSendString("close myPrimingVideo", NULL, 0, 0);
				//dx may 23, 2012: show prompt view on CG side
				mciSendString("close myPrimingVideo_CG", NULL, 0, 0);

				loopNowTime = GetTickCount();
				loopRateFile << "\t" << loopNowTime - loopStartTime << ", done close myPrimingVideo" << endl;
				loopStartTime = GetTickCount();

				
				user_being_primed_now = false;
				time(&lastprompttime);//Note time Priming video was closed
				initiate_attention_grabber = true;
			}
		}

		loopRateFile.close();
	}
}

void CMonitorandPrompt::UseAttentionGrabbers()
{
	
	//skip if state_has changed
	if (state_has_changed)
	{	
		//reset flag needed for priming to occur
		initiate_attention_grabber = false;
		//reset other "state" variables
		waiting_for_attention_grabber_to_finish = false;
		return;
	}

	//skip if not proper time attention grabbers
	if ( (!initiate_attention_grabber)&&(!waiting_for_attention_grabber_to_finish) )
		return;
	
	//skip if user is not using attention grabbers
	if (userSettingsDlg.use_attention_grabbers == DONOTUSEATTENTIONGRABBERS)
	{
		//check if the initiate_attention_grabber flag is set
		if (initiate_attention_grabber)
		{
			//reset
			initiate_attention_grabber = false;
			//set flag for attantion grabber
			//free_to_prompt = true;
			juststarting = true;
		}
		return;
	}
	
	//Show the attention grabber if it is the proper time
	else if (userSettingsDlg.use_attention_grabbers != DONOTUSEATTENTIONGRABBERS)
	{
		//if it is time to show/play the attention grabber, show/play it
		if (initiate_attention_grabber)//initiate_attention_grabber is set to true after priming has finished
		{
			//dx may 07, 2012: log attention grabber
			//time(&attentionStartTime_l);
			//dx may 09, 2012:
			dLog.Write("grabber start");

			////dxdebug may 13, 2012:
			//CString temp;
			//temp.Format("initiate_priming_user = %d, user_being_primed_now = %d, initiate_attention_grabber = %d, waiting_for_attention_grabber_to_finish = %d, free_to_prompt = %d, juststarting = %d"
			//	,initiate_priming_user,user_being_primed_now,initiate_attention_grabber,waiting_for_attention_grabber_to_finish,free_to_prompt,juststarting);
			//dLog.Write(temp);

			//if Sound ONLY...
			if (userSettingsDlg.use_attention_grabbers == USESOUNDONLY)
				PlaySound("C:\\WINDOWS\\Media\\ding.wav", NULL, SND_FILENAME | SND_ASYNC);

			//if Picture ONLY...
			else if (userSettingsDlg.use_attention_grabbers == USEIMAGEONLY)
				ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\TrialSoundsandImages\\Current Trial\\attentiongrabberimage1.jpg");

			//if Both Picture and Sound
			else if (userSettingsDlg.use_attention_grabbers == USESOUNDANDIMAGE)
			{
				ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\TrialSoundsandImages\\Current Trial\\attentiongrabberimage1.jpg");
				PlaySound("C:\\WINDOWS\\Media\\ding.wav", NULL, SND_FILENAME | SND_ASYNC);
			}

			initiate_attention_grabber = false;
			//time(&attention_grabber_time);
			attentionGrabberTime = GetTickCount();
			waiting_for_attention_grabber_to_finish = true;
		}
		//close the attention grabber if it is the proper time
		if (waiting_for_attention_grabber_to_finish)
		{
			//time(&nowtime);
			//dif = difftime(nowtime, attention_grabber_time);
			////ofstream debugFile ("dxdebug.txt", ios::app);
			////debugFile << "attention_grabber_time:" << userState << endl;
			////debugFile << dif << "\t";

			//// see if attention grabber is over
			//if (dif > 1) //dx nov 25, 2012
			////if (dif > 2) //dx sep 18, 2012
			if ((GetTickCount()-attentionGrabberTime)/1000 >= 0.8) //dx nov 25, 2012
			{
				//dx may 09, 2012:
				dLog.Write("grabber end");
			
			////dxdebug may 13, 2012:
			//CString temp;
			//temp.Format("initiate_priming_user = %d, user_being_primed_now = %d, initiate_attention_grabber = %d, waiting_for_attention_grabber_to_finish = %d, free_to_prompt = %d, juststarting = %d"
			//	,initiate_priming_user,user_being_primed_now,initiate_attention_grabber,waiting_for_attention_grabber_to_finish,free_to_prompt,juststarting);
			//dLog.Write(temp);

				waiting_for_attention_grabber_to_finish = false;
				//time(&lastprompttime);//Note time attention grabber finished
				
				//free_to_prompt = true; //dxnote nov 25, 2012: this is wrong, initiate prompting by setting juststarting = true
				juststarting = true;
			}
		}
	}
}


void CMonitorandPrompt::PromptUser()
{
	//skip if state has changed //dxnote may 03, 2012: will also enter UpdatePromptingInterface()
	if (state_has_changed)
	{
		////reset flag needed for priming to occur
		//juststarting = false;
		//free_to_prompt = false;

		////reset other "state" variables
		////user_being_primed_now = false; //dx nov 06, 2012
		return;
	}
	//Functions of this block are to:
	//	close stopped videos
	//	Deliver first prompt
	//	Deliver all subsequent prompts
	//	Only deliver next prompt if it the proper time, and if there are no videos playing

		
	//If state is the same
	if (!state_has_changed)
	{
	
		//If this is the beginning update the picture prompt
		if (juststarting)
		{
			//if (free_to_prompt) //dxnote nov 25, 2012
			//{
				ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\blackscreen.bmp");
				juststarting = false;
				free_to_prompt = true; //dxnote nov 25, 2012
				time(&lastprompttime);//Program just starting
				GiveNextPrompt();
			//}
		}

		//Is it time to give next prompt?
		if ( Time_for_NextPrompt() )
		{
			//PlaySound("C:\\WINDOWS\\Media\\ding.wav", NULL, SND_FILENAME | SND_ASYNC);
			GiveNextPrompt();
		}
	} //dx may 05, 2012: have put everything here into 	if (!state_has_changed){}
}

bool CMonitorandPrompt::Time_for_NextPrompt()
{
	if (free_to_prompt || ((!free_to_prompt) && waitForInitiatePriming) )
	{
		//dx may 05, 2012: put video closing in Time_for_NextPrompt
		//------------------------------------
		char buffer1 [128];
		char buffer2 [128];

		//close any prompting video if it has stopped playing
		mciSendString("status myVideo mode", buffer1, 128, 0);
		if (strcmp (buffer1, "stopped") == 0 )
		{
			ShowPicPrompt();
			mciSendString("close myVideo", NULL, 0, 0);
			//dx may 23, 2012: show prompt view on CG side
			mciSendString("close myVideo_CG", NULL, 0, 0);


			time(&lastprompttime);//Note time that prompting video ended
			//dx may 05, 2012: log prompt end time
			//time(&promptEndTime_l);
			//dx may 09, 2012:
			prompt_ended=true;
			CString s;
			s.Format("prompt end,%d,%d,%d",thisPromptType_l,promptLevel_l,prompt_userState_l);
			dLog.Write(s);
		}
		//If a prompting video is playing, do not Give next prompt
		else if ( strcmp (buffer1, "playing") == 0 )
		{
			return false;
		}

		//close any prompting sound if it has stopped playing
		mciSendString("status VerbalPrompt mode", buffer2, 128, 0);
		if (strcmp (buffer2, "stopped") == 0 )
		{
			mciSendString("close VerbalPrompt", NULL, 0, 0);
			//verbalPromptSound = false;//dx may 13, 2012: not used any where
			time(&lastprompttime);//Note time that verbal prompt ended
			//dx may 05, 2012: log prompt end time
			//time(&promptEndTime_l);
			//dx may 09, 2012:
			prompt_ended=true;
			CString s;
			s.Format("prompt end,%d,%d,%d",thisPromptType_l,promptLevel_l,prompt_userState_l);
			dLog.Write(s);
		}
		//If a prompting audio is playing, do not Give next prompt
		else if ( strcmp (buffer2, "playing") == 0 )
		{
			return false;
		}
		//------------------------------------

		////how much time has gone by since last prompt?
		//if(verbalPromptSound)
		//{
		//	time (&lastprompttime);//Do not allow time to accrue if verbalPromptSound is still active
		//}

		time (&nowtime);
		dif = difftime(nowtime, lastprompttime);//Calculate difference between nowtime and lastprompttime
		//if no prompts have been given yet for this step of the task
		if (numPromptsGivenThisState == 0)//Dec. 5, 2011
		{
			if ( dif >= userSettingsDlg.firstPromptTime)
				return true;
		}

		//if prompts have been given already for this step of the task
		if (numPromptsGivenThisState > 0)//Dec. 5, 2011
		{	
			if ( dif >= userSettingsDlg.nextPromptTime)
				return true;
		}
		return false;
	}
	return false;
}

void CMonitorandPrompt::GiveNextPrompt()
{
	//if no prompt is to be given...
	if (promptLevel == 0)//This should never occur - Justin - January 13, 2012
	{
		dLog.Write("promptLevel == 0!");

		promptLevel ++;
		time(&lastprompttime);//Note time that prompt (actually not a prompt) was issued/ended
		return;
	}
	
	////If the task has been completed...
	//if (taskdone)
	//	return;

	//If maximum number of prompts have been given for the current step...
	if (numPromptsGivenThisState == userSettingsDlg.nPrompts)
	{
		caregiverNeeded = true;
		numPromptsGivenThisState ++;
		//save the promptlevel before the caregiver intervention
		numPromptsGivenForEachState[userState] = numPromptsGivenThisState;
		lastStateBeforeIntervention = userState;
		return;
		//Remember to reset numPromptsGivenThisState after caregiver intervention
	}
	else
	{
		//Which prompttypes has been chosen for the current prompt level?
		
		int thisPromptType;
		//If promptLevel >=5 (used up all prompts), then keep repeat highest level prompt
		if (promptLevel >=5)
			thisPromptType = userSettingsDlg.prompttypes[4];
		else
		{
			thisPromptType = userSettingsDlg.prompttypes[promptLevel-1];
		}

		switch (thisPromptType)
		{
			case 0://verbal prompt
				ShowPicPrompt();
				PlayVerbalPrompt(userState);
				logFile.open(logFileName, ios::app);
				time (&logtime);
				dif = difftime(logtime, trialstartingtime);
				//logFile << dif << "\t" << "\t" << "\t" << "Gave Prompt, ";
				//logFile << "verbal, " << "state = " << userState << endl;
				//logFile.close();
				break;

			case 1://picture only
				ShowPicPrompt();
				time(&lastprompttime);//Note time Picture prompt was shown
				logFile.open(logFileName, ios::app);
				time (&logtime);
				dif = difftime(logtime, trialstartingtime);
				//logFile << dif << "\t" << "\t" << "\t" << "Gave Prompt, ";
				//logFile << "picture only, " << "state = " << userState << endl;
				//logFile.close();
				break;

			case 2://picture + verbal
				ShowPicPrompt();
				PlayVerbalPrompt(userState);
				logFile.open(logFileName, ios::app);
				time (&logtime);
				dif = difftime(logtime, trialstartingtime);
				//logFile << dif << "\t" << "\t" << "\t" << "Gave Prompt, ";
				//logFile << "pic + verbal, " << "state = " << userState << endl;
				//logFile.close();
				break;

			case 3://video - must not have any words - only ambient sounds
				PlayVideoPrompt(userState, false);
				logFile.open(logFileName, ios::app);
				time (&logtime);
				dif = difftime(logtime, trialstartingtime);
				//logFile << dif << "\t" << "\t" << "\t" << "Gave Prompt, ";
				//logFile << "video, " << "state = " << userState << endl;
				//logFile.close();
				break;

			case 4://video+verbal
				PlayVideoPrompt(userState, true);
				logFile.open(logFileName, ios::app);
				time (&logtime);
				dif = difftime(logtime, trialstartingtime);
				//logFile << dif << "\t" << "\t" << "\t" << "Gave Prompt, ";
				//logFile << "video + verbal, " << "state = " << userState << endl;
				//logFile.close();
				////PlayVerbalPrompt(userState);
				break;
		}

		//dx may 05, 2012: log prompt start time, promptLevel, prompt_userState_l
		//time(&promptStartTime_l);
		thisPromptType_l = thisPromptType;
		promptLevel_l = promptLevel;
		prompt_userState_l = userState;
		prompt_started=true;

		CString s;
		s.Format("prompt start,%d,%d,%d",thisPromptType_l,promptLevel_l,prompt_userState_l);
		dLog.Write(s);
		//if (!SCRUBBINGROUTINEHASSTARTED && !RINSINGROUTINEHASSTARTED)//Mar. 3, 2011
		promptLevel ++;
		numPromptsGivenThisState ++;//Dec. 5, 2011 - need to initialize this somewhere
	}
}

//void CMonitorandPrompt::UseAttentionGrabbers()
//{
//	//Is the user using the attention grabbers?  If not, just move on to prompting
//	if (userSettingsDlg.use_attention_grabbers == DONOTUSEATTENTIONGRABBERS)
//	{
//		/*reset all flags that may have been set with the assumption that the user is using attention grabbers,
//		and allow the program to move on to prompting*/
//		
//		initiate_attention_grabber = false;
//		free_to_prompt = true;
//	}
//
//	//Show the attention grabber if it is the proper time
//	else
//	{
//		//if it is time to show/play the attention grabber, show/play it
//		if (initiate_attention_grabber)//initiate_attention_grabber is set to true after priming has finished
//		{
//			//prevent prompts from occurring in case an attention grabber is being issued
//			free_to_prompt = false;
//			
//			//if Sound ONLY...
//			if (userSettingsDlg.use_attention_grabbers == USESOUNDONLY)
//				PlaySound("C:\\WINDOWS\\Media\\ding.wav", NULL, SND_FILENAME | SND_ASYNC);
//
//			//if Picture ONLY...
//			else if (userSettingsDlg.use_attention_grabbers == USEIMAGEONLY)
//				ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\TrialSoundsandImages\\Current Trial\\attentiongrabberimage1.jpg");
//
//			//if Both Picture and Sound
			//else if (userSettingsDlg.use_attention_grabbers == USESOUNDANDIMAGE)
//			{
//				ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\TrialSoundsandImages\\Current Trial\\attentiongrabberimage1.jpg");
//				PlaySound("C:\\WINDOWS\\Media\\ding.wav", NULL, SND_FILENAME | SND_ASYNC);
//			}
//
//			initiate_attention_grabber = false;
//			time(&attention_grabber_time);
//			waiting_for_attention_grabber_to_finish = true;
//		}
//		//close the attention grabber if it is the proper time
//		else if (waiting_for_attention_grabber_to_finish)
//		{
//			time(&nowtime);
//			dif = difftime(nowtime, attention_grabber_time);
//			// see if attention grabber is over
//			if (dif > 2)
//			{
//				waiting_for_attention_grabber_to_finish = false;
//				free_to_prompt = true;
//			}
//		}
//		else //no attention grabbers occurring
//			return;	
//	}
//}


void CMonitorandPrompt::ShowPictureScheduleImage(CString path)
{
	////shows picture schedule at the bottom of the screen
	//IplImage* image;
	//image = cvLoadImage(path);
	if (pictureSchedule != NULL) //dx aug 18, 2012
		cvReleaseImage(&pictureSchedule);
	pictureSchedule = cvLoadImage(path);
	
	//showPicSchedDlg.SetImage(image);	
	showPicSchedDlg.SetImage(pictureSchedule);
	showPicSchedDlg.ShowWindow(SW_SHOW);
	//showPicSchedDlg.Invalidate(FALSE);
	showPicSchedDlg.OnPaint();

	//dx may 23, 2012: prompt views on CG side
	//showPicSchedDlg_CG.SetImage(image);
	showPicSchedDlg_CG.SetImage(pictureSchedule);
	if(showPromptInCG)
		showPicSchedDlg_CG.ShowWindow(SW_SHOW);
	else
		showPicSchedDlg_CG.ShowWindow(SW_HIDE);
	showPicSchedDlg_CG.OnPaint();

	////dx aug 18, 2012: can't release the image here, or else picture schedule might appear black
	////dx may 31, 2012: memory leak issue
	//cvReleaseImage(&image);
}

void CMonitorandPrompt::ShowPicturePromptImage(CString path)
{
	//log to file

	//shows picture schedule at the bottom of the screen
	IplImage* image;
	image = cvLoadImage(path);
	
	//showPicSchedDlg.ShowWindow(SW_HIDE);
	showPicPromptDlg.SetImage(image);
	showPicPromptDlg.ShowWindow(SW_SHOW);
	//showPicPromptDlg.Invalidate(FALSE);
	showPicPromptDlg.OnPaint();

	//dx may 23, 2012: prompt views on CG side
	showPicPromptDlg_CG.SetImage(image);


	if(showPromptInCG)
		showPicPromptDlg_CG.ShowWindow(SW_SHOW);
	else
		showPicPromptDlg_CG.ShowWindow(SW_HIDE);

	showPicPromptDlg_CG.OnPaint();

}

//void CMonitorandPrompt::WaitforButton(){}

void CMonitorandPrompt::wait(float seconds)
{
  clock_t endwait;
  endwait = clock () + seconds * CLOCKS_PER_SEC ;
  while (clock() < endwait) {}
}

void CMonitorandPrompt::PlayVerbalPrompt(int user_state)
{

	//log to file

	switch (user_state)
	{
		case 0:
			if (!wateron)
			{
				//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\turnonandwethands.wav",NULL, SND_FILENAME | SND_ASYNC)
				OpenVerbalPrompt("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\turnonandwethands.wav");
			}
			else
			{
				//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\wetyourhands.wav",NULL, SND_FILENAME | SND_ASYNC);
				OpenVerbalPrompt("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\wetyourhands.wav");
			}
			break;

		case 1:
			{
				//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\getthesoap.wav",NULL, SND_FILENAME | SND_ASYNC);
				//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\putsoapon.wav",NULL, SND_FILENAME | SND_ASYNC);
				OpenVerbalPrompt("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\putsoapon.wav");
			}
			break;

		case 2:
			//if (SCRUBBINGROUTINEHASSTARTED && !RUBBINGHANDSDETECTEDBYOPTICALFLOW)  //dxtodo aug 24, 2012: why play keep on scrubbing when the user is detected to be scrubbing already?! => that's the whole point, "keep scrubbing" if already scrubbing, "scrub your hands" if haven't started scrubbing
			if (!scrubTimer.activityStarted())
			{
				//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\scrubyourhands.wav",NULL, SND_FILENAME | SND_ASYNC);
				OpenVerbalPrompt("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\scrubyourhands.wav");
			}
			else if (!scrubTimer.getActivityDetectedThisFrame())
			{
				//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\keepscrubbing.wav",NULL, SND_FILENAME | SND_ASYNC);
				OpenVerbalPrompt("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\keepscrubbing.wav");
				logFile.open(logFileName, ios::app);
				logFile << "\t" << "\t" << "\t" << "\t" <<"(Scrubbing Reminder)" << endl;
				logFile.close();
			}
			break;

		case 3:
			if (!wateron)
			{
				//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\turnonandrinsehands.wav",NULL, SND_FILENAME | SND_ASYNC);
				OpenVerbalPrompt("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\turnonandrinsehands.wav");
			}
			else if (!rinseTimer.activityStarted())
			{
				////dx debug nov 25, 2012
				//bool a = rinseTimer.activityStarted();
				//bool b = rinseTimer.getActivityDetectedThisFrame();

				//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\rinseyourhands.wav",NULL, SND_FILENAME | SND_ASYNC);
				OpenVerbalPrompt("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\rinseyourhands.wav");
			}
			//else if (RINSINGROUTINEHASSTARTED /*&& hands_away*//*dx aug 31, 2012*/)
			else if (!rinseTimer.getActivityDetectedThisFrame()) //dx nov 25, 2012
			{
				////dxnote may 11, 2012: will this be ever entered? => aug 31, 2012: yes!
				//dLog.Write("verbal prompt, case 3: RINSINGROUTINEHASSTARTED && !hands_away");

				//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\keeprinsing.wav",NULL, SND_FILENAME | SND_ASYNC);
				OpenVerbalPrompt("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\keeprinsing.wav");
				//logFile.open(logFileName, ios::app);
				//logFile << "\t" << "\t" << "\t" << "\t" <<"(Rinsing Reminder)" << endl;
				//logFile.close();
			}
			break;

		case 4:
			if (wateron)
			{
				//if (dried)
				if (dryTimer.getTotalTimerDone())
				{
					//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\turnoffwater2.wav",NULL, SND_FILENAME | SND_ASYNC);
					OpenVerbalPrompt("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\turnoffwater2.wav");
				}
				else
				{
					//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\turnoffanddryhands.wav",NULL, SND_FILENAME | SND_ASYNC);
					OpenVerbalPrompt("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\turnoffanddryhands.wav");
				}
			}
			//else if(!wateron && !dried)
			else if(!dryTimer.getTotalTimerDone())
			{
				//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\dryyourhands.wav",NULL, SND_FILENAME | SND_ASYNC);
				OpenVerbalPrompt("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\dryyourhands.wav");
			}
			else
				//assert(false); //dx nov 25, 2012: i think there isn't any other case, right? =>wrong! when user is finished!
			break;
		case 5:
			OpenVerbalPrompt("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\greatAllDone.wav");
			break;
		case 6: //ALL DONE!
			break;
	}
}

void CMonitorandPrompt::OpenVerbalPrompt(CString path)//Added Feb. 15, 2011
{
	CString temp1 = "open ";
	CString temp2 = "\"";
	CString temp3 = path;
	CString temp4 = "\" ";
	CString temp5 = "type MPEGVideo alias VerbalPrompt";
	temp1.Append(temp2);
	temp1.Append(temp3);
	temp1.Append(temp4);
	temp1.Append(temp5);
	PlaySound(NULL, 0, 0);
	mciSendString(temp1, NULL,0,0);
	mciSendString("play VerbalPrompt from 0", NULL, 0, 0);
	//verbalPromptSound = true;//dx may 13, 2012: not used any where
}

void CMonitorandPrompt::PlayVideoPrompt(int user_state, bool verbal)
{
	//log to file

	//HWND hwnd;
	//buffer = new char; 

	switch (user_state)
	{
		case 0:
			mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\wetvid.wmv\" type MPEGVideo alias myVideo style popup", NULL, 0, 0);
			//mciSendString("put myVideo window at 1024 0 1280 820", NULL, 0, 0);
			//dx april 03, 2012------
			//mciSendString_put_firstScreen("myVideo");
			//dx april 27, 2012 - 2 ----
			mciSendString_put_secondScreen("myVideo");			
			mciSendString("play myVideo", NULL, 0, 0);
			//-----------------------
			
			//dx may 23, 2012: show prompt view on CG side
			mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\wetvid.wmv\" type MPEGVideo alias myVideo_CG style popup", NULL, 0, 0);
			mciSendString_put_CGPromptView("myVideo_CG");
			if (showPromptInCG)
				mciSendString("play myVideo_CG", NULL, 0, 0);
			mciSendString("set myVideo_CG audio all off", NULL, 0, 0);

			//mciSendString("put myVideo window at 1680 0 1280 819", NULL, 0, 0);
			//mciSendString("put myVideo window at 1280 0 1024 614", NULL, 0, 0);
			showPicPromptDlg.ShowWindow(SW_HIDE);
			//dx may 23, 2012: show prompt view on CG side
			showPicPromptDlg_CG.ShowWindow(SW_HIDE);

			if (verbal)
				PlayVerbalPrompt(user_state);

			//mciSendString("close myVideo", NULL, 0, 0);
			//mciSendString("status myVideo mode", buffer, 100, 0);
			break;
		
		case 1:
			mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\soapvid.wmv\" type MPEGVideo alias myVideo style popup", NULL, 0, 0);
			mciSendString_put_secondScreen("myVideo");
			mciSendString("play myVideo", NULL, 0, 0);
			//mciSendString("put myVideo window at 1024 0 1280 820", NULL, 0, 0);
			//dx april 03, 2012------
			//mciSendString_put_firstScreen("myVideo");
			//dx april 27, 2012 - 2 ----
			//-----------------------

			//dx may 23, 2012: show prompt view on CG side
			mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\soapvid.wmv\" type MPEGVideo alias myVideo_CG style popup", NULL, 0, 0);
			mciSendString_put_CGPromptView("myVideo_CG");
			if (showPromptInCG)
				mciSendString("play myVideo_CG", NULL, 0, 0);
			mciSendString("set myVideo_CG audio all off", NULL, 0, 0);

			//mciSendString("put myVideo window at 1680 0 1280 819", NULL, 0, 0);
			//mciSendString("put myVideo window at 1280 0 1024 614", NULL, 0, 0);
			showPicPromptDlg.ShowWindow(SW_HIDE);
			//dx may 23, 2012: show prompt view on CG side
			showPicPromptDlg_CG.ShowWindow(SW_HIDE);

			if (verbal)
				PlayVerbalPrompt(user_state);
			
			break;
		
		case 2:
			//if (SCRUBBINGROUTINEHASSTARTED)
			//	PlayVerbalPrompt(user_state);
			//if (!SCRUBBINGROUTINEHASSTARTED || (SCRUBBINGROUTINEHASSTARTED && hands_away) )//Jan13
			//{
			mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\scrubvid.wmv\" type MPEGVideo alias myVideo style popup", NULL, 0, 0);
			mciSendString_put_secondScreen("myVideo");
			mciSendString("play myVideo from 650", NULL, 0, 0);
			//mciSendString("put myVideo window at 1024 0 1280 820", NULL, 0, 0);
			//dx april 03, 2012------
			//mciSendString_put_firstScreen("myVideo");
			//dx april 27, 2012 - 2 ----
			//-----------------------

			//dx may 23, 2012: show prompt view on CG side
			mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\scrubvid.wmv\" type MPEGVideo alias myVideo_CG style popup", NULL, 0, 0);
			mciSendString_put_CGPromptView("myVideo_CG");
			if (showPromptInCG)
				mciSendString("play myVideo_CG from 650", NULL, 0, 0);
			mciSendString("set myVideo_CG audio all off", NULL, 0, 0);

			//mciSendString("put myVideo window at 1680 0 1280 819", NULL, 0, 0);
			//mciSendString("put myVideo window at 1280 0 1024 614", NULL, 0, 0);
			showPicPromptDlg.ShowWindow(SW_HIDE);
			//dx may 23, 2012: show prompt view on CG side
			showPicPromptDlg_CG.ShowWindow(SW_HIDE);

			if (verbal)
				PlayVerbalPrompt(user_state);
			//}		

			break;
		
		case 3:
			//if (RINSINGROUTINEHASSTARTED)
			//	PlayVerbalPrompt(user_state);
			//if (!RINSINGROUTINEHASSTARTED || (RINSINGROUTINEHASSTARTED && hands_away))
			//if (!RINSINGROUTINEHASSTARTED || (RINSINGROUTINEHASSTARTED && hands_away))
			if (!rinseTimer.activityStarted() || rinseTimer.activityStarted() && !rinseTimer.getActivityDetectedThisFrame())
			{
				mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\rinsevid.wmv\" type MPEGVideo alias myVideo style popup", NULL, 0, 0);
				mciSendString_put_secondScreen("myVideo");
				mciSendString("play myVideo from 650", NULL, 0, 0);
				//mciSendString("put myVideo window at 1024 0 1280 820", NULL, 0, 0);
				//dx april 03, 2012------
				//mciSendString_put_firstScreen("myVideo");
				//dx april 27, 2012 - 2 ----
				//-----------------------

				//dx may 23, 2012: show prompt view on CG side
				mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\rinsevid.wmv\" type MPEGVideo alias myVideo_CG style popup", NULL, 0, 0);
				mciSendString_put_CGPromptView("myVideo_CG");
				if (showPromptInCG)
					mciSendString("play myVideo_CG from 650", NULL, 0, 0);
				mciSendString("set myVideo_CG audio all off", NULL, 0, 0);

				//mciSendString("put myVideo window at 1680 0 1280 819", NULL, 0, 0);
				//mciSendString("put myVideo window at 1280 0 1024 614", NULL, 0, 0);
				showPicPromptDlg.ShowWindow(SW_HIDE);
				//dx may 23, 2012: show prompt view on CG side
				showPicPromptDlg_CG.ShowWindow(SW_HIDE);

				if (verbal)
					PlayVerbalPrompt(user_state);
			}

			break;
		
		case 4:			
			if (wateron)
			{
				//if (dried)
				if (dryTimer.getTotalTimerDone())
				{
					mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\turnoffwatervid.wmv\" type MPEGVideo alias myVideo style popup", NULL, 0, 0);
					mciSendString_put_secondScreen("myVideo");
					mciSendString("play myVideo from 650", NULL, 0, 0);
					//mciSendString("put myVideo window at 1024 0 1280 820", NULL, 0, 0);					
					//dx april 03, 2012------
					//mciSendString_put_firstScreen("myVideo");
					//dx april 27, 2012 - 2 ----
					//-----------------------

					//dx may 23, 2012: show prompt view on CG side
					mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\turnoffwatervid.wmv\" type MPEGVideo alias myVideo_CG style popup", NULL, 0, 0);
					mciSendString_put_CGPromptView("myVideo_CG");
					if (showPromptInCG)
						mciSendString("play myVideo_CG from 650", NULL, 0, 0);
					mciSendString("set myVideo_CG audio all off", NULL, 0, 0);

					//mciSendString("put myVideo window at 1680 0 1280 819", NULL, 0, 0);
					//mciSendString("put myVideo window at 1280 0 1024 614", NULL, 0, 0);
				}
				else
				{
					mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\turnoffwateranddryvid.wmv\" type MPEGVideo alias myVideo style popup", NULL, 0, 0);
					mciSendString_put_secondScreen("myVideo");
					mciSendString("play myVideo from 650", NULL, 0, 0);
					//mciSendString("put myVideo window at 1024 0 1280 820", NULL, 0, 0);
					//dx april 03, 2012------
					//mciSendString_put_firstScreen("myVideo");
					//dx april 27, 2012 - 2 ----
					//-----------------------

					//dx may 23, 2012: show prompt view on CG side
					mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\turnoffwateranddryvid.wmv\" type MPEGVideo alias myVideo_CG style popup", NULL, 0, 0);
					mciSendString_put_CGPromptView("myVideo_CG");
					if (showPromptInCG)
						mciSendString("play myVideo_CG from 650", NULL, 0, 0);
					mciSendString("set myVideo_CG audio all off", NULL, 0, 0);

					//mciSendString("put myVideo window at 1680 0 1280 819", NULL, 0, 0);
					//mciSendString("put myVideo window at 1280 0 1024 614", NULL, 0, 0);
				}
			}
			else//water off and hands not dried
			{
				mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\dryvid.wmv\" type MPEGVideo alias myVideo style popup", NULL, 0, 0);
				mciSendString_put_secondScreen("myVideo");
				mciSendString("play myVideo from 650", NULL, 0, 0);
				//mciSendString("put myVideo window at 1024 0 1280 820", NULL, 0, 0);
				//dx april 03, 2012------
				//mciSendString_put_firstScreen("myVideo");
				//dx april 27, 2012 - 2 ----
				//-----------------------

				//dx may 23, 2012: show prompt view on CG side
				mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Video Prompts\\dryvid.wmv\" type MPEGVideo alias myVideo_CG style popup", NULL, 0, 0);
				mciSendString_put_CGPromptView("myVideo_CG");
				if (showPromptInCG)
					mciSendString("play myVideo_CG from 650", NULL, 0, 0);
				mciSendString("set myVideo_CG audio all off", NULL, 0, 0);

				//mciSendString("put myVideo window at 1680 0 1280 819", NULL, 0, 0);
				//mciSendString("put myVideo window at 1280 0 1024 614", NULL, 0, 0);
			}
						
			showPicPromptDlg.ShowWindow(SW_HIDE);
			//dx may 23, 2012: show prompt view on CG side
			showPicPromptDlg_CG.ShowWindow(SW_HIDE);

			if (verbal)
				PlayVerbalPrompt(user_state);

			break;
		case 5:
			break;
		case 6: //ALL DONE!
			break;
	}
}


//Finite State Machine Model of AutismCOACH - the state machine is implemented in UpdateState() below
//
//State Variable:	userState
//
//
//	State		Transition Condition										New State	Forward/Backward Transition?		Transition Code
//-------------------------------------------------------------------------------------------------------------------------------------------
//
//	0		(wateron) && (nwater > 0) && (!faucetOccluded)					1	Forward											1
//-----------------------------------------------------------------------------------------------------------------------------------
//
//	0		(gettingsoap)													2	Forward											2
//-----------------------------------------------------------------------------------------------------------------------------------
//	1		(gettingsoap)													2	Forward											3
//-----------------------------------------------------------------------------------------------------------------------------------
//	2		SCRUBBINGDETECTED && SCRUBBINGROUTINEHASSTARTED,
//			wateron  && nwater > 0,
//			userSettingsDlg.resoap == 0										1	Backward										4
//-----------------------------------------------------------------------------------------------------------------------------------	
//	2		SCRUBBINGDETECTED && SCRUBBINGROUTINEHASSTARTED,
//			scrub_timer < -1												3	Forward											5
//-----------------------------------------------------------------------------------------------------------------------------------
//	2		SCRUBBINGDETECTED && SCRUBBINGROUTINEHASSTARTED,
//			wateron  && nwater > 0,
//			userSettingsDlg.resoap != 0										3	Forward											6
//-----------------------------------------------------------------------------------------------------------------------------------
//	2		(userSettingsDlg.useButtons == USEBUTTONS) && (userSettingsDlg.useButtons2Input == USEASINPUT),
//			SCRUBBINGROUTINEHASSTARTED,
//			
//-----------------------------------------------------------------------------------------------------------------------------------
//	3		RINSINGROUTINEHASSTARTED,
//			rinse_timer < -1												4	Forward											7
//-----------------------------------------------------------------------------------------------------------------------------------
//	3		(userSettingsDlg.useButtons == USEBUTTONS) && (userSettingsDlg.useButtons2Input == USEASINPUT),
//			RINSINGROUTINEHASSTARTED,
//			
//-----------------------------------------------------------------------------------------------------------------------------------
//	3		RINSINGROUTINEHASSTARTED,
//			(ntowel) && (rinse_timer < userSettingsDlg.rinseTime - 2)		5	Forward											9
//-----------------------------------------------------------------------------------------------------------------------------------
//	4		!wateron,
//			(dried || ntowel > 0)											5	Forward											10
//-----------------------------------------------------------------------------------------------------------------------------------
//	4		(gettingsoap)													3	Backward										8
//-----------------------------------------------------------------------------------------------------------------------------------
//	1-4		userSettingsDlg.startOver == STARTOVER,
//			HandsUNKNOWN(ntowel, nsoap, ntap, nwater, nsink)				0	Backward										11
//-----------------------------------------------------------------------------------------------------------------------------------

void CMonitorandPrompt::UpdateState(int ntowel, int nsoap, int nltap, int nrtap, int nwater, int nsink, int nsoapspout, int numblobsdetected/*numblobsdetected only works for scrubbing and rinsing step*/)
{	
	//dx nov 06, 2012:
	dLog_decision.nObsns.setVars(wateron, faucetOccluded, nwater, nsoap, nsoapspout, RUBBINGHANDSDETECTEDBYOPTICALFLOW, nltap, nrtap, ntowel, towelLifted, nsink);

	//ofstream frameratedata;//January 10, 2012
	int ntap = nltap + nrtap;
	//When state is updated, mark transition and reset promptLevel to 1
	switch (userState)
	{
	case 0: //Starting point

		////dxdebug nov 06, 2012:
		//if (faucetOccluded)
		//	int a = 1;

		//dxnote nov 06, 2012: during calib, select only a small rectangle of the faucet that's at the back of the neck so to avoid the user's reflections (not near the head where the user's reflections are seen, which requires too much faucetNotOccluded cases)
		//dx nov 05, 2012:
		if (!wetTimer.getTotalTimerDone())
			wetTimer.checkTimer(wateron && (nwater>0) && (!faucetOccluded)); //dxnote nov 12, 2012: we aren't checking if nsink == 1 here, so user is allowed to only wet a single hand
		if (wetTimer.getTotalTimerDone())
		{
			userState = 1;
			state_has_changed = true;
			dLog_decision.Write(wetTimer.toStruct(), userState);//log about change in userState
			break;
		}

		////dxdebug nov 05, 2012:
		//if ((nsoap>0) && (nsoapspout>0))
		//	int a = 1;
		if (!soapTimer.getTotalTimerDone())
			soapTimer.checkTimer((nsoap>0) && (nsoapspout>0));
		if (soapTimer.getTotalTimerDone())
		{
			userState = 2;
			state_has_changed = true;
			dLog_decision.Write(soapTimer.toStruct(), userState);//log about userState change
			break;
		}
		
		//dx dec 04, 2012
		//if (wetTimer.getActivityDetectedThisFrame())
		//	time(&lastprompttime); //dx nov 26, 2012: refresh the prompt timer

		break;
			
	case 1: //Hands are Wet
		//dx nov 05, 2012:
		if (!soapTimer.getTotalTimerDone())
			soapTimer.checkTimer((nsoap>0) && (nsoapspout>0));
		if (soapTimer.getTotalTimerDone())
		{
			userState = 2;
			state_has_changed = true;
			dLog_decision.Write(soapTimer.toStruct(), userState);//log about userState change
			break;
		}

		//dx dec 04, 2012
		//if (soapTimer.getActivityDetectedThisFrame())
		//	time(&lastprompttime); //dx nov 26, 2012: refresh the prompt timer

		break;
	
	case 2: //Soapy Hand(s)
		//dx nov 05, 2012:
		if (!scrubTimer.getTotalTimerDone())
			scrubTimer.checkTimer(RUBBINGHANDSDETECTEDBYOPTICALFLOW);
		if (scrubTimer.getTotalTimerDone())
		{
			userState = 3;
			state_has_changed = true;
			dLog_decision.Write(scrubTimer.toStruct(), userState);//log about userState change
			
			//dx dec 01, 2012:
			scrubbingLoop_die = true;
			initializeScrubbingLoop_GRABBING_MODE();
			break;
		}

		//allows rinseTimer to decrement as well
		if (!rinseTimer.getTotalTimerDone())
			rinseTimer.checkTimer(wateron && (nwater>0) && (!faucetOccluded)); //dxnote nov 25, 2012: actually, when rinsing, the faucet blocks the view so most of the times there are two blobs even when the hands are touching under the water
			//rinseTimer.checkTimer(wateron && (nwater>0) && (!faucetOccluded) && numblobsdetected==1); //dx nov 12, 2012: must be rinsing with both hands

		//show count down timer image
		if (!scrubTimer.getTotalTimerDone())
			ShowCountdown(scrubTimer.getRemainingTotalTime(), scrubTimer.checkRemainingTotalTimeLastChecked()); //dxtodo nov 25, 2012

		if (scrubTimer.getActivityDetectedThisFrame())
			time(&lastprompttime); //dx nov 26, 2012: refresh the prompt timer
		
		break;
	
	case 3://Hands have been Scrubbed
		//dx nov 05, 2012:
		//if user got soap again, restart rinsing
		if (!soapTimer.getTotalTimerDone())
			soapTimer.checkTimer((nsoap>0) && (nsoapspout>0));
		if (soapTimer.getTotalTimerDone())
		{
			userState = 3; //restart rinsing
			state_has_changed = true;
			dLog_decision.Write(soapTimer.toStruct(), userState);//log about userState change
			break;
		}

		if (!rinseTimer.getTotalTimerDone())
			rinseTimer.checkTimer(wateron && (nwater>0) && (!faucetOccluded)); //dxnote nov 25, 2012: actually, when rinsing, the faucet blocks the view so most of the times there are two blobs even when the hands are touching under the water
			//rinseTimer.checkTimer(wateron && (nwater>0)); 
			//rinseTimer.checkTimer(wateron && (nwater>0) && (!faucetOccluded) && numblobsdetected==1); //dx nov 12, 2012: must be rinsing with both hands
		if (rinseTimer.getTotalTimerDone())
		{
			userState = 4;
			state_has_changed = true;
			dLog_decision.Write(rinseTimer.toStruct(), userState);//log about change in userState
			break;
		}

		//show count down timer image
		//if (rinseTimer.getUnitTimerDone() && !rinseTimer.getTotalTimerDone()) //dxnote nov 05, 2012: getUnitTimerDone() cannot be used as a flag, it is set to true then to false immediately internally
		if (!rinseTimer.getTotalTimerDone())
			ShowCountdown(rinseTimer.getRemainingTotalTime(), rinseTimer.checkRemainingTotalTimeLastChecked()); //dxtodo nov 25, 2012

		if (rinseTimer.getActivityDetectedThisFrame())
			time(&lastprompttime); //dx nov 26, 2012: refresh the prompt timer

		break;

	case 4://Hands have been Rinsed
		//dx nov 05, 2012:	
		//if user got soap again, restart rinsing
		if (!soapTimer.getTotalTimerDone())
			soapTimer.checkTimer((nsoap>0) && (nsoapspout>0));
		if (soapTimer.getTotalTimerDone())
		{
			userState = 3; //restart rinsing
			state_has_changed = true;
			dLog_decision.Write(soapTimer.toStruct(), userState);//log about userState change
			break;
		}

		if (!dryTimer.getTotalTimerDone())
			//dryTimer.checkTimer((ntowel>0) && towelLifted);
			dryTimer.checkTimer(TOWELINTERACTIONDETECTED && towelLifted); //dx nov 28, 2012
		if (dryTimer.getTotalTimerDone() && (!wateron) )
		{
			userState = 5;
			state_has_changed = true;
			dLog_decision.Write(dryTimer.toStruct(), userState);//log about userState change
			break;
		}
		//dx dec 04, 2012: commented out because we shouldn't refresh dryTimer, or else kid would just keep on playing with the towel and system wouldn't prompt the kid to turn off the water
		//if (dryTimer.getActivityDetectedThisFrame())
		//	time(&lastprompttime); //dx nov 26, 2012: refresh the prompt timer

		break;

	case 5://dx nov 28, 2012: see if kid leaves the sink
		if (!leaveTimer.getTotalTimerDone())
			leaveTimer.checkTimer(nsink == 0);
		if (leaveTimer.getTotalTimerDone())
		{
			userState = 6;
			state_has_changed = true;
			dLog_decision.Write(leaveTimer.toStruct(), userState);//log about userState change
			break;
		}
		
		if (leaveTimer.getActivityDetectedThisFrame())
			time(&lastprompttime); //dx nov 26, 2012: refresh the prompt timer
		
		break;
	case 6://ALL DONE!
		break;
	}
}

void CMonitorandPrompt::UpdatePromptingInterface()
{
	//Take necessary action if the user has completed a step: Update picture schedule and screen,
	//reset data (prompt level, number of prompts given), //save the promptlevel before resetting it
	
	if (!state_has_changed)
		return;
	
	else if (state_has_changed)
	{
		//dx may 07, 2012:
		if(user_being_primed_now)
		{
			//time(&primingEndTime_l);
			//dx may 09, 2012:
			dLog.Write("prime end");
		}
		else if(waiting_for_attention_grabber_to_finish)
		{
			//time(&attentionEndTime_l);
			//dx may 09, 2012:
			dLog.Write("grabber end");
		} 
		else if(prompt_started)
		{
			if(!prompt_ended)
			{
				//time(&promptEndTime_l);
				CString s;
				s.Format("prompt end,%d,%d",promptLevel_l,prompt_userState_l);
				dLog.Write(s);
			}
		}

		//dx oct 29, 2012: reseting the flags to prepare for priming
		//juststartingPriming = true; 
		juststartingPriming = false; //dx dec 04, 2012: should wait for good job prompt to finish before starting priming (this is crucial for userState == 5 since prompt for that starts immediately)
		//dxnote oct 29, 2012: we should set juststartingPriming = true; here instead of relying on reward module to set these flags.  the following comment is invalid: ////dx may 14, 2012: should set initiate_priming_user to false here, since it should be set true after GoodJobSound is set false
		waitForInitiatePriming = false;
		//reset flag needed for priming to occur
		initiate_priming_user = false; 
		//reset other "state" variables
		user_being_primed_now = false;

		initiate_attention_grabber = false;
		waiting_for_attention_grabber_to_finish = false;

		free_to_prompt = false;
		juststarting = false;
		prompt_started=false;
		prompt_ended=false;

		state_has_changed = false;//Added January 11, 2012 - resolves issue A - see page 99 of lab notebook


		//dx may 05, 2012: dx may 07, 2012: logs state start time, end time, this state, next state
		stateStartTime_l = stateEndTime_l;
		time(&stateEndTime_l);
		thisState_l = nextState_l;
		nextState_l = userState;
		//dxtodo may 07, 2012: write state log to file (state changed,old state,new state,waterOn, ntowel, nsoap, nltap, nrtap, nwater, nsink)
		CString s;
		s.Format("state changed,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",thisState_l,nextState_l,waterOn, ntowel, nsoap,nsoapspout, nltap, nrtap, nwater, nsink);
		dLog.Write(s);


		//Close any prompting videos that may be playing, stop any sounds
		mciSendString("close myVideo", NULL, 0, 0);
		mciSendString("close myPrimingVideo", NULL, 0, 0);//January 12, 2012

		//dx may 23, 2012: show prompt view on CG side
		mciSendString("close myVideo_CG", NULL, 0, 0);
		mciSendString("close myPrimingVideo_CG", NULL, 0, 0);//January 12, 2012

		//PlaySound(NULL, 0, 0);
		//mciSendString("close StartOverPrompt", NULL, 0, 0);//dx may 23, 2012: not used
		mciSendString("close GoodJobPrompt", NULL, 0, 0);
		mciSendString("close VerbalPrompt", NULL, 0, 0);
		
		//StartOverSound = false; //dx may 13, 2012: not used any where
		GoodJobSound = false;
		//verbalPromptSound = false;//dx may 13, 2012: not used any where

		time(&lastprompttime);//Note time Priming video was closed - January 12, 2012
		
		//update the picture schedule
		ShowPicSchedule();
		//Show the blank screen
		ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\blackscreen.bmp");
				
		/*if a caregiver intervention has just ended, make sure the correct data gets written to the action module for all steps
		that the caregiver helped with - ie. make sure it is recorded to the action module that 1 prompt (caregiver assistance
		counts as 1 prompt) was needed for each of the steps that the caregiver helped with.*/
		if (careGiverInterventionJustOccurred)
		{
			if(userState - lastStateBeforeIntervention > 1)
			{
				//for each skipped state, set numPromptsGivenForEachState[step] to 1
				for (int i = lastStateBeforeIntervention + 1; i < userState; i++)
				{
					numPromptsGivenForEachState[i] = 1;
				}
			}
			//RewardCompletionofStep();//January 13
		}
		else
		{
			//save the promptlevel before resetting it - in the case of caregiver interventions - this gets done before the intervention
			numPromptsGivenForEachState[lastState] = numPromptsGivenThisState;
		}
		
		//Debugging - Dec. 5, 2011
		ofstream testdata;
		testdata.open("C:\\dxFiles\\COACH4ASDresourceFiles\\Testdata.txt", ofstream::app);
		testdata << "State: " << lastState <<" , ";
		testdata << "numPromptsGivenForEachState[lastState]: " << numPromptsGivenForEachState[lastState] << ", numPromptsGivenThisState: " << numPromptsGivenThisState << endl;
		testdata.close();
		
		//reset the number of prompts given and the prompt level
		numPromptsGivenThisState = 0;
		//dxdebug may 10, 2012:
		//promptLevel = startingPromptLevels[userState];
		promptLevel = 1;
		
		//determine if user has advanced (as opposed to miving backwards in the task)
		if (!careGiverInterventionJustOccurred && userState > lastState)//January 13
			initiate_reward = true;//Modified Dec. 16, 2011
		else
		{
			juststartingPriming = true; //dx dec 04 ,2012: if not going into RewardCompletionofStep, then gotta set juststartingPriming to true or else loop won't continue
			careGiverInterventionJustOccurred = false;
		}


		//update the lastState to the current step
		lastState = userState;

		//dx dec 03, 2012: we need to kill scrubbingLoop_workerThread if it hasn't be killed already
		//kill scrubbingLoop_workerThread and videoWritting_workerThread
		scrubbingLoop_die = true;
		initializeScrubbingLoop_GRABBING_MODE();
		if (pDoc->TScrubbingLoopWorker != NULL)
			WaitForSingleObject(pDoc->TScrubbingLoopWorker->m_hThread, INFINITE); //this wait is needed before killing the prompt loop and start the pause loop because pause loop has its own frame grabbing code, which interfere with recordVideos? in scrubbingLoop_workerThread
		pDoc->TScrubbingLoopWorker = NULL;

		//dx nov 05, 2012: init timers and other vars before entering that state
		switch (userState)
		{
		case 0:
			showTimerDlg.ShowWindow(SW_HIDE);
			showTimerDlg_CG.ShowWindow(SW_HIDE);

			wetTimer.initTimer(userSettingsDlg.wetTime, userSettingsDlg.wetTime);
			soapTimer.initTimer(userSettingsDlg.soapTime, userSettingsDlg.soapTime);
			
			break;


		case 1:
			showTimerDlg.ShowWindow(SW_HIDE);
			showTimerDlg_CG.ShowWindow(SW_HIDE);

			soapTimer.initTimer(userSettingsDlg.soapTime, userSettingsDlg.soapTime);
			
			break;


		case 2:
			showTimerDlg.ResizeLargeImage(FIRSTMONITORWIDTH + SECONDMONITORWIDTH/2 - 100, SECONDMONITORHEIGHT - 200 - SECONDMONITORHEIGHT/5, 200, 200);
			//dx may 23, 2012: show prompt view on CG side
			showTimerDlg_CG.ResizeLargeImage(FIRSTMONITORWIDTH-(FIRSTMONITORWIDTH/2 + 100)*0.2, FIRSTMONITORHEIGHT-(200 + FIRSTMONITORHEIGHT/5)*0.2, 200*0.2, 200*0.2);
			//showTimerDlg_CG.ResizeLargeImage((0 + FIRSTMONITORWIDTH/2 - 100)*0.2, (FIRSTMONITORHEIGHT - 200 - FIRSTMONITORHEIGHT/5)*0.2, 200*0.2, 200*0.2);

			scrubTimer.initTimer(userSettingsDlg.scrubTime, 1);
			rinseTimer.initTimer(userSettingsDlg.rinseTime, 1);
			//assert(!rinseTimer.dontInit); 
			//------------rinseTimer's dontInit should be false except in scrubbing (as set by the next line)-----------
			//===> this is not true! e.g. if in scrubbing and goto scrubbing using pause system
			rinseTimer.dontInit = true;

			//initialize variables to start the worker thread
			RUBBINGHANDSDETECTEDBYOPTICALFLOW = false;
			scrubbingLoop_die = false;
			initializeScrubbingLoop_GRABBING_MODE();
			//start the worker thread
			pDoc->TScrubbingLoopWorker = AfxBeginThread(pDoc->opticalFlowWorker_start, pDoc); 
			//SetThreadPriority(pDoc->TScrubbingLoopWorker->m_hThread, THREAD_PRIORITY_LOWEST);//dxnote: the opticalFlow processing makes video playback choppy due to high cpu usage, thus setting priority of thread to THREAD_PRIORITY_LOWEST might help
			SetThreadPriority(pDoc->TScrubbingLoopWorker->m_hThread, THREAD_PRIORITY_BELOW_NORMAL);
			break;


		case 3:
			showTimerDlg.ResizeLargeImage(FIRSTMONITORWIDTH + 7*SECONDMONITORWIDTH/10 - 100, SECONDMONITORHEIGHT - 200 - SECONDMONITORHEIGHT/5, 200, 200);//Feb. 7, 2011
			//dx may 23, 2012: show prompt view on CG side
			showTimerDlg_CG.ResizeLargeImage(FIRSTMONITORWIDTH-(FIRSTMONITORWIDTH/2 + 100)*0.2, FIRSTMONITORHEIGHT-(200 + FIRSTMONITORHEIGHT/5)*0.2, 200*0.2, 200*0.2);
			//showTimerDlg_CG.ResizeLargeImage((0 + FIRSTMONITORWIDTH/2 - 100)*0.2, (FIRSTMONITORHEIGHT - 200 - FIRSTMONITORHEIGHT/5)*0.2, 200*0.2, 200*0.2);

			soapTimer.initTimer(userSettingsDlg.soapTime, userSettingsDlg.soapTime); //if user got soap again, restarts rinsing			
			if (!rinseTimer.dontInit) //to allow rinseTimer to be counting during scrubbing
				rinseTimer.initTimer(userSettingsDlg.rinseTime, 1); //unitTime is 1 second						
			rinseTimer.dontInit = false;

			break;


		case 4:
			showTimerDlg.ShowWindow(SW_HIDE);
			showTimerDlg_CG.ShowWindow(SW_HIDE);

			soapTimer.initTimer(userSettingsDlg.soapTime, userSettingsDlg.soapTime); //if user got soap again, restarts rinsing			
			//dxtodo nov 05, 2012
			dryTimer.initTimer(userSettingsDlg.dryTime, userSettingsDlg.dryTime);
			//dryTimer.initTimer(1, 1);

			break;


		case 5: //dx nov 28, 2012
			showTimerDlg.ShowWindow(SW_HIDE);
			showTimerDlg_CG.ShowWindow(SW_HIDE);

			soapTimer.initTimer(userSettingsDlg.soapTime, userSettingsDlg.soapTime); //if user got soap again, restarts rinsing			
			leaveTimer.initTimer(2, 2);

			break;

		case 6: //all done
			break;
		default:
			assert(false);//no other cases allowed
			break;
		}
	}
}


void CMonitorandPrompt::ShowCountdown(int imagenumber, bool remainingTimeChanged)
{
	//time (&nowtime); //dxtodo nov 25, 2012
	//dif = difftime(nowtime, lastcounttime);
	//if ( dif >= 1)
	//{
		if (!trainScrubbing_justStarted && !trainScrubbing_scrubbingStarted && !trainScrubbing_nonScrubbingStarted && !trainScrubbing_finished) //dx oct 20, 2012: i.e. if we aren't in trainScrubbing loop. this is necessary since we set scrub_timer to be 1000 (>20) in trainScrubbing loop
		{
			if (imagenumber >=0)
			{
				if ( (imagenumber == (int) userSettingsDlg.nextPromptTime/2 ) && ( userSettingsDlg.scrubTime <= userSettingsDlg.nextPromptTime ) )//issue a scrubbing reminder
					PlayVerbalPrompt(userState);
				//else
				//PlaySound("C:\\WINDOWS\\Media\\ding.wav",NULL, SND_FILENAME | SND_ASYNC);
				IplImage* timerimage;
				CString path = Timerpictures[imagenumber];
				timerimage = cvLoadImage(path);
				//dx may 23, 2012: show prompt view on CG side
				showTimerDlg_CG.SetImage(timerimage);
				if(showPromptInCG)
					showTimerDlg_CG.ShowWindow(SW_SHOW);
				else
					showTimerDlg_CG.ShowWindow(SW_HIDE);
				showTimerDlg_CG.OnPaint();
				if (userSettingsDlg.useCountdown == 0)
				{
					showTimerDlg.SetImage(timerimage);
					showTimerDlg.ShowWindow(SW_SHOW);
					showTimerDlg.OnPaint();
				}
				/*time (&lastcounttime);*///Mar.1, 2011				
				cvReleaseImage(&timerimage); //dx dec 03, 2012: this wasn't there, causing memory leak
			}
		}
		//time (&lastcounttime);
		////imagenumber--; //dx nov 05, 2012: commented out for use by the new decision making code (Timer)
		if (userSettingsDlg.useCountdown == 0 && remainingTimeChanged) //dx nov 05, 2012: added so we don't hear a sound if CG chose not to show count down timer on kid's side
			PlaySound("C:\\WINDOWS\\Media\\ding.wav", NULL, SND_FILENAME | SND_ASYNC);
		//time(&lastprompttime);//January 13, 2012
	//}
}

void CMonitorandPrompt::Setflags()
{
	//scrub_timer = userSettingsDlg.scrubTime;
	//SCRUBBINGROUTINEHASSTARTED = FALSE;
	//rinse_timer = userSettingsDlg.rinseTime;
	//RINSINGROUTINEHASSTARTED = FALSE;
	
	//state_has_changed = true;//commented out January 11, 2012 - this was seen as redundant after resolving issue A - see page 99 of lab notebook
}

void CMonitorandPrompt::RewardCompletionofStep()
{
	////skip if no state change
	//if (state_has_changed)
	//{
	//	//reset flag needed for rewarding to occur
	//	initiate_reward = false;
	//	return;
	//}
	
	//skip if not proper time for rewarding
	if ( (!initiate_reward)&&(!GoodJobSound) )
		return;


	//skip if user not using rewarding
	if (userSettingsDlg.rewardallsteps != REWARDALLSTEPS)
	{
		initiate_reward = false;
		GoodJobSound = false;
		////initiate_priming_user = true;
		juststartingPriming = true;
		return;
	}
	//Reward the user if required
	else if (userSettingsDlg.rewardallsteps == REWARDALLSTEPS)
	{
		//Reward the user if required
		if (initiate_reward)
		{
			//prevent prompts from occurring in case the user is going to be rewarded
			//free_to_prompt = false;//Jan12

			PlaySound(NULL, 0, 0);
			mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\tada_goodjob.wav\" type MPEGVideo alias GoodJobPrompt", NULL, 0, 0);
			mciSendString("play GoodJobPrompt from 0", NULL, 0, 0);
			//PlaySound("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\tada_goodjob.wav",NULL, SND_FILENAME | SND_ASYNC);

			initiate_reward = false;
			GoodJobSound = true;//GoodJobSound is to mark the period of time that good job sound is playing
		}
		//close the "good job" prompt if it has stopped playing
		if(GoodJobSound)
		{
			char buffer [128];
			mciSendString("status GoodJobPrompt mode", buffer, 128, 0);
 			if (strcmp (buffer, "stopped") == 0 )
			{
				mciSendString("close GoodJobPrompt", NULL, 0, 0);
				GoodJobSound = false;
				time(&lastprompttime);//Note time finished rewarding
				//initiate_priming_user = true;
				juststartingPriming = true;
			}
		}
		return;		
	}
}


//dx may 06, 2012: logState()
void CMonitorandPrompt::logState(){
	waterOn_l = waterOn;
	ntowel_l = ntowel;
	nsoap_l = nsoap;
	nsoapspout_l = nsoapspout;
	nltap_l = nltap;
	nrtap_l = nrtap;
	nwater_l = nwater;
	nsink_l = nsink;	
}

void CMonitorandPrompt::reinitialize(void)
{
	//dx nov 26, 2012: set the timers right
	//dxnote nov 26, 2012: ultimately, we wanna use UpdatePromptingInterface() to do this reinitialize() business	
	////state_has_changed = true; //this is to make it enterable in UpdatePromptingInterface
	////UpdatePromptingInterface();
	wetTimer.initTimer(userSettingsDlg.wetTime, userSettingsDlg.wetTime);
	soapTimer.initTimer(userSettingsDlg.soapTime, userSettingsDlg.soapTime);
	scrubTimer.initTimer(userSettingsDlg.scrubTime, 1);
	rinseTimer.initTimer(userSettingsDlg.rinseTime, 1);		
	dryTimer.initTimer(userSettingsDlg.dryTime, userSettingsDlg.dryTime);
	leaveTimer.initTimer(2, 2); //dx nov 28, 2012


//open_and_play_priming_video = false;//Jan12
	juststartingPriming = true;
	waitForInitiatePriming = false;
	initiate_priming_user = false;
	user_being_primed_now = false;

	initiate_attention_grabber = false;
	waiting_for_attention_grabber_to_finish = false;

	//juststarting = true;
	juststarting = false;
	free_to_prompt = false;

	careGiverInterventionJustOccurred = false;
	
	//Initialize userState, lastState to 0, taskCompleted to false
	imnumber = 10;
	wateron = false;
	gettingsoap = false;//Jan 3
	hands_were_at_soap_dispenser_in_last_frame = false;
	hands_were_at_towel_in_last_frame = false;
	
 
	
	userState = 0;
	lastState = 0;
	lastStateBeforeIntervention = 0;
	state_has_changed = false;
	//taskdone = false;
	updated = false;
	caregiverNeeded = false;
	//fwd_state_transition = false;//dx may 13, 2012: not used any where
	hands_away = false;
	initiate_reward = false;
	//taskCompleted = false;

	//SCRUBBINGROUTINEHASSTARTED = false;
	//RINSINGROUTINEHASSTARTED = false;
	RUBBINGHANDSDETECTEDBYOPTICALFLOW = false;//Dec. 21

	startover = false;
	//transitioning = false; //dx may 13, 2012: not used any where
	//ready2prime = false; //dx may 13, 2012: not used any where

	//dried = false;
	skipscrub_n_rinsetracking = false;
	//verbalPromptSound = false;//dx may 13, 2012: not used any where
	GoodJobSound = false;
	//StartOverSound = false; //dx may 13, 2012: not used any where
	
	//dx may 09, 2012:
	prompt_started=false;
	prompt_ended=false;
	//dx may 07, 2012: 
	//observations
	waterOn=false;
	ntowel=0;
	nsoap=0;
	nltap=0;
	nrtap=0;
	nwater=0;
	nsink=0;
	//dx may 06, 2012: logging initialization
	//decision making log
	waterOn_l=false;
	ntowel_l=0;
	nsoap_l=0;
	nltap_l=0;
	nrtap_l=0;
	nwater_l=0;
	nsink_l=0;
	thisState_l=0;
	nextState_l=0;
	stateStartTime_l=0;
	stateEndTime_l=0;
	//prompting log
	promptLevel_l=0;
	prompt_userState_l=0;
	//promptStartTime_l=0;
	//promptEndTime_l=0;
	////priming log
	//primingStartTime_l=0;
	//primingEndTime_l=0;
	////attention grabber log
	//attentionStartTime_l=0;
	//attentionEndTime_l=0;

	/*Set log path*/
	dLog.logPath=LOG_PATH;
}

void CMonitorandPrompt::sessionEndLog(void)
{

	//dx may 07, 2012:
	if(user_being_primed_now)
	{
		//time(&primingEndTime_l);
		//dx may 09, 2012:
		dLog.Write("prime end");
		//reset flag needed for priming to occur
		juststartingPriming = true;
		//reset other "state" variables
		waitForInitiatePriming = false;
		initiate_priming_user = false;
		user_being_primed_now = false;
	}
	else if(waiting_for_attention_grabber_to_finish)
	{
		//time(&attentionEndTime_l);
		//dx may 09, 2012:
		dLog.Write("grabber end");
		initiate_attention_grabber = false;
		waiting_for_attention_grabber_to_finish = false;
	} 
	else if(prompt_started)
	{
		if(!prompt_ended)
		{
			//time(&promptEndTime_l);
			CString s;
			s.Format("prompt end,%d,%d",promptLevel_l,prompt_userState_l);
			dLog.Write(s);
		}
		prompt_started=false;
		prompt_ended=false;
	}



	CString s;
	s.Format("session end,%d",sessionNumber);
	dLog.Write(s);
	//sessionNumber ++; //dx may 12, 2012: don't need to ++ here, everytime session starts, the prev. session number is read and plused one already	
}


int CMonitorandPrompt::doModalUserSettings(int index)
{
	int temp = -1;
	switch (index)
	{
	case 1:
		//temp = settings1.DoModal();
		temp = settings2.DoModal();
		break;

	case 2:
		//temp = settings2.DoModal(); //dx sep 18, 2012
		settings1.nPromptsFromSettings2 = settings2.nPrompts;
		temp = settings1.DoModal();
		break;

	case 3:
		temp = settings3.DoModal();
		break;

	default:
		temp = 0;
		break;
	}
	switch (temp)
	{
	case IDOK:
		return IDOK;
	case IDCANCEL:
		return IDCANCEL;
	case IDC_BUTTON_NEXT:
		return doModalUserSettings(index+1);
	case IDC_BUTTON_PREV:
		return doModalUserSettings(index-1);
	default:
		return temp;
	}
}

//dx dec 01, 2012:
CString CMonitorandPrompt::timeStamp2CStr()
{
	CString s = timeStampStruct.timeStamp.Format("%y-%m-%d_%H-%M-%S");
	CString formatedTime;
	formatedTime.Format("%s-%04d,", s, timeStampStruct.timeStamp_msec%10000);//dx aug 30, 2012: display the last 4 digits of ticks since system start as a way to track the different frames within 1 sec (that's the resolution of CTime)
	//need 4 digits so to deal with 9** becoming 0** within a sec

	return formatedTime;
}
void CMonitorandPrompt::initializeScrubbingLoop_GRABBING_MODE()
{//initialize @ useState update [UpdatePromptingInterface] (when going into userState == 2) and @ changing from processing_mode to grabbing_mode in workerThread
	//note: don't need to call this at OnFileStartpromptingsession, since UpdatePromptingInterface will be called
	bundleCount = 0;
	scrubbingLoop_mode = GRABBING_MODE;	
	scrubbingLoop_clearQueues = true;
}