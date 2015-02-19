// AutismCOACHDoc.h : interface of the CAutismCOACHDoc class
//

#pragma once
#include <iostream>
#include <fstream>
#include <time.h>

#include "HandTracker.h"//Justin
#include "CalibrateDlg.h"
//#include "HandPositionsDlg.h"//Jan 12
#include "UserDataDlg.h"
#include "resource.h"
#include "ShowImageDlg.h"
#include "afxwin.h"
#include <afxmt.h> //for multithreading, CCriticalSection
#include "MonitorandPrompt.h"
#include "TrainWaterDetectorDlg.h"
#include "WaterDetector.h"
#include "BlobResult.h"
#include "OpticalFlow.h"
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat)
#include <opencv2/highgui/highgui.hpp>  // Video write
#include "ActionModule.h"
//#include "StartupMenu.h"
#include "PrematureStopDlg.h"

#include "CLEyeMulticam.h"
//dx oct 23, 2012:
#include "backgroundChangeDetector.h"
//dx nov 24, 2012
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat)
#include <opencv2/highgui/highgui.hpp>  // Video write
using namespace cv;

using namespace std;


//#include <vector>


#define STARTOVER 0
#define REWARDALLSTEPS 0
#define TOTALNUMBEROFSTEPS 5

//dx sep 13, 2012: reduced second monitor to lower resolution for hoping that screen sharing cpu usage would be less
#define FIRSTMONITORWIDTH 1280
#define FIRSTMONITORHEIGHT 720
#define SECONDMONITORWIDTH 1280	
#define SECONDMONITORHEIGHT 800

//dx nov 06, 2012
#define SCRUBBING_IMAGE_BUNDLE_SIZE 7 //dxscrub
#define PERCENTAGE_OF_FRAMES_SCRUBBING 0.5
//dx nov 28, 2012
#define TOWEL_INTERACTION_DETECTED_QUEUE_SIZE 5
#define PERCENTAGE_OF_FRAMES_TOWEL_INTERACTING 0.6


//#define CURRENTFRAME_FROM_FILE
//#define USE_PS3_FOR_CURRENT_FRAME //dxcamera
#define VIDEO_FILE_PATH "C:\\dxFiles\\COACH4ASDresourceFiles\\testLog\\non scrub video.avi"
#define USE_MT_VIDEO_WRITER //multithread worker thread for video writers

void safe_release_image(IplImage*& im);


class CAutismCOACHDoc : public CDocument
{
protected: // create from serialization only
	CAutismCOACHDoc();
	DECLARE_DYNCREATE(CAutismCOACHDoc)


//*************Member variables***********************************************//
public:
	int videoplaytype;
	int ntowel_obs, nsoap_obs, ntapl_obs, ntapr_obs, nwater_obs, nsink_obs, nliquidsoap_obs, nsoapspout_obs; //Aug 19 - Justin
	double dif;
	time_t currenttime, startsoaptime;
	clock_t reinforcing_buttonwaittime;
	
	////deleted - Oct. 18, 2011

	bool camera_running;
	int testCase;
	
	CString Announcement[32];
	CString MainImage[32];
	CString SkinImage[32];
	CString BWImage_left[32];
	CString BWImage_right[32];
	CString BWImageData_left[32];
	CString BWImageData_right[32];
	
protected:
	HINSTANCE hLib;
	CString lastimage, currentimage;

public:
	CUserDataDlg userDataDlg; //Jan 12
	CMonitorandPrompt monitor_n_Prompt;
	CActionModule actionModule;//November 11, 2011 - Justin
	CTrainWaterDetectorDlg trainWaterDetectorDlg;
	CString lh_Obs, rh_Obs, water_Obs;
	ofstream logFile_D;
	ofstream threshdataFile;

protected://Justin
	
	//bool promptingmode;
	bool firstentry;
	bool selected;
	bool calibrating; // true: calib dlg is on

public:
	bool prompting;

	bool announcing;
	bool first_iteration;
	bool trainingWaterDetector;

public:
	CCalibrateDlg calibDlg;

	unsigned int imageWidth;
	unsigned int imageHeight;
	static const int particleCount = NUMBER_OF_PARTICLES;

	//bool waterDetectorIsInitialized;
	//bool actionDetectorIsInitialized;

	void CAutismCOACHDoc::LoadDefaultCalibFile();
	void ResizeWindow(int width, int height);

	CString videoFilename;

	//Justin - Oct 25
	//CBlobResult blobs;
	//vector<CActionScore> actionScores;

	//dx may 30, 2012: moved these to public so currentFrame can be accessed by AutismCOACHView
public:
	//IplImage* image;//(protected) //dx may 30, 2012: not used anywhere
	IplImage* currentFrame; // latest frame grabbed from the camera capture
	IplImage* currentFrameWhole; // latest frame grabbed from the camera captureWhole
	IplImage* skinImg;//Sept 2
protected://Images and Pointers

