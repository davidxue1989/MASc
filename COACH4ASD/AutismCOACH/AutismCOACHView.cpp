// AutismCOACHView.cpp : implementation of the CAutismCOACHView class
//
#include "stdafx.h"
#include "AutismCOACH.h"
#include "AutismCOACHDoc.h"
#include "AutismCOACHView.h"
#include <iostream>
#include <Windows.h>
#include <Mmsystem.h>
#include <atltime.h>

#include "VidPlayerDefines.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//dx sep 25, 2012
#define SCRUBTRAIN_TIME_BLANK 5 //dxscrub
#define SCRUBTRAIN_TIME_NOPROMPT 45
#define SCRUBTRAIN_TIME_PROMPT 85


// CAutismCOACHView

IMPLEMENT_DYNCREATE(CAutismCOACHView, CView)

BEGIN_MESSAGE_MAP(CAutismCOACHView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)


	ON_WM_TIMER()
//	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// CAutismCOACHView construction/destruction

CAutismCOACHView::CAutismCOACHView()
//: loopType(0) //dx may 12, 2012: i dont' know wat this is, but it doesn't compile
{
	showFrame = NULL;
	pDoc = NULL;
	//frame = NULL;
}

CAutismCOACHView::~CAutismCOACHView()
{
	//dxnote may 30, 2012: don't need to release showFrame, it wasn't cvCreateImage 'd anyways
	//if (showFrame!=NULL)
	//{
	//	cvReleaseImage(&showFrame);
	//	showFrame = NULL;
	//}
	showFrame = NULL;
	pDoc = NULL;
}

BOOL CAutismCOACHView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CAutismCOACHView drawing

void CAutismCOACHView::OnDraw(CDC* pDC)
{
	CAutismCOACHDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (pDoc->GetCurrentFrame(showFrame))
	{
		//dx may 23, 2012: makes the frames full screen		
		//// display current show image
		////use these openCV functions to show an image in the document window		
		//cvvImage.CopyOf(showFrame);
		//cvvImage.Show(pDC->m_hDC, 0, 0, showFrame->width, showFrame->height, 0, 0);
		//cvvImage.Destroy();

		//dx may 30, 2012 
		//dxnote aug 29, 2012: this is how to display a frame onto the screen
		//dxnote aug 29, 2012: ?is it necessary to use invalidate() to refresh for it to be displayed correctly?
		IplImage  *resizeFrame = NULL;
		CMainFrame* pFrame_temp = (CMainFrame*)AfxGetMainWnd();	
		//RECT rect_temp;
		//pFrame_temp->m_wndToolBar.GetItemRect(1,&rect_temp);
		//LONG barHeight = rect_temp.bottom - rect_temp.top;
		pFrame_temp->updateToolBarHeight(); //dx oct 28, 2012
		LONG barHeight = pFrame_temp->getToolBarHeight(); //dx oct 28, 2012
		int height = FIRSTMONITORHEIGHT-barHeight;
		//int width = (double)height/(pDoc->currentFrame->height)*(pDoc->currentFrame->width);
		int width = (double)height/pDoc->h*pDoc->w; //dx nov 24, 2012: apparently currentFrame becomes messed up (its dimensions are negative) after terminatePrompting, but onDraw is somehow ran again before the prompting loop exits
		resizeFrame = cvCreateImage(cvSize((int)width, (int)height), IPL_DEPTH_8U, 3);



		CSingleLock lock(&pDoc->csVideoFrame);
		lock.Lock();
		cvResize(showFrame, resizeFrame);
		lock.Unlock();
		cvvImage.CopyOf(resizeFrame);
		cvvImage.Show(pDC->m_hDC, 0, 0, resizeFrame->width, resizeFrame->height, 0, 0);
		cvvImage.Destroy();
		
		//cvShowImage("test2", resizeFrame);//dxdebug nov 22, 2012
		//cvWaitKey(1);


		//dx may 30, 2012: memory leak issue //dxnote may 30, 2012: doesn't seem to fix it tho
		cvReleaseImage(&resizeFrame); 
		resizeFrame = NULL;

		////dxdebug may 16, 2012: 
		//CRect a(resizeFrame->width,0,resizeFrame->height,0);
		//bool b = pDC->RectVisible(a);
		//CRect c(showFrame->width-10,10,showFrame->height-10,10);
		//bool d = pDC->RectVisible(c);
		//CRect e(100,100,110,110);
		//bool f = pDC->RectVisible(e);
		//RECT client, window;
		//GetClientRect(&client);
		//GetClientRect(&window);
		//	
		//CString s1, s2;
		//s1.Format("client: t = %d, b = %d, l = %d, r = %d", 
		//	client.top,client.bottom,client.left,client.right);
		//s2.Format("window: t = %d, b = %d, l = %d, r = %d", 
		//	window.top,window.bottom,window.left,client.right);
		//pDoc->monitor_n_Prompt.dLog.Write(s1);
		//pDoc->monitor_n_Prompt.dLog.Write(s2);

	}
}


// CAutismCOACHView printing

BOOL CAutismCOACHView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CAutismCOACHView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CAutismCOACHView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CAutismCOACHView diagnostics

#ifdef _DEBUG
void CAutismCOACHView::AssertValid() const
{
	CView::AssertValid();
}

void CAutismCOACHView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CAutismCOACHDoc* CAutismCOACHView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAutismCOACHDoc)));
	return (CAutismCOACHDoc*)m_pDocument;
}
#endif //_DEBUG


void CAutismCOACHView::OnTimer(UINT_PTR nIDEvent)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	switch (nIDEvent)
	{
	case TIMER_ID_GRAB_IMAGES:
		{
			//pFrame->looptype = TIMER_ID_GRAB_IMAGES;
			pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			pDoc->RetrieveCamImage();//This will invalidate and call ondraw
			cvWaitKey(1);
			InvalidateRect(NULL, FALSE);//Triggers Views OnDraw to show latest captured image
			break;
		}

	case TIMER_ID_TRAINWATERDETECTOR:
		{
			//pFrame->looptype = TIMER_ID_TRAINWATERDETECTOR;
			//Grab an image
			pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			pDoc->RetrieveCamImage();//This will invalidate and call ondraw
			cvWaitKey(1);
			InvalidateRect(NULL, FALSE);//Triggers Views OnDraw to show latest captured image
			pDoc->ProcessOneImage(pDoc->currentFrame, pDoc->skinImg);
			pDoc->GatherTrainingData();
			//WriteNumBlobs2StatusBar(); //dx nov 05, 2012: this function gives me errors when ran in debug, I commented it out		
			//dx nov 22, 2012: commented out recordVideos (since it creates iplimages for writing tasks but never release them because the the images aren't being written to files
			//pDoc->recordVideos("trainWater");//dxnote oct 24, 2012: put this after GatherTrainingData so the currentFrame is not annotated
			pDoc->detailedAnnotation(false, "trainWater");
			break;
		}

	case TIMER_ID_PROMPT:
		{
			//dx sep 20, 2012: loop rate log
			ofstream loopRateFile ("C:\\dxFiles\\COACH4ASDresourceFiles\\loopRate.txt", ios::app);
			DWORD loopNowTime;

			////dxdebug may 01, 2012: trying to see what value logFileName is:
			//pDoc = GetDocument();
			//pDoc->logFile_D.open("C:\\dxFiles\\test.txt", ios::app);
			//pDoc->logFile_D << "logFileName = " << pDoc->logFileName << endl;
			//pDoc->logFile_D.close();
			////conclusion: logFileName is empty.  need to set it to trialLogFile

			//Grab an image
			pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			//dx nov 11, 2012: moved pDoc->prompting = true to here to make sure any button pressed before this wouldn't trigger anything
			//set prompting flag
			pDoc->prompting = true;

			//getting the time stamp for the video frame
			pDoc->monitor_n_Prompt.setTimeStamp(); //saves current time into pDoc->monitor_n_Prompt.timeStampStruct
			pDoc->monitor_n_Prompt.dLog.SetTimeStamp(pDoc->monitor_n_Prompt.timeStampStruct.timeStamp, pDoc->monitor_n_Prompt.timeStampStruct.timeStamp_msec);//dx aug 30, 2012
			pDoc->monitor_n_Prompt.dLog_decision.SetTimeStamp(pDoc->monitor_n_Prompt.timeStampStruct.timeStamp, pDoc->monitor_n_Prompt.timeStampStruct.timeStamp_msec);//dx nov 08, 2012

			if (pDoc->monitor_n_Prompt.userState == 2)
			{
				//things needed to be done for processing scrubbing are done in MT worker thread
			}

			//Tracking and Decision Making
			//See if user has completed the step, update the user's state if they have completed a step, and set state_has_changed flag
			//3. Pass image to tracker and get number of hand blobs in each region
			if (pDoc->monitor_n_Prompt.userState != 2)//dx oct 18, 2012: no image processing during bundling of frames
			{
				loopNowTime = GetTickCount();
				loopRateFile << loopNowTime - loopStartTime << ", loop start, " << pDoc->monitor_n_Prompt.userState << endl;
				loopStartTime = GetTickCount();	

				pDoc->RetrieveCamImage();
				cvWaitKey(1);
				if (pDoc->cameraFramesStampsFile.is_open()) //dxnote dec 01, 2012: could be already closed by videoWriter worker thread as the thread exits
					pDoc->cameraFramesStampsFile << pDoc->monitor_n_Prompt.timeStamp2CStr() << endl;

				loopNowTime = GetTickCount();
				loopRateFile << loopNowTime - loopStartTime << ", done RetrieveCamImage" << endl;
				loopStartTime = GetTickCount();

				pDoc->ProcessOneImage(pDoc->currentFrame, pDoc->skinImg);
				
				loopNowTime = GetTickCount();
				loopRateFile << loopNowTime - loopStartTime << ", done ProcessOneImage" << endl;
				loopStartTime = GetTickCount();				

				//dx nov 28, 2012: use a queue of past results of towel interaction detections to smooth the signal (maybe this will cover up the problem of towel centroid jumping around when using to the flock method)
				pDoc->towelInteractionFramesQueue.push_back(pDoc->ntowel_obs>0);
				while (pDoc->towelInteractionFramesQueue.size() > TOWEL_INTERACTION_DETECTED_QUEUE_SIZE)
				{
					pDoc->towelInteractionFramesQueue.pop_front();
				}
				//count if at least PERCENTAGE_OF_FRAMES_TOWEL_INTERACTING percent of frames in towelInteractionDectectedQueue are true
				pDoc->monitor_n_Prompt.TOWELINTERACTIONDETECTED = (pDoc->percentageTrueInQueue(pDoc->towelInteractionFramesQueue) >= PERCENTAGE_OF_FRAMES_TOWEL_INTERACTING);

				//dx oct 23, 2012: I don't think we'd ever should have the case of trainingWaterDetector == true in prompting loop
				////If training the water detector...
				//if (pDoc->trainingWaterDetector)
				//{
				//	pDoc->GatherTrainingData();
				//	WriteNumBlobs2StatusBar();
				//}

				//4. Else, if not training the water detector, check if water is on
				//else
				//Check if the hands are occluding the taps
				if ( (pDoc->ntapl_obs > 0)||(pDoc->ntapr_obs > 0) )
				{		
					//dx oct 22, 2012
					//pDoc->GetWaterStatusbyWaterFeatures();
					pDoc->monitor_n_Prompt.wateron = false; 
				}
				else
				{
					//If there are no occlusions...
					int left = pDoc->ntapl_obs;
					int right = pDoc->ntapr_obs;
					pDoc->GetWaterStatus(left, right);
				}

				//dx oct 25, 2012:
				pDoc->getFaucetOcclusionStatus();
				//dx oct 25, 2012:
				if (pDoc->monitor_n_Prompt.userState == 4) //in getting towel
					pDoc->getTowelLiftStatus();

				//5. Write number of blobs in each region to main document window's status bar
				//WriteNumBlobs2StatusBar(); //dx nov 05, 2012: this function gives me errors when ran in debug, I commented it out
				//dx may 07, 2012: Decision Making log: updating the nobservation variables in pDoc->monitor_n_Prompt
				WriteNumBlobs2LogVars();

				//6. Check for soap usage, rubbing hands, drying hands
				//dxnote oct 18, 2012: this is where doOpticalFlow is called
				pDoc->CheckHandPositions(pDoc->monitor_n_Prompt.userState, pDoc->currentFrame, pDoc->skinImg);//January 12, 2012
								
				loopNowTime = GetTickCount();
				loopRateFile << loopNowTime - loopStartTime << ", done CheckHandPositions" << endl;
				loopStartTime = GetTickCount();
			}

			//record the annotated video logs
			//dx aug 29, 2012: save hand colour, hand skin, and whole colour image here
			//dxnote aug 29, 2012: need to save the skin image log after ProcessOneImage is called
			if (pDoc->currentFrame != NULL)//dx sep 23, 2012: for reading frames from file, where the frame at end of file has NULL value
				//pDoc->recordVideos("prompt");
				pDoc->detailedAnnotation(true, "prompt");
			InvalidateRect(NULL, FALSE);//Triggers Views OnDraw to show latest captured image


			loopNowTime = GetTickCount();
			loopRateFile << loopNowTime - loopStartTime << ", done recordVideos" << endl;
			loopStartTime = GetTickCount();


			//Play priming video, if needed, or
			//stop and close priming video as soon as it is finished, or as soon as user completes the next step in the task - whichever comes first
			//if (pDoc->monitor_n_Prompt.initiate_priming_user)//January 12, 2012
			pDoc->monitor_n_Prompt.PrimeUser();
			
			loopNowTime = GetTickCount();
			loopRateFile << loopNowTime - loopStartTime << ", done PrimeUser" << endl;
			loopStartTime = GetTickCount();

			//Show flashy image, play sound to get user's sattention before playing prompt
			//Stop and close attention grabber as soon as it is finished, or as soon as user completes the next step in the task - whichever comes first
			pDoc->monitor_n_Prompt.UseAttentionGrabbers();
			
			loopNowTime = GetTickCount();
			loopRateFile << loopNowTime - loopStartTime << ", done UseAttentionGrabbers" << endl;
			loopStartTime = GetTickCount();

			//Prompt the user if it is the proper time, and priming video/attention grabbers are not being shown
			//Close any videos, verbal prompts as soon as they are finished, or as soon as user completes the next step in the task - whichever comes first
			//Make exceptions if user is scrubbing/rinsing - ie. overlay countdown timer on picture/video prompts, do not increment prompt level for reminders,
			//only issue reminder if the user is on task
			pDoc->monitor_n_Prompt.PromptUser();
			
			loopNowTime = GetTickCount();
			loopRateFile << loopNowTime - loopStartTime << ", done PromptUser" << endl;
			loopStartTime = GetTickCount();

			//If maximum number of prompts for the current step have been issued, alert the caregiver for an intervention
			if (pDoc->monitor_n_Prompt.caregiverNeeded /*// || parentpausedsystem)*/ )
				pDoc->CallCaregiver();

#ifndef RECORD_SCRUB_VIDEO_FULL_FPS //dx sep 25, 2012: i.e. if we are NORMAL_PROMPTING
			//7. Update the user's state
			pDoc->monitor_n_Prompt.UpdateState(pDoc->ntowel_obs, pDoc->nsoap_obs, pDoc->ntapl_obs, pDoc->ntapr_obs, pDoc->nwater_obs, pDoc->nsink_obs, pDoc->nsoapspout_obs, pDoc->numblobsdetected);//January 12, 2012			
			//7. Update the user's state
			//pDoc->monitor_n_Prompt.UpdateState(0, 0, 0, 0, 0, 0);//Added in Dec. 13, 2011			
			//If a step has been completed, update the prompting interface

			loopNowTime = GetTickCount();
			loopRateFile << loopNowTime - loopStartTime << ", done UpdateState" << endl;
			loopStartTime = GetTickCount();

			pDoc->monitor_n_Prompt.UpdatePromptingInterface();
			
			loopNowTime = GetTickCount();
			loopRateFile << loopNowTime - loopStartTime << ", done UpdatePromptingInterface" << endl;
			loopStartTime = GetTickCount();

			//Reward the User if a step was just completed
			if (pDoc->monitor_n_Prompt.userState != 6) //dx nov 28, 2012: don't reward if finished leaving the sink
				pDoc->monitor_n_Prompt.RewardCompletionofStep();
			
			loopNowTime = GetTickCount();
			loopRateFile << loopNowTime - loopStartTime << ", done RewardCompletionofStep" << endl;
			loopStartTime = GetTickCount();

			//End prompting if the user has completed all steps in the task
			//if(pDoc->monitor_n_Prompt.userState == 5)
			if(pDoc->monitor_n_Prompt.userState == 6) //dx nov 28, 2012
			{
				//dx nov 28, 2012: don't play goodJobSound at very end.
				////wait until good job sound is closed, then terminate prompting
				//if(!pDoc->monitor_n_Prompt.GoodJobSound && !pDoc->monitor_n_Prompt.initiate_reward)
				//{
					pDoc->TerminatePrompting();
					//dx nov 25, 2012: moved sessionEndLog out of TerminatePrompting so we could have doModal premature stop dialogue logging in between
					////dx may 11, 2012: making the stop and restart function
					pDoc->monitor_n_Prompt.sessionEndLog();
				//}
			}
			//pDoc->monitor_n_Prompt.dLog.Write("test "+pDoc->monitor_n_Prompt.dLog.GetCurrentTimeStamp());//dxdebug aug 30, 2012
#endif

			loopRateFile.close();
			break;
		}

	case TIMER_ID_SYSTEM_PAUSE:
		{	

			//pFrame->looptype = TIMER_ID_SYSTEM_PAUSE;

			pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			pDoc->RetrieveCamImage();//This will invalidate and call ondraw
			cvWaitKey(1);
			//InvalidateRect(NULL, FALSE);//Triggers Views OnDraw to show latest captured image //dxnote dec 02, 2012: pause dlg is shown, no need to show the frames since it will be under the dlg
			
			pDoc->monitor_n_Prompt.dLog.SetTimeStamp(pDoc->monitor_n_Prompt.timeStampStruct.timeStamp, pDoc->monitor_n_Prompt.timeStampStruct.timeStamp_msec);//dx aug 30, 2012
			//dxtodo aug 29, 2012: do the video loggings during pause
			pDoc->ProcessOneImage(pDoc->currentFrame, pDoc->skinImg);
			//pDoc->recordVideos("pause");
			pDoc->detailedAnnotation(true, "pause");

			if (pDoc->monitor_n_Prompt.pauseSystemDlg.systemResume)
			{
				CString rewardEntered;
				CString pauseReason;
				//CString updatedState;

				////Read info from dialog box
				////pDoc->monitor_n_Prompt.pauseSystemDlg.reasonForPause.GetWindowTextA(pauseReason);
				////pDoc->monitor_n_Prompt.pauseSystemDlg.updateToState.GetWindowTextA(updatedState);
				//updatedState = pDoc->monitor_n_Prompt.pauseSystemDlg.currentUserState; //dx oct 28, 2012


				//log to file
				pDoc->logFile_D.open(pDoc->logFileName, ios::app);
				time (&pDoc->monitor_n_Prompt.logtime);
				pDoc->dif = difftime(pDoc->monitor_n_Prompt.logtime, pDoc->monitor_n_Prompt.trialstartingtime);
				pDoc->logFile_D << "\t" << "\t" << "\t" << "\t" << "\t" <<  "\t" << "\t" << "\t" << pauseReason << endl;
				pDoc->logFile_D << pDoc->dif << "\t" << "\t" << "\t" << "\t" << "\t" <<  "\t" << "\t" << "\t" << "System Resumed" << endl;
				pDoc->logFile_D.close();
				
				//Clear CEdit variables
				////pDoc->monitor_n_Prompt.pauseSystemDlg.reasonForPause.SetWindowTextA("");
				////pDoc->monitor_n_Prompt.pauseSystemDlg.updateToState.SetWindowTextA("");
				//pDoc->monitor_n_Prompt.pauseSystemDlg.currentUserState = ""; //dx oct 28, 2012


				//Close window
				pDoc->monitor_n_Prompt.pauseSystemDlg.Close();
				//hide scrub/rinse countdown timer
				pDoc->monitor_n_Prompt.showTimerDlg.ShowWindow(SW_HIDE);
				//dx may 23, 2012: show prompt view on CG side
				pDoc->monitor_n_Prompt.showTimerDlg_CG.ShowWindow(SW_HIDE);

				//update system
				//pDoc->monitor_n_Prompt.userState = atoi(updatedState);
				pDoc->monitor_n_Prompt.userState = pDoc->monitor_n_Prompt.pauseSystemDlg.userState;
				
				//stop thread
				KillTimer(TIMER_ID_SYSTEM_PAUSE);
				//udpate state, (re)set flags accordingly
				pDoc->monitor_n_Prompt.Setflags();
				SetTimer(TIMER_ID_PROMPT, TIMER_ID_CAMERA_PERIOD, 0);
				pDoc->monitor_n_Prompt.caregiverNeeded = false;

				//Reward the child, update info needed by the action module

				//First determine if the child has moved forward
				//if (pDoc->monitor_n_Prompt.userState != pDoc->monitor_n_Prompt.lastState)
				//{
					pDoc->monitor_n_Prompt.state_has_changed = true;
					pDoc->monitor_n_Prompt.careGiverInterventionJustOccurred = true;
					pDoc->monitor_n_Prompt.UpdatePromptingInterface();
				//}//commented out January 11, 2012 - resolves issue A - see page 99 of lab notebook
				
				//reset flag
				pDoc->monitor_n_Prompt.pauseSystemDlg.systemResume = false;

				
				//dxdebug may 10, 2012:
				pDoc->monitor_n_Prompt.free_to_prompt = false;
				pDoc->monitor_n_Prompt.promptLevel=1;
				//---------------
				
				//dx may 09, 2012:
				CString s;
				s.Format("CG pause end,%d,",pDoc->monitor_n_Prompt.userState);
				pDoc->monitor_n_Prompt.dLog.Write(s + pauseReason);
			}
			break;
		}
	case TIMER_ID_STARTTRACKER:
		{
			//pFrame->looptype = TIMER_ID_STARTTRACKER;
			
			pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			pDoc->RetrieveCamImage();//This will invalidate and call ondraw
			cvWaitKey(1);
			InvalidateRect(NULL, FALSE);//Triggers Views OnDraw to show latest captured image
			pDoc->ProcessOneImage(pDoc->currentFrame, pDoc->skinImg);
			//pDoc->recordVideosHelper("tracker");//dxnote sep 12, 2012: gotta put cvWaitKey(1) after changes to currentFrame, or else it won't be displayed
			pDoc->basicAnnotation(false, "tracker");

			////dxdebug aug 31, 2012: just so I can see scrubbing detection annotation during calibration ==> doesn't work, the actual loop speeds are different in here than in TIMER_ID_PROMPT
			//cvZero(pDoc->ShowImg);
			//cvCopy(pDoc->currentFrame, pDoc->ShowImg, pDoc->thresholdImg);
			//pDoc->opticalFlow.DoOpticalFlow(pDoc->ShowImg);//Jan 5 - 2//Jan 7

			break;
		}
	case TIMER_ID_TRAINSCRUBBING://dx sep 16, 2012:
		{
			pDoc = GetDocument();
			ASSERT_VALID(pDoc);

			if (pDoc->monitor_n_Prompt.trainScrubbing_justStarted)
			{
				pDoc->monitor_n_Prompt.state_has_changed = false;
				pDoc->monitor_n_Prompt.userState = 2;

				//pDoc->monitor_n_Prompt.userSettingsDlg.LoadDefaultUserSettings();

				//Prompts to be given at each Prompt Level
				pDoc->monitor_n_Prompt.userSettingsDlg.levelOnePrompt = 2; //so for scrubbing part, it displays the picture
				pDoc->monitor_n_Prompt.userSettingsDlg.levelTwoPrompt = 1;
				pDoc->monitor_n_Prompt.userSettingsDlg.levelThreePrompt = 3;
				pDoc->monitor_n_Prompt.userSettingsDlg.levelFourPrompt = 1; //don't wanna hear scrub your hands all the time
				pDoc->monitor_n_Prompt.userSettingsDlg.levelFivePrompt = 3; //so for nonscrubbing part, it doesn't keep on saying scrub your hands

				//Timing of Prompts
				pDoc->monitor_n_Prompt.userSettingsDlg.firstPromptTime = 0; //set to zero to show priming
				pDoc->monitor_n_Prompt.userSettingsDlg.nextPromptTime  = 1000; //set to a really large number so not to play prompt for now; this ensures more than 15 mins of scrubbing videos

				//Number of Prompts to give
				pDoc->monitor_n_Prompt.userSettingsDlg.nPrompts = 1000; //set to a really large number so not to pause system (doesn't really matter if nextPromptTime is already large

				//Visual Features (Priming and Attention Grabbers)
				pDoc->monitor_n_Prompt.userSettingsDlg.priming = 0; //0 means use priming
				pDoc->monitor_n_Prompt.userSettingsDlg.use_attention_grabbers = 0; //0 means not use attention grabber
				//Rewarding
				pDoc->monitor_n_Prompt.userSettingsDlg.rewardallsteps = 0; //0 means use rewards; for trainScrubbing, this doesn't matter since we won't increment into another userState

				//Scrubbing, rinsing
				pDoc->monitor_n_Prompt.userSettingsDlg.useCountdown = 1; //0 means use countDown
				pDoc->monitor_n_Prompt.userSettingsDlg.scrubTime = 1000; //dxnote oct 20, 2012: this does matter! 
				pDoc->monitor_n_Prompt.userSettingsDlg.rinseTime = 1000;
				//pDoc->monitor_n_Prompt.scrub_timer = 1000; //dx oct 20, 2012: need to set this one actually
				//pDoc->monitor_n_Prompt.rinse_timer = 1000;
				pDoc->monitor_n_Prompt.scrubTimer.initTimer(pDoc->monitor_n_Prompt.userSettingsDlg.scrubTime, pDoc->monitor_n_Prompt.userSettingsDlg.scrubTime); //dx nov 06, 2012: seems like the new timer needs to be init here or else it still remembers the old scrubTime

				//Strictness
				pDoc->monitor_n_Prompt.userSettingsDlg.startOver = 0; //doesn't matter
				pDoc->monitor_n_Prompt.userSettingsDlg.resoap = 0; //doesn't matter


				pDoc->monitor_n_Prompt.userSettingsDlg.prompttypes[0] = pDoc->monitor_n_Prompt.userSettingsDlg.levelOnePrompt;
				pDoc->monitor_n_Prompt.userSettingsDlg.prompttypes[1] = pDoc->monitor_n_Prompt.userSettingsDlg.levelTwoPrompt;
				pDoc->monitor_n_Prompt.userSettingsDlg.prompttypes[2] = pDoc->monitor_n_Prompt.userSettingsDlg.levelThreePrompt;
				pDoc->monitor_n_Prompt.userSettingsDlg.prompttypes[3] = pDoc->monitor_n_Prompt.userSettingsDlg.levelFourPrompt;
				pDoc->monitor_n_Prompt.userSettingsDlg.prompttypes[4] = pDoc->monitor_n_Prompt.userSettingsDlg.levelFivePrompt;

				//Read in starting prompt levels from file
				pDoc->actionModule.SetStartingPromptLevels();
				for (int i=0; i<5; i++)
				{
					pDoc->monitor_n_Prompt.startingPromptLevels[i] = pDoc->actionModule.startingPromptLevel[i]; //set to 5 (most detailed) for the trial's purpose
				}
				pDoc->monitor_n_Prompt.promptLevel = 1;
				pDoc->monitor_n_Prompt.numPromptsGivenThisState = 0;

				pDoc->monitor_n_Prompt.trainScrubbing_justStarted = false;
				pDoc->monitor_n_Prompt.trainScrubbing_scrubbingStarted = true;
				//record current time
				tic = CTime::GetCurrentTime();

				//dxnote sep 24, 2012: very bad idea to try saving structures in binary forms. Hassle to retrieve into the right format, and is very platform dependent
				////dxtest sep 16, 2012: will i be able to save a structure such as array and array implemented list in binary file form?
				////http://www.java2s.com/Tutorial/Cpp/0240__File-Stream/Saveandreadstructure.htm
				////testlog: binary write on 
				////  - array passed; 
				////  - queue passed (even with very large and unknown queue size;
				
				//int myArray [36], myArray2[36];
				//queue<int> myQueue, myQueue2;
				//for (int i=0; i<36; i++)
				//{
				//	myArray[i] = i;
				//	myArray2[i] = i;
				//	myQueue.push(i);
				//}
				//ofstream outFile ("outputTest", ios::out | ios::binary);
				////outFile.write((char *) &myArray, sizeof(myArray)); 
				//outFile.write((char *) &myQueue, sizeof(myQueue));
				//outFile.close();

				//ifstream inFile ("outputTest", ios::in | ios::binary);
				////inFile.read((char *) &myArray2, sizeof(int [36])); //would sizeof(myArray) work?=> yes
				////inFile.read((char *) &myArray2, sizeof(myArray2));
				//inFile.read((char *) &myQueue2, sizeof(myQueue2));
				//inFile.close();

				//ofstream debug ("debugFile.txt", ios::out);
				////debug << "hi";
				//while (!myQueue2.empty())
				//{
				//	debug << myQueue2.front() << endl;
				//	myQueue2.pop();
				//}
				//debug.close();

				//int myArray3 [36];
				//ifstream inFile2 ("outputTest", ios::in | ios::binary);
				//inFile2.read((char *) &myArray3, sizeof(myArray2));
				//inFile2.close();
			}

			if (pDoc->monitor_n_Prompt.trainScrubbing_scrubbingStarted || pDoc->monitor_n_Prompt.trainScrubbing_nonScrubbingStarted)
			{
				
				//check time and change flag if needed
				toc = CTime::GetCurrentTime();
				tspan = toc - tic;

				//do the usual things
				if (tspan.GetTotalSeconds()<SCRUBTRAIN_TIME_BLANK)
				{
					pDoc->opticalFlow.saveDataFrame_scrubTraining = false; //dx oct 20, 2012: stop saving data frames now
					//dx sep 24, 2012: clear opticalFlowDataFrameFile and do nothing 					
					ofstream dataFrameFile ("C:\\dxFiles\\COACH4ASDresourceFiles\\scrub_training_opticalFlowDataFrameFile");
					dataFrameFile.close();
					//pDoc->opticalFlow.dataFrameReady = false; //let the opticalFlow runs over few frames (5 secs) before recording the data
					//dxnote sep 24, 2012: pDoc->opticalFlow.dataFrameReady is true during this time, so opticalFlowDataFrameFile is not being written
				}
				else if (tspan.GetTotalSeconds()>SCRUBTRAIN_TIME_BLANK && tspan.GetTotalSeconds()<=SCRUBTRAIN_TIME_NOPROMPT) //don't start getting the data until 5 secs after to make sure the person is already scrubbing
				{
					pDoc->opticalFlow.saveDataFrame_scrubTraining = true; //dx sep 24, 2012
				}
				else if (tspan.GetTotalSeconds()>SCRUBTRAIN_TIME_NOPROMPT && tspan.GetTotalSeconds()<=SCRUBTRAIN_TIME_PROMPT)
				{
					pDoc->monitor_n_Prompt.userSettingsDlg.nextPromptTime  = 0;
					pDoc->opticalFlow.saveDataFrame_scrubTraining = true; //dx sep 24, 2012
				}
				else if (tspan.GetTotalSeconds()>SCRUBTRAIN_TIME_PROMPT)
				{
					pDoc->opticalFlow.saveDataFrame_scrubTraining = false; //dx oct 20, 2012: stop saving data frames now
					//grab the scrub/nonscrub features training data
					CSingleLock lock(&pDoc->opticalFlow.csDataFrameFile); //dx oct 20, 2012: dxnote sep 24, 2012: this is to ensure when we read the file, the other loop (in DoAngularHistogram) is not writting into the file at the same time
					lock.Lock();

					ifstream inFile ("C:\\dxFiles\\COACH4ASDresourceFiles\\scrub_training_opticalFlowDataFrameFile", ios::in);
					ofstream outFile;
					if (pDoc->monitor_n_Prompt.trainScrubbing_scrubbingStarted)
						outFile.open("C:\\dxFiles\\COACH4ASDresourceFiles\\scrub_training_queueScrubbingFrames", ios::out | ios::app);
					else if (pDoc->monitor_n_Prompt.trainScrubbing_nonScrubbingStarted)
						outFile.open("C:\\dxFiles\\COACH4ASDresourceFiles\\scrub_training_queueNonScrubbingFrames", ios::out | ios::app);
					if (inFile.is_open() && outFile.is_open())
					{
						outFile << inFile.rdbuf(); //copy the whole file
						//outFile << endl; //separate it by an endl
						inFile.close();
						outFile.close();
					}

					lock.Unlock();
					
					if (pDoc->monitor_n_Prompt.trainScrubbing_scrubbingStarted)
					{
						//prepare for nonScrubbing
 						pDoc->monitor_n_Prompt.trainScrubbing_scrubbingStarted = false;
						pDoc->monitor_n_Prompt.trainScrubbing_nonScrubbingStarted = true;
						pDoc->monitor_n_Prompt.state_has_changed = true;
						pDoc->monitor_n_Prompt.userState = 2;
						//pDoc->opticalFlow.dataFrameReady = false; //dx sep 24, 2012:
					}
					else if (pDoc->monitor_n_Prompt.trainScrubbing_nonScrubbingStarted)
					{
						pDoc->monitor_n_Prompt.trainScrubbing_nonScrubbingStarted = false;
						pDoc->monitor_n_Prompt.trainScrubbing_finished = true;
					}
					pDoc->monitor_n_Prompt.userSettingsDlg.nextPromptTime  = 1000;
					tic = CTime::GetCurrentTime();
				}
			}

			if (pDoc->monitor_n_Prompt.trainScrubbing_finished)
			{
				KillTimer(TIMER_ID_TRAINSCRUBBING);
				pDoc->TerminatePrompting();
				pDoc->monitor_n_Prompt.dLog.nonVerbose = false;

				//mine the data and minimizes error using the following parameters:
				//nFrames?{1,2,3,4,5,6,7,8,9,10}, nAngleBins{1, ..., 360}, nDistanceBins{1,2,3,4,5}, nAngleBinsActivated{+/- some int...}
				//try one var change of nAngleBinsActivated, holding others at: 3 nFrames, 36 nAngleBins, 3 nDistanceBins

				//read in the data
				queueScrubbingFrames.clear();
				queueNonScrubbingFrames.clear();
				const char *scrubFileName = "C:\\dxFiles\\COACH4ASDresourceFiles\\scrub_training_queueScrubbingFrames";
				const char *nonScrubFileName = "C:\\dxFiles\\COACH4ASDresourceFiles\\scrub_training_queueNonScrubbingFrames";
				queueScrubbingFrames = getScrubTrainingData(scrubFileName);				
				queueNonScrubbingFrames = getScrubTrainingData(nonScrubFileName);

				//runs through and filter out frames that contain too little (?or too many) corner points?
				
				int nAngleBins=36, nDistanceBins=3;
				deque<int> binDistanceBoundaries(nDistanceBins-1);
				binDistanceBoundaries [0] = 5;
				binDistanceBoundaries [1] = 80;

				//count the nAngleBinsActivated:
				//countNAngleBinsActivated(queueScrubbingFrames, nDistanceBins, nAngleBins, binDistanceBoundaries);				
				//countNAngleBinsActivated(queueNonScrubbingFrames, nDistanceBins, nAngleBins, binDistanceBoundaries);
				pDoc->opticalFlow.countNAngleBinsActivated(queueScrubbingFrames, nDistanceBins, nAngleBins, binDistanceBoundaries); //dx oct 29, 2012
				pDoc->opticalFlow.countNAngleBinsActivated(queueNonScrubbingFrames, nDistanceBins, nAngleBins, binDistanceBoundaries); //dx oct 29, 2012


				//a scheme for 1) minimizing classification error using nAngleBins as inputs, 2) optimizes by changing nAngleBins, nDistanceBins as well as nFrames
				//1)
				//first, let's try to visualize - output to text file, then use matlab
				ofstream out ("C:\\dxFiles\\COACH4ASDresourceFiles\\scrub_training_scrubNAngleBinsActivated.csv", ios::out);
				for (int i=0; i<queueScrubbingFrames.size(); i++)
				{
					for (int j=0; j<nDistanceBins; j++)
					{
						out << queueScrubbingFrames[i].nAngleBinsActivated[j] << ",";
					}
					out << endl;
				}
				out.close();
				out.open("C:\\dxFiles\\COACH4ASDresourceFiles\\scrub_training_nonScrubNAngleBinsActivated.csv", ios::out);
				for (int i=0; i<queueNonScrubbingFrames.size(); i++)
				{
					for (int j=0; j<nDistanceBins; j++)
					{
						out << queueNonScrubbingFrames[i].nAngleBinsActivated[j] << ",";
					}
					out << endl;
				}
				pDoc->monitor_n_Prompt.trainScrubbing_finished = false;
			}

			break;
		}
	}
	CView::OnTimer(nIDEvent);
}