	IplImage* gray_backgroundImage_left;
	IplImage* gray_backgroundImage_right;
	IplImage* gray_foregroundImage_left;
	IplImage* gray_foregroundImage_right;
	IplImage* gray_currentFrame;
	IplImage* gray_backgroundImg_left;
	IplImage* gray_backgroundImg_right;
	//IplImage* current_grayscale_Img; //dx may 30, 2012: not used anywhere
	IplImage* resized_currentFrame;
	CView* view; //dxnote oct 28, 2012: can't have CAutismCOACHView in CAutismCOACHDoc class due to circular inclusion. that's why we're using CView as the type
	// but we can cast it as CAutismCOACHView * later on when using it. like this:
	//POSITION posV = GetFirstViewPosition();
	//CAutismCOACHView *view = (CAutismCOACHView*) GetNextView(posV);
	//view->myfuntion();

	HandTracker* tracker;
	//IplImage* currentTrackingFrame; // for passing to CView  //dx may 30, 2012: not used anywhere
									// if showing the original image, currentFrame will be passed

									// else, currentTrackingFrame will be passed which will contain skin/particle/... image depending on videoPlayType
	CvSize imgSize;
	double fps;


public:
	unsigned char* finalImageRaw;
	unsigned char* skinImageRaw;
	unsigned char* bestpImageRaw;
	unsigned char* demoImageRaw;
	int* observe;
	CvCapture *capture;
	CvCapture *captureWhole;
	//IplImage* frame; //dx may 30, 2012: not used anywhere
	IplImage* small_currentFrame;
	IplImage* small_skinImg;
	IplImage* thresholdImg;
	IplImage* splitImg;
	IplImage* ShowImg;
	ofstream scrublogFileD;//Jan 5
	CString logFileName;//Jan 12

public:
	bool resized;//has the doc window been resized?//Dec. 20
	ofstream logFile;//Dec. 21
	bool tracker_and_calibration_initialized;//Dec.20

public:
	COpticalFlow opticalFlow;//for Scrubbing Detection
	//CStartupMenu startupMenuDlg;//Dec. 12

//Water and Scrubbing Detection
protected:
	CWaterDetector waterDetector;
	bool waterDetectorIsInitialized;
	CvBox2D32f sinkZone; // automatically detected sink ellipse
	void InitializeWaterDetectorIfNeeded();

public:
	int groundtruth_label;
	bool userPromptedToContinueWaterDetectionTraining;

//******************************************************************************//
//**************************  Operations  **************************************//
//******************************************************************************//

public://Justin
	//Functions that take pointers as arguments
	bool GetCurrentFrame(IplImage*& frame);//April 12, 2010 - Justin
	void GetRegionForeground(int region);//LEFT = 1, RIGHT = 2
	void WriteImageData2File(IplImage* &writeImg, CString filename);
	void GetTapFeatures(IplImage* tapImg, CString side);
	void ShowHandPositions(int l, int r);
	void RetrieveCamImage();//Added April 9 - Justin
	void ProcessOneImage(IplImage *currentFrame, IplImage *skinImg);//April 12, 2010 - Justin	
	void wait(float seconds);
	void TerminatePrompting();
	void CallCaregiver();
	void TransitionToNextState();
	void GetBackground(int region);
	bool IsAlignByte(int count, int width, int widthstep);
	int Magnitude(int number);
	void SaveMainImage();
	void SaveSkinImage();
	void GatherTrainingData();
	void SaveTapImagesandData(CString imagetype);
	void ResetTapFeatures();
	void GetWaterStatus(int numtapleftobs, int numtaprightobs);
	void GetWaterStatusbyWaterFeatures();//Dec. 22, 2010
	void GetButtonInput();//Feb. 11, 2011
	void ReadInMostRecentUserData();//Feb. 18, 2011
protected:
	void ReportHandPositions();
// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
// Implementation
public:
	virtual ~CAutismCOACHDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileStartcamera();//Justin
	afx_msg void OnFileStartpromptingsession();	
	afx_msg void OnFileCalibrate();
	afx_msg void OnFileEnterusersettings();
	afx_msg void OnFilePauseSystem();
	afx_msg void OnFileStarttracker();
	afx_msg void OnFileSwitchImage();
	afx_msg void OnFileToggleWaterStatus();
	afx_msg void OnFileSkipScrubAndRinseDetection();
	afx_msg void OnFileTrainWaterDetector();
	afx_msg void OnFileManuallyIncrementState();
	//dxnote sep 27, 2012: seems like I didn't have these declared as afx_msg prefixed before. don't know what's the difference
	afx_msg void OnFileStoppromptingsession(); 
	afx_msg void OnViewPromptView();
	afx_msg void OnTrainScrubbing();	//dx sep 15, 2012
	

//Added Methods

public:
	void CleanUp();//Dec. 20
	void MTCleanUp();
	void OutputFrameRate();//Dec. 21
	void CheckHandPositions(int userState, IplImage *currentFrame, IplImage *skinImg);//Dec. 21
	void CheckToSeeIfGettingSoap();//Feb. 18
	//void CheckToSeeIfHandsDried();//Feb. 18
	void InitializeTrackerandCalibration();//Dec.20
	
protected:
	void StartTrainingWaterDetector();//Oct.20, 2011
public:
	