deque<COpticalFlow::FRAME> CAutismCOACHView::getScrubTrainingData(const char *fileName)
{
	deque<COpticalFlow::FRAME> queueFrames;

	ifstream inFile (fileName, ios::in);
	if (inFile.is_open())
	{
		char buffer [256];
		CString str;
		int size = 0;

		while (!inFile.eof())
		{
			COpticalFlow::FRAME frame;

			inFile.getline(buffer, 256, ',');
			if (inFile.eof())
				break;
			str = buffer;
			assert(str.Compare("distanceQueue") == 0);
			inFile.getline(buffer, 256, ',');
			if (inFile.eof())
				break;
			str = buffer;
			size = atoi((LPCTSTR)str);
			int i;
			for (i=0; i<size; i++)
			{
				inFile.getline(buffer, 256, ',');
				if (inFile.eof())
					break;
				str = buffer;
				frame.queueDistances.push_back(atof((LPCTSTR)str));
			}
			if (i<size)
				break; //if we broke from the for loop, need to break from the while loop

			inFile.getline(buffer, 256, ',');
			if (inFile.eof())
				break;
			str = buffer;
			assert(str.Compare("angleQueue") == 0);
			inFile.getline(buffer, 256, ',');
			if (inFile.eof())
				break;
			str = buffer;
			size = atoi((LPCTSTR)str);
			for (i=0; i<size; i++)
			{
				inFile.getline(buffer, 256, ',');
				if (inFile.eof())
					break;
				str = buffer;
				frame.queueAngles.push_back(atof((LPCTSTR)str));
			}
			if (i<size)
				break;

			inFile.getline(buffer, 256);
			if (inFile.eof())
				break;
			str = buffer;
			assert(str.Compare("") == 0);

			queueFrames.push_back(frame);
		}
	}
	return queueFrames;
}


void CAutismCOACHView::WriteNumBlobs2StatusBar()
{
	//dx nov 05, 2012: don't display the statusBar
	////Write the number of hand blobs in each region to the status bar of the main document window - ie:
	////Towel: ,Soap: ,Left Tap: ,Right Tap: ,Water: ,Sink: ,WATER STATUS:
	////Write status string
	//
	////dx nov 05, 2012:
	//CString statusString = "WATER:";
	//if (pDoc->monitor_n_Prompt.wateron)
	//	statusString += "ON";
	//else
	//	statusString += "OFF";
	//statusString += ", Towel:"; 
	//dxnote nov 05, 2012: can't use aString.format("%s ... %d", aString, someInt); this gives unpredictable behaviour http://msdn.microsoft.com/en-us/library/aa314327(v=vs.60).aspx
	//statusString.Format("%s%d", statusString, pDoc->ntowel_obs);
	//statusString += ", Soap Region:";
	//statusString.Format("%s%d", statusString, pDoc->nsoap_obs);
	//statusString += ", Left Tap:";
	//statusString.Format("%s%d", statusString, pDoc->ntapl_obs);
	//statusString += ", Right Tap:";
	//statusString.Format("%s%d", statusString, pDoc->ntapr_obs);
	//statusString += ", Water:";
	//statusString.Format("%s%d", statusString, pDoc->nwater_obs);
	//statusString += ", Sink:";
	//statusString.Format("%s%d", statusString, pDoc->nsink_obs);

	//CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	//pFrame->m_wndStatusBar.SetPaneText(0, statusString);
}

void CAutismCOACHView::WriteNumBlobs2LogVars(){
	pDoc->monitor_n_Prompt.waterOn = pDoc->monitor_n_Prompt.wateron;
	pDoc->monitor_n_Prompt.ntowel = pDoc->ntowel_obs;
	pDoc->monitor_n_Prompt.nsoap = pDoc->nsoap_obs;
	pDoc->monitor_n_Prompt.nltap = pDoc->ntapl_obs;
	pDoc->monitor_n_Prompt.nrtap = pDoc->ntapr_obs;
	pDoc->monitor_n_Prompt.nwater = pDoc->nwater_obs;
	pDoc->monitor_n_Prompt.nsink = pDoc->nsink_obs;
	pDoc->monitor_n_Prompt.nsoapspout = pDoc->nsoapspout_obs;
}