	//dxdebug may 31, 2012:
	int debug_count;

	//dx dec 02, 2012:
	void basicAnnotation(bool write2File, string loopState); //timestamp, session number, regions
	void detailedAnnotation(bool write2File, string loopState); //this calls basicAnnotation, also adds: log msg, and CV outputs: nObsvns, scrubbing/rinsing/faucet occlusion/towel lift/towel interaction 
	IplImage *currentFrame_annotationCopy, *skinImg_annotationCopy, *currentFrameWhole_annotationCopy;
	void ShowRegions(string loopState, IplImage *currentFrame, IplImage *skinImg, IplImage *currentFrameWhole);

	//dx aug 30, 2012:
	void closePrompts();
	PrematureStopDlg prematureStopDlg;

	//dx sep 20, 2012: connecting to PS3 CLeye cam
	CLEyeCameraInstance _cam;
	int w, h;
	IplImage *pCapImage;
	PBYTE pCapBuffer;

	//dx aug 29, 2012: video logging - redone	
	CvVideoWriter *writer;
	CvVideoWriter *writerAnnotated;
	CvVideoWriter *writerWhole;
	CvVideoWriter *writerWholeAnnotated;
	CvVideoWriter *writerSkinImg;
	CvVideoWriter *writerSkinImgAnnotated;

#ifdef USE_MT_VIDEO_WRITER
	//dx sep 22, 2012: multithread writeFrame
	void writeFrameTasksQueue_addTask(CvVideoWriter *writer, IplImage *imageFrame);
	static UINT writeFrameWorker_start(LPVOID p);
	CWinThread *TWriteFrameWorker;
	bool writeFrameWorker_die;
	struct writeFrameTask
	{
		CvVideoWriter *writer;
		IplImage *imageFrame;
	};
	writeFrameTask task;
	deque<writeFrameTask> writeFrameTasksQueue;
	CCriticalSection csWriteFrameTasksQueue;
#endif // USE_MT_VIDEO_WRITER


	////dx nov 24, 2012: using the new cv::VideoWriter in opencv-2.4.3
	//VideoWriter	videoWriter;
	//VideoWriter	videoWriterAnnotated;
	//VideoWriter	videoWriterWhole;
	//VideoWriter	videoWriterWholeAnnotated;
	//VideoWriter	videoWriterSkinImg;
	//VideoWriter	videoWriterSkinImgAnnotated;
	//void writeFrameTasksQueue_addTask(CString videoFileName, IplImage *imageFrame); //dx nov 24, 2012: using the new cv::VideoWriter in oepncv-2.4.3
	//struct VideoWriterTask_1Frame
	//{
	//	CString videoFileName;
	//	IplImage *imageFrame;
	//};
	//VideoWriterTask_1Frame videoWriterTask_1Frame;
	//deque<VideoWriterTask_1Frame> videoWriterTasksQueue;
	//CCriticalSection csvideoWriterTasksQueue;
	
	CString videoName, videoAnnotatedName, videoWholeName, videoWholeAnnotatedName, videoSkinImgName, videoSkinImgAnnotatedName;

	CvCapture * fileCapture;
	
	////dxdebug oct 19, 2012:
	//CvVideoWriter *writerSkinImagesBundle;
	//dx oct 19, 2012:
	deque<bool> scrubbingDetectorResults;
	//dx oct 23, 2012:
	backgroundChangeDetector tapDetectorL;
	backgroundChangeDetector tapDetectorR;
	backgroundChangeDetector faucetOcclusionDetector;
	backgroundChangeDetector towelLiftDetector;

	void getFaucetOcclusionStatus();
	void getTowelLiftStatus();

	int numblobsdetected;//dx nov 12, 2012

	//dx nov 28, 2012:
	deque<bool> towelInteractionFramesQueue;

	double percentageTrueInQueue (deque<bool> queue);


	//dx sep 27, 2012: 
	ofstream cameraFramesStampsFile;

	//dx oct 18, 2012:
	deque<IplImage *> imagesBundleForProcessing;
	deque<IplImage *> skinImagesBundleForProcessing;

	//dx dec 01, 2012:
	CWinThread *TScrubbingLoopWorker;	
	static UINT opticalFlowWorker_start(LPVOID p);
	CCriticalSection csVideoFrame;
};