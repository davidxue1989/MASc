// AutismCOACDoc.cpp : implementation of the CAutismCOACHDoc class
//

#include "stdafx.h"
#include "afxwin.h"
#include "AutismCOACH.h"

#include <iostream>
#include <fstream>

#include <stdlib.h>
#include "AutismCOACHDoc.h"
#include "AutismCOACHView.h"
#include "MainFrm.h"
#include "PFHTDefines.h"
#include "VidPlayerDefines.h"
#include <time.h>
#include <MMSystem.h>
#pragma comment(lib, "winmm.lib")
#include <windows.h>
#include <winuser.h>
#include "ParallelPort.h"
#include "BlobResult.h"

using namespace std;



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


void safe_release_image(IplImage*& im)
{
	if (im != NULL)
	{
		cvReleaseImage(&im);
		im = NULL;
	}
}


// CAutismCOACHDoc

IMPLEMENT_DYNCREATE(CAutismCOACHDoc, CDocument)

BEGIN_MESSAGE_MAP(CAutismCOACHDoc, CDocument)
	ON_COMMAND(ID_FILE_STARTCAMERA, &CAutismCOACHDoc::OnFileStartcamera)
	ON_COMMAND(ID_FILE_STARTPROMPTINGSESSION, &CAutismCOACHDoc::OnFileStartpromptingsession)
	ON_COMMAND(ID_FILE_STOPPROMPTINGSESSION, &CAutismCOACHDoc::OnFileStoppromptingsession)
	ON_COMMAND(ID_FILE_CALIBRATE, &CAutismCOACHDoc::OnFileCalibrate)
	ON_COMMAND(ID_FILE_ENTERUSERSETTINGS, &CAutismCOACHDoc::OnFileEnterusersettings)
	ON_COMMAND(ID_FILE_PAUSESYSTEM, &CAutismCOACHDoc::OnFilePauseSystem)
	ON_COMMAND(ID_FILE_STARTTRACKER, &CAutismCOACHDoc::OnFileStarttracker)
	ON_COMMAND(ID_FILE_SWITCHIMAGE, &CAutismCOACHDoc::OnFileSwitchImage)
	ON_COMMAND(ID_FILE_TOGGLEWATERSTATUS, &CAutismCOACHDoc::OnFileToggleWaterStatus)
	ON_COMMAND(ID_FILE_SKIPSCRUBRINSEDETECTION, &CAutismCOACHDoc::OnFileSkipScrubAndRinseDetection)
	ON_COMMAND(ID_FILE_STARTDATACOLLECTION, &CAutismCOACHDoc::OnFileTrainWaterDetector)
	ON_COMMAND(ID_FILE_MANUALLYINCREMENTSTATE, &CAutismCOACHDoc::OnFileManuallyIncrementState)
	ON_COMMAND(ID_VIEW_PROMPTVIEW, &CAutismCOACHDoc::OnViewPromptView)
	ON_COMMAND(ID_TRAINSCRUBBING, &CAutismCOACHDoc::OnTrainScrubbing)

END_MESSAGE_MAP()


// CAutismCOACHDoc construction/destruction

CAutismCOACHDoc::CAutismCOACHDoc()
	: 
	//dx oct 23, 2012:		
	tapDetectorL (REGION_TAP_LEFT, &calibDlg)
	, tapDetectorR (REGION_TAP_RIGHT, &calibDlg)
	//dx oct 25, 2012:
	, faucetOcclusionDetector (REGION_FAUCET, &calibDlg)
	, towelLiftDetector (REGION_TOWEL, &calibDlg)
{

	//dxdebug may 31, 2012:
	debug_count = 0;

	capture = NULL;
	captureWhole = NULL;
	writer = NULL;
	writerAnnotated = NULL;
	writerWhole = NULL;
	writerWholeAnnotated = NULL;
	writerSkinImg = NULL;
	writerSkinImgAnnotated = NULL;

	imageWidth = 640;
	imageHeight = 480;
	
	capture = NULL;
	captureWhole = NULL;
	fileCapture = NULL;

	//deal with pointers here
	//Images
	currentFrame = NULL;
	currentFrame = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 3);
	skinImg = NULL;
	skinImg = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 3);
	currentFrameWhole = NULL;
	currentFrameWhole = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 3);

	//dx dec 02, 2012:
	currentFrame_annotationCopy = NULL;
	currentFrame_annotationCopy = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 3);
	skinImg_annotationCopy = NULL;
	skinImg_annotationCopy = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 3);
	currentFrameWhole_annotationCopy = NULL;
	currentFrameWhole_annotationCopy = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 3);

	resized_currentFrame = NULL;
	resized_currentFrame = cvCreateImage(cvSize(imageWidth/2, imageHeight/2), IPL_DEPTH_8U, 3);

	small_currentFrame = NULL;
	small_currentFrame = cvCreateImage(cvSize(imageWidth/2, imageHeight/2), IPL_DEPTH_8U, 3);//image to pass to the tracker

	small_skinImg = NULL;
	small_skinImg = cvCreateImage(cvSize(imageWidth/2, imageHeight/2), IPL_DEPTH_8U, 3);

	thresholdImg = NULL;
	thresholdImg = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 1);

	splitImg = NULL;
	splitImg = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 1);

	ShowImg = NULL;
	ShowImg = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 3);

	pCapImage = NULL;

	//Images for background subtraction
	gray_backgroundImage_left = NULL;
	gray_backgroundImage_right = NULL;

	gray_foregroundImage_left = NULL;
	gray_foregroundImage_right = NULL;

	gray_currentFrame = NULL;
	
	gray_currentFrame = NULL;
	gray_backgroundImg_left = NULL;
	gray_backgroundImg_right = NULL;

	//Other (ie. for hand tracking and calibration)
	tracker = NULL;//BabakT
	finalImageRaw = NULL;
	skinImageRaw = NULL;
	bestpImageRaw = NULL;
	observe = NULL;
	capture = NULL;
	captureWhole = NULL;


	hLib = LoadLibrary("inpout32.dll");
	/* get the address of the function */
	inp32fp = (inpfuncPtr) GetProcAddress(hLib, "Inp32");
	oup32fp = (oupfuncPtr) GetProcAddress(hLib, "Out32");

	camera_running = false;//Justin April 1
	prompting = false;
	calibrating = false;

	selected = false;
	firstentry = true;

	videoplaytype = 1;

	//Initialize all bool flags/ int counters;
	waterDetectorIsInitialized = false;
	announcing = false;
	first_iteration = true;
	testCase = 0;

	//bools
	resized = false;
	tracker_and_calibration_initialized = false;
	trainingWaterDetector = false;
	userPromptedToContinueWaterDetectionTraining = false;//Feb. 23, 2011

	//startupMenuDlg.DestroyWindow();
	//startupMenuDlg.Create(IDD_STARTUPMENU, NULL);
	//startupMenuDlg.ShowWindow(SW_SHOW);
	//startupMenuDlg.SetPointerToDoc(this);

	//dx sep 20, 2012:
	_cam = NULL;
	w = 0;
	h = 0;
	pCapImage = NULL;
	pCapBuffer = NULL;

	//dx sep 22, 2012:
#ifdef USE_MT_VIDEO_WRITER
	writeFrameWorker_die = false;
	TWriteFrameWorker = NULL;	
#endif //USE_MT_VIDEO_WRITER
	//dx dec 01, 2012:
	TScrubbingLoopWorker = NULL;
}

CAutismCOACHDoc::~CAutismCOACHDoc()
{
	FreeLibrary(hLib);

	safe_release_image(currentFrame);
	safe_release_image(skinImg);
	//dx sep 20, 2012:
	safe_release_image(pCapImage);

	safe_release_image(currentFrameWhole);
	
	//dx dec 02, 2012:
	safe_release_image(currentFrame_annotationCopy);
	safe_release_image(skinImg_annotationCopy);
	safe_release_image(currentFrameWhole_annotationCopy);

	safe_release_image(resized_currentFrame);
	safe_release_image(small_currentFrame);
	safe_release_image(small_skinImg);
	
	//if (capture != NULL) { cvReleaseCapture( &capture );	capture = NULL; }
	//if (writer != NULL) { cvReleaseVideoWriter(&writer); writer = NULL; }
	//if (writerSkinImg != NULL) {	cvReleaseVideoWriter(&writerSkinImg); writerSkinImg = NULL; }

	//if (capture == NULL) { cvReleaseCapture( &capture );	capture = NULL; }
	//if (writer ==NULL) { cvReleaseVideoWriter(&writer); writer = NULL; }
	//if (writerSkinImg == NULL) {	cvReleaseVideoWriter(&writerSkinImg); writerSkinImg = NULL; }

	if (capture != NULL) { 
		cvReleaseCapture( &capture );
		capture = NULL; 
	}
	if (captureWhole != NULL) {
		cvReleaseCapture( &captureWhole );
		captureWhole = NULL; 
	}
	if (fileCapture != NULL) {
		cvReleaseCapture(&fileCapture);
		fileCapture = NULL;
	}

	//dx sep 20, 2012:
#ifdef USE_PS3_FOR_CURRENT_FRAME
	// Destroy camera object
	if (_cam!=NULL)
		CLEyeDestroyCamera(_cam);
	_cam = NULL;
#endif // USE_PS3_FOR_CURRENT_FRAME

#ifdef USE_MT_VIDEO_WRITER
	writeFrameWorker_die = true;
	//dx sep 22, 2012: making sure no residue pointers from this session gets remained onto next session since they'd point to wrong places once everything is newed again
	CSingleLock lock(&csWriteFrameTasksQueue);
	lock.Lock();
	while(!writeFrameTasksQueue.empty())
	{
		writeFrameTask task = writeFrameTasksQueue.front();
		writeFrameTasksQueue.pop_front();
		IplImage *imageFrame = task.imageFrame;				
		cvReleaseImage(&imageFrame);
	}
	lock.Unlock();
#else
	if (writer != NULL) { 
		cvReleaseVideoWriter(&writer); 
		writer = NULL; 
	}
	if (writerAnnotated != NULL) {
		cvReleaseVideoWriter(&writerAnnotated); 
		writerAnnotated = NULL; 
	}
	if (writerWhole != NULL) { 
		cvReleaseVideoWriter(&writerWhole); 
		writerWhole = NULL; 
	}
	if (writerWholeAnnotated != NULL) { 
		cvReleaseVideoWriter(&writerWholeAnnotated); 
		writerWholeAnnotated = NULL; 
	}		
	if (writerSkinImg != NULL) {
		cvReleaseVideoWriter(&writerSkinImg); 
		writerSkinImg = NULL; 
	}
	if (writerSkinImgAnnotated != NULL) {
		cvReleaseVideoWriter(&writerSkinImgAnnotated); 
		writerSkinImgAnnotated = NULL; 
	}	
#endif USE_MT_VIDEO_WRITER
	//videoWriter.release();

	safe_release_image(gray_backgroundImage_left);
	safe_release_image(gray_backgroundImage_right);
	safe_release_image(gray_foregroundImage_left);
	safe_release_image(gray_foregroundImage_right);
	safe_release_image(gray_currentFrame);
	safe_release_image(gray_backgroundImg_left);
	safe_release_image(gray_backgroundImg_right);

	safe_release_image(thresholdImg);
	safe_release_image(splitImg);
	safe_release_image(ShowImg);

	MTCleanUp();
}

BOOL CAutismCOACHDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

// CAutismCOACHDoc serialization

void CAutismCOACHDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CAutismCOACHDoc diagnostics

#ifdef _DEBUG
void CAutismCOACHDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAutismCOACHDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CAutismCOACHDoc commands

void CAutismCOACHDoc::RetrieveCamImage()
{
#ifndef CURRENTFRAME_FROM_FILE //dx sep 25, 2012: i.e. if we are NORMAL_PROMPTING
#ifdef USE_PS3_FOR_CURRENT_FRAME		
	//CHAR _windowName [256];
	//strcpy(_windowName, "test1");
	//cvNamedWindow(_windowName, CV_WINDOW_AUTOSIZE);
	//// image capturing loop
	//while(TRUE)
	//{
	CLEyeCameraGetFrame(_cam, pCapBuffer);
	cvGetImageRawData(pCapImage, &pCapBuffer);
	//convert 4 channel image pCapImage to 3 channel image
	//http://tech.groups.yahoo.com/group/OpenCV/message/43911
	// copy the 3 channels one by one
	for (int channel = 1; channel <= 3; channel++)
	{
		cvSetImageCOI(pCapImage, channel);
		cvSetImageCOI(currentFrame, channel);
		cvCopy(pCapImage, currentFrame);
	}
	// reset the COI (Channel Of Interrest) to all channels
	cvSetImageCOI(pCapImage, 0);
	cvSetImageCOI(currentFrame, 0);

	//	cvShowImage(_windowName, currentFrame);
	//	//cvShowImage(_windowName, pCapImage);
	//	cvWaitKey(1);
	//}
	//cvDestroyWindow(_windowName);
#endif //USE_PS3_FOR_CURRENT_FRAME
#ifndef USE_PS3_FOR_CURRENT_FRAME
	currentFrame = cvQueryFrame(capture);
#endif //!USE_PS3_FOR_CURRENT_FRAME
#endif //!CURRENTFRAME_FROM_FILE
	currentFrameWhole = cvQueryFrame(captureWhole);

#ifdef CURRENTFRAME_FROM_FILE
	currentFrame = cvQueryFrame(fileCapture);
	if (currentFrame == NULL) //dx sep 23, 2012: reached end of file from fileCapture
	{
		TerminatePrompting();
	}
#endif

	//dx dec 02, 2012: commented out to separate the InvalidateRect from frame grabbing
	////cvShowImage("currentimagefromcamera", currentFrame);
	//view->InvalidateRect(NULL, FALSE);//Triggers Views OnDraw to show latest captured image
	///*view->OnDraw(*/
}

void CAutismCOACHDoc::ResizeWindow(int width, int height)
{
	// this function is based on code in here:
	// http://path.berkeley.edu/~zuwhan/OpenCVWin/index.html

	POSITION posV = GetFirstViewPosition();
	view = (CAutismCOACHView*) GetNextView(posV);

	RECT VideoRect, WindowRect, ParentFrameRect;
	GetClientRect(view->m_hWnd, &VideoRect);
	GetWindowRect(view->m_hWnd, &WindowRect);

	//dx jul 09, 2012: is not used
	//int m_BorderWidth = VideoRect.left-WindowRect.left;
	//int m_BorderHeight = VideoRect.top-WindowRect.top;

	int m_WindowDw = (WindowRect.right-WindowRect.left) - (VideoRect.right-VideoRect.left);
	int m_WindowDh = (WindowRect.bottom-WindowRect.top) - (VideoRect.bottom-VideoRect.top);

	int parentFrameDw, parentFrameDh;
	if (view->GetParentFrame()->m_hWnd)
	{
		GetWindowRect(view->GetParentFrame()->m_hWnd, &ParentFrameRect);
		parentFrameDw = (ParentFrameRect.right-ParentFrameRect.left) - (VideoRect.right-VideoRect.left);
		parentFrameDh = (ParentFrameRect.bottom-ParentFrameRect.top) - (VideoRect.bottom-VideoRect.top);
	}

	//dx jul 09, 2012: width and height being the window (with frame) dimensions
	//SetWindowPos(view->m_hWnd, NULL, 0, 0, width+m_WindowDw, height+m_WindowDh,
	//	SWP_NOMOVE | SWP_NOOWNERZORDER);
	SetWindowPos(view->m_hWnd, NULL, 0, 0, width, height,
		SWP_NOMOVE | SWP_NOOWNERZORDER);

	if (view->GetParentFrame()->m_hWnd)
	{
		//dx jul 09, 2012: width and height being the window (with frame) dimensions
		//SetWindowPos(view->GetParentFrame()->m_hWnd, NULL, 0, 0, width + parentFrameDw, height + parentFrameDh,
		//	SWP_NOMOVE | SWP_NOOWNERZORDER);
		SetWindowPos(view->GetParentFrame()->m_hWnd, NULL, 0, 0, width, height,
			SWP_NOMOVE | SWP_NOOWNERZORDER);
	}
	resized = true;//Justin - November 11, 2010
}

void CAutismCOACHDoc::LoadDefaultCalibFile()
{
	if (tracker != NULL)
	{
		if (!calibrating) // make sure calib dlg is not already on
		{
			calibDlg.DestroyWindow();
			//calibDlg.SetImage(currentFrame); // commented. no need for this. we wont' show anything this time.
			calibDlg.SetTracker(tracker); 
			//calibrating = true; 
			//calibDlg.SetFlag(&calibrating);
			calibDlg.SetVideoFilename(videoFilename); // to set the default calib filename
			calibDlg.Create(IDD_CALIBRATE_DIALOG, NULL);
			calibDlg.LoadDefaultCalibFileIfItExists();
			calibDlg.DestroyWindow();
			//calibrating = false; 
		}
	}
}

void CAutismCOACHDoc::OnFileStartcamera()
{

	/*Function Description:
	If the camera has not already been initialized,
	initialize and connect to the camera, capture the first image
	to get the image size and then create the display image
	*/

	//skip if camera already been initialized
	if (camera_running)
		return;
	
	// dx sep 20, 2012: CLEye camera setup
#ifdef USE_PS3_FOR_CURRENT_FRAME	
	// Create camera instance
	//int fps_eye = 75;//dxnote sep 21, 2012: it seems the higher we set the CLEye cam's fps, the less we wait in RetrieveCamImage when grabbing the frame. for CLEYE_VGA, highest fps = 75
	float fps_eye = 40; //dx sep 25, 2012: changed it to a lower value to have better exposure (better colour tracking?)
	//dxnote sep 09, 2012: don't know what's the difference between CLEYE_COLOR_RAW and CLEYE_COLOR_PROCESSED
	//CLEyeCameraInstance _cam = CLEyeCreateCamera(CLEyeGetCameraUUID(0), CLEYE_COLOR_PROCESSED, CLEYE_VGA, fps);	
	_cam = CLEyeCreateCamera(CLEyeGetCameraUUID(0), CLEYE_COLOR_RAW, CLEYE_VGA, fps_eye);	
	CLEyeCameraStart(_cam);		
	
	// Get camera frame dimensions
	CLEyeCameraGetFrameDimensions(_cam, w, h);

	//dx sep 20, 2012:
	//pCapImage = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3); //dxnote sep 09, 2012: camera won't work if it's not 4 channels
	pCapImage = NULL;
	pCapImage = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 4);
#endif

	//grab first image from camera
	//dxshortcut capture
	captureWhole = cvCreateCameraCapture (1); //dxcamera
	capture = cvCreateCameraCapture (1); //dxcamera
	//captureWhole = cvCreateCameraCapture (0); //dxcamera
	//capture = cvCreateCameraCapture (0); //dxcamera

#ifndef USE_PS3_FOR_CURRENT_FRAME
	currentFrame = cvQueryFrame(capture);
	w = currentFrame->width;
	h = currentFrame->height;
	currentFrameWhole = cvQueryFrame(captureWhole);
	assert((currentFrame->width == currentFrameWhole->width) && (currentFrame->height == currentFrameWhole->height));
#endif // !USE_PS3_FOR_CURRENT_FRAME	
	
	fileCapture = cvCreateFileCapture(VIDEO_FILE_PATH);

	////dx may 11, 2012: create video logging handle
	//imgSize.width = currentFrame->width;
	//imgSize.height = currentFrame->height;
	imgSize.width = w; //dx sep 20, 2012
	imgSize.height = h;


	//fps = cvGetCaptureProperty(
	//	capture,
	//	CV_CAP_PROP_FPS
	//	);
	fps=15;
	//fps = 30;

	//CTime t = CTime::GetCurrentTime();
	//CString s = t.Format("%y-%m-%d_%H-%M-%S");
	CString s = monitor_n_Prompt.dLog.GetTimeStamp();
	videoName.Format("C:\\dxFiles\\COACH4ASDresourceFiles\\testLog\\session%d_",monitor_n_Prompt.sessionNumber); //dxnote may 15, 2012: this is the correct new session number, since the number is read from log and +1ed right before entering this function
	videoName += s + ".avi";
	videoAnnotatedName.Format("C:\\dxFiles\\COACH4ASDresourceFiles\\testLog\\session%d_",monitor_n_Prompt.sessionNumber);
	videoAnnotatedName += s + "_Annotated.avi";
	videoWholeName.Format("C:\\dxFiles\\COACH4ASDresourceFiles\\testLog\\session%d_",monitor_n_Prompt.sessionNumber);
	videoWholeName += s + "_WholeImage.avi";
	videoWholeAnnotatedName.Format("C:\\dxFiles\\COACH4ASDresourceFiles\\testLog\\session%d_",monitor_n_Prompt.sessionNumber);
	videoWholeAnnotatedName += s + "_WholeImage_Annotated.avi";
	videoSkinImgName.Format("C:\\dxFiles\\COACH4ASDresourceFiles\\testLog\\session%d_",monitor_n_Prompt.sessionNumber);
	videoSkinImgName += s + "_SkinImage.avi";
	videoSkinImgAnnotatedName.Format("C:\\dxFiles\\COACH4ASDresourceFiles\\testLog\\session%d_",monitor_n_Prompt.sessionNumber);
	videoSkinImgAnnotatedName += s + "_SkinImage_Annotated.avi";

	writer = cvCreateVideoWriter(
		videoName,
		CV_FOURCC('D', 'I', 'V', 'X'),
		//CV_FOURCC('I','4','2','0'), //dx nov 24, 2012
		fps,
		imgSize
		);
	writerAnnotated = cvCreateVideoWriter(
		videoAnnotatedName,
		CV_FOURCC('D', 'I', 'V', 'X'),
		//CV_FOURCC('I','4','2','0'), //dx nov 24, 2012
		fps,
		imgSize
		);


	writerWhole = cvCreateVideoWriter(
		videoWholeName,
		CV_FOURCC('D', 'I', 'V', 'X'),
		//CV_FOURCC('I','4','2','0'), //dx nov 24, 2012
		fps,
		imgSize
		);
	writerWholeAnnotated = cvCreateVideoWriter(
		videoWholeAnnotatedName,
		CV_FOURCC('D', 'I', 'V', 'X'),
		//CV_FOURCC('I','4','2','0'), //dx nov 24, 2012
		fps,
		imgSize
		);


	writerSkinImg = cvCreateVideoWriter(
		videoSkinImgName,
		CV_FOURCC('D', 'I', 'V', 'X'),
		//CV_FOURCC('I','4','2','0'), //dx nov 24, 2012
		fps,
		imgSize
		);		
	writerSkinImgAnnotated = cvCreateVideoWriter(
		videoSkinImgAnnotatedName,
		CV_FOURCC('D', 'I', 'V', 'X'),
		//CV_FOURCC('I','4','2','0'), //dx nov 24, 2012
		fps,
		imgSize
		);

	////dxdebug oct 19, 2012:
	//CString videoName_skinImagesBundle;
	//videoName.Format("C:\\dxFiles\\COACH4ASDresourceFiles\\testLog\\session%d_",monitor_n_Prompt.sessionNumber); //dxnote may 15, 2012: this is the correct new session number, since the number is read from log and +1ed right before entering this function
	//videoName += s + "_testSkinImagesBundle.avi";
	//writerSkinImagesBundle = cvCreateVideoWriter(
	//	videoName,
	//	CV_FOURCC('D', 'I', 'V', 'X'),
	//	fps,
	//	imgSize
	//	);

	////dxdebug may 15, 2012:
	//CString temp;
	//temp.Format("writer = %d, writerSkinImg = %d", writer, writerSkinImg);
	//monitor_n_Prompt.dLog.Write(temp);


	//imageWidth = currentFrame->width; //January 3
	//imageHeight = currentFrame->height; //Janury 3

	//Resize the Document Window
	//dx may 23, 2012: 
	////dx may 16, 2012
	////ResizeWindow(imageWidth, imageHeight);
	////ResizeWindow(FIRSTMONITORWIDTH, FIRSTMONITORHEIGHT);
	//CMainFrame* pFrame_temp = (CMainFrame*)AfxGetMainWnd();
	//RECT rect_temp;
	//pFrame_temp->m_wndToolBar.GetItemRect(1,&rect_temp);
	//LONG barHeight = rect_temp.bottom - rect_temp.top;
	//ResizeWindow(FIRSTMONITORWIDTH, FIRSTMONITORHEIGHT-barHeight);
	ResizeWindow(FIRSTMONITORWIDTH, FIRSTMONITORHEIGHT);

	//Start loop to show images to screen
	view->SetTimer(TIMER_ID_GRAB_IMAGES, TIMER_ID_CAMERA_PERIOD, 0);

	camera_running = true;

}

void CAutismCOACHDoc::InitializeTrackerandCalibration()
{
	if (!tracker_and_calibration_initialized)
	{
		//allocate memory - to send small images to tracker
		tracker = new HandTracker(imageWidth/2, imageHeight/2, particleCount);
		finalImageRaw = new unsigned char[imageWidth/2 * imageHeight/2 * 3];
		skinImageRaw = new unsigned char[imageWidth/2 * imageHeight/2 * 3];
		bestpImageRaw = new unsigned char[imageWidth/2 * imageHeight/2 * 3];

		observe = new int[FRAME_NUM];
		for (int ii=0; ii<FRAME_NUM; ii++) 
		{
			observe[ii] =  0;
		}

		//Load the calibration
		//CString filename = userDataDlg.calibrationFileName;
		CString filename = "C:\\dxFiles\\COACH4ASDresourceFiles\\TrackerCalibrationFile.calib";
		calibDlg.SetTracker(tracker); 
		calibDlg.LoadCalib(filename);

		tracker_and_calibration_initialized = true;
	}
}

struct CleanUpStruct
{
	HandTracker * pHT;
	unsigned char * pFIR;
	unsigned char * pSIR;
	unsigned char * pBIR;
	int * pObs;
};

//clearn up thread
UINT __cdecl CleanUpThread( LPVOID pParam )
{
	CleanUpStruct * pStruc = (CleanUpStruct*)pParam;

	if (pStruc->pHT != NULL) 
	{	
		delete pStruc->pHT;
	}

	if (pStruc->pFIR != NULL)
	{	
		delete [] pStruc->pFIR; 
	}

	if (pStruc->pSIR!= NULL) 
	{ 
		delete [] pStruc->pSIR; 
	}

	if (pStruc->pBIR != NULL) 
	{ 
		delete [] pStruc->pBIR; 
	}

	if (pStruc->pObs != NULL) 
	{ 
		delete [] pStruc->pObs; 
	}



	delete pStruc;

	return 1;
}


void CAutismCOACHDoc::MTCleanUp ()
{
	CleanUpStruct * param = new CleanUpStruct();

	param->pHT=tracker;
	param->pFIR=finalImageRaw;
	param->pSIR=skinImageRaw;
	param->pBIR=bestpImageRaw;
	param->pObs=observe;

	tracker = NULL; 
	finalImageRaw = NULL; 
	skinImageRaw = NULL; 
	bestpImageRaw = NULL; 
	observe = NULL; 

	tracker_and_calibration_initialized = false;

	AfxBeginThread(CleanUpThread,param);
}

void CAutismCOACHDoc::CleanUp()
{
	//deal with pointers here

	//variables for hand tracking and calibration
	if (tracker != NULL) 
	{	
		delete tracker; tracker = NULL; 
	}

	if (finalImageRaw != NULL)
	{	
		delete [] finalImageRaw; finalImageRaw = NULL; 
	}

	if (skinImageRaw!= NULL) 
	{ 
		delete [] skinImageRaw; skinImageRaw = NULL; 
	}

	if (bestpImageRaw != NULL) 
	{ 
		delete [] bestpImageRaw; bestpImageRaw = NULL; 
	}

	if (observe != NULL) 
	{ 
		delete [] observe; observe = NULL; 
	}

	//images for tracking

	tracker_and_calibration_initialized = false;
}

void CAutismCOACHDoc::StartTrainingWaterDetector()
{
	//If system is already prompting, do nothing
	if (prompting || trainingWaterDetector)
		return;

	trainingWaterDetector = true;

	////Initialize the camera and resize the document window
	//OnFileStartcamera();

	////Initialize the Tracker and Calibration, Water Detector
	//MTCleanUp();
	////CleanUp();

	InitializeTrackerandCalibration();
	InitializeWaterDetectorIfNeeded();

	view->KillTimer(TIMER_ID_GRAB_IMAGES);
	view->KillTimer(TIMER_ID_STARTTRACKER);
	view->SetTimer(TIMER_ID_TRAINWATERDETECTOR, TIMER_ID_CAMERA_PERIOD, 0);
}

void CAutismCOACHDoc::OnFileStartpromptingsession()
{
	monitor_n_Prompt.pDoc = this; //dx dec 01, 2012

	//If system is already prompting, do nothing
	if (prompting)
	{
		return;
	}

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->OnFileStartpromptingsession();
	pFrame->disableToolBarButtons(true); //dx oct 28, 2012: disable the buttons till the camera is ready (i.e. system stops being unresponsive)


	//Load scrubbing detection feature weights from file - needed to identify scrubbing
	ifstream scrubweightFile("C:\\dxFiles\\COACH4ASDresourceFiles\\scrub_training_Scrubweights.txt");
	string line;
	if (scrubweightFile.is_open())
	{
		for (int i = 0; i < 4; i++)
		{
			getline(scrubweightFile, line);
			opticalFlow.scrubweights[i] = atof(line.c_str());
		}
	}
	scrubweightFile.close();

	//Load water detection feature weights from file - needed to identify when taps are in "on" position
	//First load left tap features
	ifstream lefttapfeatureweightsFile("C:\\dxFiles\\COACH4ASDresourceFiles\\LeftTapfeatureweights.txt");
	if (lefttapfeatureweightsFile.is_open())
	{
		for (int i = 0; i < 8; i++)
		{
			getline(lefttapfeatureweightsFile, line);
			waterDetector.lefttapfeatureweight[i] = atof(line.c_str());
		}
	}
	lefttapfeatureweightsFile.close();
	//Now load right tap features
	ifstream righttapfeatureweightsFile("C:\\dxFiles\\COACH4ASDresourceFiles\\RightTapfeatureweights.txt");
	if (righttapfeatureweightsFile.is_open())
	{
		for (int i = 0; i < 8; i++)
		{
			getline(righttapfeatureweightsFile, line);
			waterDetector.righttapfeatureweight[i] = atof(line.c_str());
		}
	}
	righttapfeatureweightsFile.close();

	//dxnote oct 25, 2012: don't need to loadFeatureWeights here, since it'll be loaded in predictProbabilityOfBackgroundChange
	////oct 25, 2012:
	//tapDetectorL.loadFeatureWeights();
	//tapDetectorR.loadFeatureWeights();
	//faucetOcclusionDetector.loadFeatureWeights();
	//towelLiftDetector.loadFeatureWeights();

	//Read in the most recent user data from file and show to screen in a modal dialog box
	//ReadInMostRecentUserData();

	//Load previous (default) User Settings if not already done by user
	if(!monitor_n_Prompt.userSettingsDlg.alreadyentered_usersettings)
		monitor_n_Prompt.userSettingsDlg.LoadDefaultUserSettings();

	//Save the user settings for this trial
	monitor_n_Prompt.userSettingsDlg.SaveUserSettingsForTrial(userDataDlg.trialUserSettings);

	//Prepare to log the information for this trial
	//All 3 strings below (logFileName, monitor_n_Prompt.logFileName, monitor_n_Prompt.userSettingsDlg.logFileName) are identical
	//They are all needed in order to be able to write to file from:
	//AutismCOACHDOC.cpp, MonitorandPrompt.cpp and UserSettingsDlg.cpp

	//logFileName = userDataDlg.trialLogFile;
	//monitor_n_Prompt.logFileName = userDataDlg.trialLogFile;
	//monitor_n_Prompt.userSettingsDlg.logFileName = userDataDlg.trialLogFile;


	monitor_n_Prompt.sessionNumber = monitor_n_Prompt.dLog.GetLastSessionNumber()+1;
	//monitor_n_Prompt.sessionNumber = monitor_n_Prompt.dLog.GetSessionNumber();
	//dx may 09, 2012: logging session start
	monitor_n_Prompt.dLog.WriteNewLine(3);
	CString s,s1;
	s.Format("Session Start,%d",monitor_n_Prompt.sessionNumber);
	s1.Format("User settings,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"
		,monitor_n_Prompt.userSettingsDlg.levelOnePrompt,monitor_n_Prompt.userSettingsDlg.levelTwoPrompt,monitor_n_Prompt.userSettingsDlg.levelThreePrompt,monitor_n_Prompt.userSettingsDlg.levelFourPrompt,monitor_n_Prompt.userSettingsDlg.levelFivePrompt
		,monitor_n_Prompt.userSettingsDlg.firstPromptTime,monitor_n_Prompt.userSettingsDlg.nextPromptTime,monitor_n_Prompt.userSettingsDlg.nPrompts,monitor_n_Prompt.userSettingsDlg.priming,monitor_n_Prompt.userSettingsDlg.use_attention_grabbers
		,monitor_n_Prompt.userSettingsDlg.rewardallsteps,monitor_n_Prompt.userSettingsDlg.useCountdown,monitor_n_Prompt.userSettingsDlg.scrubTime,monitor_n_Prompt.userSettingsDlg.rinseTime,monitor_n_Prompt.userSettingsDlg.startOver,monitor_n_Prompt.userSettingsDlg.resoap
		,monitor_n_Prompt.userSettingsDlg.soapTime
		/*dx nov 09, 2012*/,monitor_n_Prompt.userSettingsDlg.wetTime,monitor_n_Prompt.userSettingsDlg.dryTime,monitor_n_Prompt.userSettingsDlg.volume);
	monitor_n_Prompt.setTimeStamp();	
	monitor_n_Prompt.dLog.SetTimeStamp(monitor_n_Prompt.timeStampStruct.timeStamp, monitor_n_Prompt.timeStampStruct.timeStamp_msec);
	monitor_n_Prompt.dLog.Write(s);
	monitor_n_Prompt.dLog.Write(s1);

	//Initialize the camera
	OnFileStartcamera();

	//dx may 29, 2012: trying out what if we don't reinitialize
	//Initialize the tracker and calibration
	MTCleanUp();
	//CleanUp();

	InitializeTrackerandCalibration();


	//Read in starting prompt levels from file
	actionModule.SetStartingPromptLevels();
	for (int i=0; i<5; i++)
	{
		monitor_n_Prompt.startingPromptLevels[i] = actionModule.startingPromptLevel[i];
	}
	//Debugging - Dec. 5, 2011
	ofstream testdata;
	testdata.open("C:\\dxFiles\\COACH4ASDresourceFiles\\Testdata.txt", ofstream::app);
	testdata << "Starting Prompt Levels" << endl;
	testdata << monitor_n_Prompt.startingPromptLevels[0] << endl << monitor_n_Prompt.startingPromptLevels[1] << endl << monitor_n_Prompt.startingPromptLevels[2] << endl << monitor_n_Prompt.startingPromptLevels[3] <<endl << monitor_n_Prompt.startingPromptLevels[4] << endl;
	testdata.close();

	//Set values of variables needed for prompting
	//dx may 09, 2012: we won't use actionModule 's starting prompt levels for the in-home trial, so just set the starting level to 1
	//monitor_n_Prompt.promptLevel = monitor_n_Prompt.startingPromptLevels[0];//Check
	monitor_n_Prompt.promptLevel = 1;
	//------------
	monitor_n_Prompt.numPromptsGivenThisState = 0;

	//CString s1;
	//s1.Format("prompt level initialized: %d", monitor_n_Prompt.promptLevel);
	//monitor_n_Prompt.dLog.Write(s1);


	////Begin writing to logfile
	//logFile_D.open(logFileName, ios::app);//Jan. 26, 2011
	//logFile_D << "Date: " << userDataDlg.trialDate << endl;
	//logFile_D << "Start Time: " << userDataDlg.trialStartTime << endl;

	//if (firstentry){
	//	logFile_D << "Timestamp (ms)" << "\t" << "\t" << "System's Activity" << "\t" << "\t" << "\t" << "Child's Activity" << endl << endl;
	//	firstentry = false;
	//}
	//
	//time (&monitor_n_Prompt.trialstartingtime);
	//logFile_D << "0" << "\t" << "\t" << "\t" << "Session Started" << endl;
	//logFile_D.close();

	//Hide the SetupMenu Dialog
	//startupMenuDlg.ShowWindow(SW_HIDE);

	//dx may 25, 2012: restore white background at CG interface side
	CDC* pDC = AfxGetMainWnd()->GetDC();
	CBrush brush;
	brush.CreateSolidBrush(RGB(255,255,255));
	RECT rect;
	rect.top = 0;
	rect.bottom = FIRSTMONITORHEIGHT;
	rect.left = 0;
	rect.right = FIRSTMONITORWIDTH;
	pDC->FillRect(&rect,&brush);

	////dx sep 22, 2012:
#ifdef USE_MT_VIDEO_WRITER
	TWriteFrameWorker = AfxBeginThread(writeFrameWorker_start, this); //dxtodo sep 22, 2012: set priority to Thread::P_LOWEST?
	//SetThreadPriority(TWriteFrameWorker->m_hThread, THREAD_PRIORITY_HIGHEST); //need to make sure frames are written or else an excess of unworked frames would cause insufficient memory problems => but setting it to THREAD_PRIORITY_HIGHEST doesn't do much, the problem is maybe each frame's size is too big, need to use codec for compression
	writeFrameWorker_die = false;
	assert(writeFrameTasksQueue.empty()); //dx nov 27, 2012: i don't know if the worker thread is ever slow enough to raise exception, just wanna test for it
	while (!writeFrameTasksQueue.empty())
	{ //dx nov 27, 2012: making sure writeFrameTasksQueue is empty
		writeFrameTasksQueue.pop_front();
	}
#endif // USE_MT_VIDEO_WRITER

	//dx sep 27, 2012:
	cameraFramesStampsFile.open("C:\\dxFiles\\COACH4ASDresourceFiles\\testLog\\cameraFramesStamps.txt", ios::app);
	cameraFramesStampsFile << endl << "session " << monitor_n_Prompt.sessionNumber << ":" << endl;

	pFrame->disableToolBarButtons(false); //dx oct 28, 2012: enable the buttons

	view->KillTimer(TIMER_ID_GRAB_IMAGES);
	view->SetTimer(TIMER_ID_PROMPT, TIMER_ID_CAMERA_PERIOD, 0);
}

void CAutismCOACHDoc::OnFileCalibrate()
{	
	//RetrieveCamImage();

	if (!camera_running)
	{
		MessageBox(NULL, "Please Start Camera Before Calibrating", "ERROR!", MB_OK);
		return;
	}

	//Added April 20, 2010 - Justin
	cvResize(currentFrame, resized_currentFrame);

	if (tracker != NULL)
	{
		if (!calibrating) // make sure calib dlg is not already on
		{
			calibDlg.DestroyWindow();
			//Modified line below - April 20, Justin
			calibDlg.SetImage(resized_currentFrame); // always show the original image in the calibration window (not the skin/particle/... image)
			calibDlg.SetTracker(tracker); 
			calibDlg.SetFlags(&calibrating);//, &waterDetectorIsInitialized, &actionDetectorIsInitialized);
			calibDlg.SetVideoFilename(videoFilename); // to set the default calib filename
			calibDlg.Create(IDD_CALIBRATE_DIALOG, NULL);
			calibDlg.ShowWindow(SW_SHOW);
			calibrating = true; 
		}
	}
}

void CAutismCOACHDoc::ShowRegions(string loopState, IplImage *currentFrame, IplImage *skinImg, IplImage *currentFrameWhole)
{
	//Define color of circles and text
	CvScalar regioncolor = CV_RGB(0,255,0);

	//REGION_SOAP
	CvPoint soapcentre = cvPoint(calibDlg.regions[REGION_SOAP].centreX, calibDlg.regions[REGION_SOAP].centreY);
	CvPoint soaptextcentre = cvPoint(calibDlg.regions[REGION_SOAP].centreX-16, calibDlg.regions[REGION_SOAP].centreY);
	int soapradius = calibDlg.regions[REGION_SOAP].radius;

	//REGION_SOAP_SPOUT
	CvPoint soap_spoutcentre = cvPoint(calibDlg.regions[REGION_SOAP_SPOUT].centreX, calibDlg.regions[REGION_SOAP_SPOUT].centreY);
	CvPoint soap_spouttextcentre = cvPoint(calibDlg.regions[REGION_SOAP_SPOUT].centreX-16, calibDlg.regions[REGION_SOAP_SPOUT].centreY);
	int soap_spoutradius = calibDlg.regions[REGION_SOAP_SPOUT].radius;

	//REGION_TAP_LEFT
	CvPoint ltapcentre = cvPoint(calibDlg.regions[REGION_TAP_LEFT].centreX, calibDlg.regions[REGION_TAP_LEFT].centreY);
	CvPoint ltaptextcentre = cvPoint(calibDlg.regions[REGION_TAP_LEFT].centreX + 25, calibDlg.regions[REGION_TAP_LEFT].centreY - 15);
	int ltapradius = calibDlg.regions[REGION_TAP_LEFT].radius;

	//REGION_TAP_RIGHT
	CvPoint rtapcentre = cvPoint(calibDlg.regions[REGION_TAP_RIGHT].centreX, calibDlg.regions[REGION_TAP_RIGHT].centreY);
	CvPoint rtaptextcentre = cvPoint(calibDlg.regions[REGION_TAP_RIGHT].centreX - 93, calibDlg.regions[REGION_TAP_RIGHT].centreY-10);
	int rtapradius = calibDlg.regions[REGION_TAP_RIGHT].radius;

	//REGION_WATER
	CvPoint watercentre = cvPoint(calibDlg.regions[REGION_WATER].centreX, calibDlg.regions[REGION_WATER].centreY);
	CvPoint watertextcentre = cvPoint(calibDlg.regions[REGION_WATER].centreX - 20, calibDlg.regions[REGION_WATER].centreY);
	int waterradius = calibDlg.regions[REGION_WATER].radius;

	//REGION_SINK
	CvPoint sinkcentre = cvPoint(calibDlg.regions[REGION_SINK].centreX, calibDlg.regions[REGION_SINK].centreY);
	CvPoint sinktextcentre = cvPoint(calibDlg.regions[REGION_SINK].centreX - 14, calibDlg.regions[REGION_SINK].centreY +100);
	int sinkradius = calibDlg.regions[REGION_SINK].radius;

	//REGION_TOWEL
	CvPoint towelcentre = cvPoint(calibDlg.regions[REGION_TOWEL].centreX, calibDlg.regions[REGION_TOWEL].centreY);
	CvPoint toweltextcentre = cvPoint(calibDlg.regions[REGION_TOWEL].centreX - 100, calibDlg.regions[REGION_TOWEL].centreY);
	int towelradius = calibDlg.regions[REGION_TOWEL].radius;

	//dx oct 24, 2012
	//REGION_FAUCET 
	//CvPoint faucetcentre = cvPoint(calibDlg.regions[REGION_FAUCET].centreX, calibDlg.regions[REGION_FAUCET].centreY);
	CvPoint faucettextcentre = cvPoint(calibDlg.regions[REGION_FAUCET].centreX - 100, calibDlg.regions[REGION_FAUCET].centreY);
	CvPoint faucetLeftUp = cvPoint(calibDlg.regions[REGION_FAUCET].centreX - calibDlg.regions[REGION_FAUCET].width, calibDlg.regions[REGION_FAUCET].centreY - calibDlg.regions[REGION_FAUCET].height);
	CvPoint faucetRightBottom = cvPoint(calibDlg.regions[REGION_FAUCET].centreX + calibDlg.regions[REGION_FAUCET].width, calibDlg.regions[REGION_FAUCET].centreY + calibDlg.regions[REGION_FAUCET].height);

	//HAND CENTROID
	//CvScalar centroidcolor = CV_RGB(255,255,255);
	//CvPoint centroidcentre = cvPoint(tracker->centroidX*2, tracker->centroidY*2);
	//int centroidradius = 10;

	//Define font
	CvFont waitfont;
	cvInitFont(&waitfont,CV_FONT_HERSHEY_SIMPLEX ,0.5f, 0.5f, 0, 1);  //Initialise font


	if (skinImg != NULL)
	{
		//Put circles
		cvCircle(skinImg, soapcentre, soapradius, regioncolor, 2);
		cvPutText(skinImg, "soap", soaptextcentre, &waitfont, regioncolor);

		cvCircle(skinImg, ltapcentre, ltapradius, regioncolor, 2);
		cvPutText(skinImg, "left tap", ltaptextcentre, &waitfont, regioncolor);

		cvCircle(skinImg, rtapcentre, rtapradius, regioncolor, 2);
		cvPutText(skinImg, "right tap", rtaptextcentre, &waitfont, regioncolor);

		cvCircle(skinImg, watercentre, waterradius, regioncolor, 2);
		cvPutText(skinImg, "water", watertextcentre, &waitfont, regioncolor);

		cvCircle(skinImg, sinkcentre, sinkradius, regioncolor, 2);
		cvPutText(skinImg, "sink", sinktextcentre, &waitfont, regioncolor);

		cvCircle(skinImg, towelcentre, towelradius, regioncolor, 2);
		cvPutText(skinImg, "towel", toweltextcentre, &waitfont, regioncolor);

		cvCircle(skinImg, soap_spoutcentre, soap_spoutradius, regioncolor, 2);
		cvPutText(skinImg, "soap spout", soap_spouttextcentre, &waitfont, regioncolor);

		//dx oct 24, 2012
		cvRectangle(skinImg, faucetLeftUp , faucetRightBottom, regioncolor, 2);
		cvPutText(skinImg, "faucet", faucettextcentre, &waitfont, regioncolor);
	}

	if (loopState.compare("tracker") == 0)
	{
		if (currentFrame != NULL)
		{
			//Put circles
			cvCircle(currentFrame, soapcentre, soapradius, regioncolor, 2);
			cvPutText(currentFrame, "soap", soaptextcentre, &waitfont, regioncolor);

			cvCircle(currentFrame, ltapcentre, ltapradius, regioncolor, 2);
			cvPutText(currentFrame, "left tap", ltaptextcentre, &waitfont, regioncolor);

			cvCircle(currentFrame, rtapcentre, rtapradius, regioncolor, 2);
			cvPutText(currentFrame, "right tap", rtaptextcentre, &waitfont, regioncolor);

			cvCircle(currentFrame, watercentre, waterradius, regioncolor, 2);
			cvPutText(currentFrame, "water", watertextcentre, &waitfont, regioncolor);

			cvCircle(currentFrame, sinkcentre, sinkradius, regioncolor, 2);
			cvPutText(currentFrame, "sink", sinktextcentre, &waitfont, regioncolor);

			cvCircle(currentFrame, towelcentre, towelradius, regioncolor, 2);
			cvPutText(currentFrame, "towel", toweltextcentre, &waitfont, regioncolor);

			cvCircle(currentFrame, soap_spoutcentre, soap_spoutradius, regioncolor, 2);
			cvPutText(currentFrame, "soap spout", soap_spouttextcentre, &waitfont, regioncolor);

			//dx oct 24, 2012
			cvRectangle(currentFrame, faucetLeftUp , faucetRightBottom, regioncolor, 2);
			cvPutText(currentFrame, "faucet", faucettextcentre, &waitfont, regioncolor);
		}
	}
	else
	{
		if (currentFrame != NULL)
		{
			//Put circles
#ifndef DEBUG
			if (loopState.compare("trainWater")!=0)
			{ //dx oct 25, 2012: if not in trainWater, display soap and water circles
#endif // !DEBUG
				cvCircle(currentFrame, soapcentre, soapradius, regioncolor, 2);
				cvPutText(currentFrame, "soap", soaptextcentre, &waitfont, regioncolor);

				cvCircle(currentFrame, watercentre, waterradius, regioncolor, 2);
				cvPutText(currentFrame, "water", watertextcentre, &waitfont, regioncolor);

				cvCircle(currentFrame, towelcentre, towelradius, regioncolor, 2);
				cvPutText(currentFrame, "towel", toweltextcentre, &waitfont, regioncolor);
#ifndef DEBUG
			}
			else if (loopState.compare("trainWater")==0)
			{//dx oct 25, 2012: if in trainWater, display faucet and towel circles				
#endif // !DEBUG
				cvRectangle(currentFrame, faucetLeftUp , faucetRightBottom, regioncolor, 2);
				cvPutText(currentFrame, "faucet", faucettextcentre, &waitfont, regioncolor);

				cvCircle(currentFrame, towelcentre, towelradius, regioncolor, 2);
				cvPutText(currentFrame, "towel", toweltextcentre, &waitfont, regioncolor);
#ifndef DEBUG
			}
#endif // !DEBUG
			cvCircle(currentFrame, ltapcentre, ltapradius, regioncolor, 2);
			cvPutText(currentFrame, "left tap", ltaptextcentre, &waitfont, regioncolor);

			cvCircle(currentFrame, rtapcentre, rtapradius, regioncolor, 2);
			cvPutText(currentFrame, "right tap", rtaptextcentre, &waitfont, regioncolor);

#ifdef DEBUG //dx oct 25, 2012: displays everything if in debug
			cvCircle(currentFrame, sinkcentre, sinkradius, regioncolor, 2);
			cvPutText(currentFrame, "sink", sinktextcentre, &waitfont, regioncolor);

			cvCircle(currentFrame, soap_spoutcentre, soap_spoutradius, regioncolor, 2);
			cvPutText(currentFrame, "soap spout", soap_spouttextcentre, &waitfont, regioncolor);			

			cvRectangle(currentFrame, faucetLeftUp , faucetRightBottom, regioncolor, 2);
			cvPutText(currentFrame, "faucet", faucettextcentre, &waitfont, regioncolor);
#endif // DEBUG
		}
	}

	//cvCircle(skinImg, centroidcentre, centroidradius, centroidcolor, 2);

	//Write activity to skinImg
	//CvScalar textcolor = CV_RGB(255,0,0);
	//CvPoint textcentre = cvPoint(240, 50);
	//		
	//if (text != NULL)
	//	cvPutText(skinImg, text, textcentre, &waitfont, textcolor);
}


void CAutismCOACHDoc::ProcessOneImage(IplImage *currentFrame, IplImage *skinImg)
{
	ofstream loopRateFile ("C:\\dxFiles\\COACH4ASDresourceFiles\\loopRate.txt", ios::app);
	DWORD loopStartTime, loopNowTime;
	loopRateFile << "\tIn ProcessOneImage: " << endl;
	loopStartTime = GetTickCount();


	//If passing small frame (320X240) to tracker
	cvResize(currentFrame, small_currentFrame);

	loopNowTime = GetTickCount();
	loopRateFile << "\t" << loopNowTime - loopStartTime << ", cvResize" << endl;
	loopStartTime = GetTickCount();

	// update the tracker
	tracker->update((unsigned char*)(small_currentFrame->imageData), NULL /*framePos*/, false, finalImageRaw, skinImageRaw, bestpImageRaw);

	loopNowTime = GetTickCount();
	loopRateFile << "\t" << loopNowTime - loopStartTime << ", tracker->update" << endl;
	loopStartTime = GetTickCount();

	//Copy image data to skinImg
	memcpy(small_skinImg->imageData, skinImageRaw, imageWidth/2 * imageHeight/2 * 3);
		//dxdebug nov 27, 2012:
		//memcpy(small_skinImg->imageData, finalImageRaw, imageWidth/2 * imageHeight/2 * 3);
		//memcpy(small_skinImg->imageData, bestpImageRaw, imageWidth/2 * imageHeight/2 * 3);
	//Get number of blobs in each region
	tracker->extractObservationsNewer(skinImageRaw, ntowel_obs, nsoap_obs, nsoapspout_obs, ntapl_obs, ntapr_obs, nwater_obs, nsink_obs, nliquidsoap_obs);

	loopNowTime = GetTickCount();
	loopRateFile << "\t" << loopNowTime - loopStartTime << ", tracker->extractObservationsNewer" << endl;
	loopStartTime = GetTickCount();

	cvResize(small_skinImg, skinImg);

	loopNowTime = GetTickCount();
	loopRateFile << "\t" << loopNowTime - loopStartTime << ", cvResize" << endl;
	loopStartTime = GetTickCount();

	//dx oct 23, 2012: don't need this since we are only using background subtraction
	////Initialize and Update the water detector
	////Note: this is not necessary if only using background subtraction to determine if water is on
	//InitializeWaterDetectorIfNeeded();//Dec. 13, 2010

	//loopNowTime = GetTickCount();
	//loopRateFile << "\t" << loopNowTime - loopStartTime << ", InitializeWaterDetectorIfNeeded" << endl;
	//loopStartTime = GetTickCount();

	//dx oct 23, 2012: UpdateImages is useless since we are only using background subtraction
	//waterDetector.UpdateImages(currentFrame);//Dec. 13, 2010

	//loopNowTime = GetTickCount();
	//loopRateFile << "\t" << loopNowTime - loopStartTime << ", waterDetector.UpdateImages" << endl;
	//loopStartTime = GetTickCount();

	loopRateFile.close();
}

bool CAutismCOACHDoc::GetCurrentFrame(IplImage*& frame)
{
	if (resized == false)
		return false;

	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_DUPLEX ,0.3f, 0.3f, 0, 1);  //Initialise font
	CvScalar regioncolor = CV_RGB(0,255,0);
	CvPoint topTextPosL = cvPoint(5, 20);
	CString topText;

	if (videoplaytype == 1)
	{
		frame = currentFrame_annotationCopy;
		topText.Format("currentFrame",monitor_n_Prompt.sessionNumber);
		cvPutText(frame, topText, topTextPosL, &font, regioncolor);
	}


	else if (videoplaytype == 2)
	{
		frame = skinImg_annotationCopy;
		topText.Format("skinImg",monitor_n_Prompt.sessionNumber);
		cvPutText(frame, topText, topTextPosL, &font, regioncolor);
		//if (monitor_n_Prompt.userState == 2)
		//{
		//	CvScalar scrubbingcolor = CV_RGB(0,255,0);
		//	CvFont scrubfont;
		//	cvInitFont(&scrubfont,CV_FONT_HERSHEY_SIMPLEX ,0.5f, 0.5f, 0, 1);  //Initialize font
		//	CvPoint textcentre = cvPoint(320, 240);

		//	char * scrubfeatschar = new char[5];

		//	CString scrubfeatsString = "";
		//	_itoa_s(opticalFlow.numfeaturesinlast7frames, scrubfeatschar, 25, 10);
		//	scrubfeatsString.AppendChar(scrubfeatschar[0]);
		//	scrubfeatsString.AppendChar(scrubfeatschar[1]);
		//	scrubfeatsString.AppendChar(scrubfeatschar[2]);
		//	cvPutText(skinImg, scrubfeatsString, textcentre, &scrubfont, scrubbingcolor);
		//	delete [] scrubfeatschar;
		//}	
	}

	else if (videoplaytype == 3)
	{
		if (monitor_n_Prompt.userState != 2) //not in scrubbing
		{
			frame = currentFrameWhole_annotationCopy;
			topText.Format("currentFrameWhole",monitor_n_Prompt.sessionNumber);
			cvPutText(frame, topText, topTextPosL, &font, regioncolor);
		}

		else if (monitor_n_Prompt.userState == 2) //in scrubbing
		{
			frame = currentFrame_annotationCopy;
			topText.Format("currentFrame_opticalFlow",monitor_n_Prompt.sessionNumber);
			cvPutText(frame, topText, topTextPosL, &font, regioncolor);
			//dx nov 29, 2012: show opticalFlow image
			//display all the corner points
			deque<CvPoint2D32f> A = opticalFlow.cornersA_copy,B = opticalFlow.cornersB_copy;
			assert(A.size() == B.size());
			while (!A.empty() && !B.empty())
			{
				cvCircle(currentFrame, cvPoint(A.front().x, A.front().y), 2, CV_RGB(255,0,0));
				cvCircle(currentFrame, cvPoint(B.front().x, B.front().y), 2, CV_RGB(0,255,0));
				cvLine(currentFrame, cvPoint(A.front().x, A.front().y), cvPoint(B.front().x, B.front().y), CV_RGB(255,255,0));
				A.pop_front();
				B.pop_front();
			}
		}
	}

	return true;
}

void CAutismCOACHDoc::OutputFrameRate()
{
	//logFile.open("D:\\Visual Studio 2008\\logfile.txt", ios::app);
	//logFile << clock() << endl;
	//logFile.close();
}

void CAutismCOACHDoc::OnFileEnterusersettings()
{
	logFile_D.open(logFileName, ios::app);
	if (firstentry)
	{
		logFile_D << "Timestamp (ms)" << "\t" << "\t" << "System's Activity" << "\t" << "\t" << "\t" << "Child's Activity" << endl << endl;
		firstentry = false;
	}
	time (&monitor_n_Prompt.logtime);
	dif = difftime(monitor_n_Prompt.logtime, monitor_n_Prompt.trialstartingtime);
	logFile_D << dif << "\t" << "\t" << "\t" << "\t" << "\t" << "\t" << "\t" << "\t" << "Entering User Settings" << endl;
	logFile_D.close();


	//dx aug 19, 2012:
	monitor_n_Prompt.userSettingsDlg.LoadDefaultUserSettings();
	monitor_n_Prompt.settings1.levelOnePrompt = monitor_n_Prompt.userSettingsDlg.levelOnePrompt;
	monitor_n_Prompt.settings1.levelTwoPrompt = monitor_n_Prompt.userSettingsDlg.levelTwoPrompt;
	monitor_n_Prompt.settings1.levelThreePrompt = monitor_n_Prompt.userSettingsDlg.levelThreePrompt;
	monitor_n_Prompt.settings1.levelFourPrompt = monitor_n_Prompt.userSettingsDlg.levelFourPrompt;
	monitor_n_Prompt.settings1.levelFivePrompt = monitor_n_Prompt.userSettingsDlg.levelFivePrompt;

	monitor_n_Prompt.settings2.nPrompts = monitor_n_Prompt.userSettingsDlg.nPrompts;
	monitor_n_Prompt.settings2.firstPromptTime = monitor_n_Prompt.userSettingsDlg.firstPromptTime;
	monitor_n_Prompt.settings2.nextPromptTime = monitor_n_Prompt.userSettingsDlg.nextPromptTime;
	monitor_n_Prompt.settings2.priming = monitor_n_Prompt.userSettingsDlg.priming;
	monitor_n_Prompt.settings2.use_attention_grabbers = monitor_n_Prompt.userSettingsDlg.use_attention_grabbers;
	monitor_n_Prompt.settings2.rewardallsteps = monitor_n_Prompt.userSettingsDlg.rewardallsteps;
	
	monitor_n_Prompt.settings3.useCountdown = monitor_n_Prompt.userSettingsDlg.useCountdown;
	monitor_n_Prompt.settings3.scrubTime = monitor_n_Prompt.userSettingsDlg.scrubTime;
	monitor_n_Prompt.settings3.rinseTime = monitor_n_Prompt.userSettingsDlg.rinseTime;
	monitor_n_Prompt.settings3.soapTime = monitor_n_Prompt.userSettingsDlg.soapTime;
	//dx nov 09, 2012
	monitor_n_Prompt.settings3.wetTime = monitor_n_Prompt.userSettingsDlg.wetTime;
	monitor_n_Prompt.settings3.dryTime = monitor_n_Prompt.userSettingsDlg.dryTime;
	monitor_n_Prompt.settings3.volume = monitor_n_Prompt.userSettingsDlg.volume;
	//dx nov 10, 2012: pointers to other setting dlgs
	monitor_n_Prompt.settings3.pSettingDlg1 = & monitor_n_Prompt.settings1;
	monitor_n_Prompt.settings3.pSettingDlg2 = & monitor_n_Prompt.settings2;

	//if(monitor_n_Prompt.doModalUserSettings(1) == IDOK)
	if(monitor_n_Prompt.doModalUserSettings(1) == IDOK) //dx sep 18, 2012: changed so that page 2 is displayed first (so # of prompts selection is displayed first)
	{
		monitor_n_Prompt.userSettingsDlg.levelOnePrompt = monitor_n_Prompt.settings1.levelOnePrompt;
		monitor_n_Prompt.userSettingsDlg.levelTwoPrompt = monitor_n_Prompt.settings1.levelTwoPrompt;
		monitor_n_Prompt.userSettingsDlg.levelThreePrompt = monitor_n_Prompt.settings1.levelThreePrompt;
		monitor_n_Prompt.userSettingsDlg.levelFourPrompt = monitor_n_Prompt.settings1.levelFourPrompt;
		monitor_n_Prompt.userSettingsDlg.levelFivePrompt = monitor_n_Prompt.settings1.levelFivePrompt;

		monitor_n_Prompt.userSettingsDlg.nPrompts = monitor_n_Prompt.settings2.nPrompts;
		monitor_n_Prompt.userSettingsDlg.firstPromptTime = monitor_n_Prompt.settings2.firstPromptTime;
		monitor_n_Prompt.userSettingsDlg.nextPromptTime = monitor_n_Prompt.settings2.nextPromptTime;
		monitor_n_Prompt.userSettingsDlg.priming = monitor_n_Prompt.settings2.priming;
		monitor_n_Prompt.userSettingsDlg.use_attention_grabbers = monitor_n_Prompt.settings2.use_attention_grabbers;
		monitor_n_Prompt.userSettingsDlg.rewardallsteps = monitor_n_Prompt.settings2.rewardallsteps;

		monitor_n_Prompt.userSettingsDlg.useCountdown = monitor_n_Prompt.settings3.useCountdown;
		monitor_n_Prompt.userSettingsDlg.scrubTime = monitor_n_Prompt.settings3.scrubTime;
		monitor_n_Prompt.userSettingsDlg.rinseTime = monitor_n_Prompt.settings3.rinseTime;
		monitor_n_Prompt.userSettingsDlg.soapTime = monitor_n_Prompt.settings3.soapTime;
		//dx nov 09, 2012
		monitor_n_Prompt.userSettingsDlg.wetTime = monitor_n_Prompt.settings3.wetTime;
		monitor_n_Prompt.userSettingsDlg.dryTime = monitor_n_Prompt.settings3.dryTime;
		monitor_n_Prompt.userSettingsDlg.volume = monitor_n_Prompt.settings3.volume;

		monitor_n_Prompt.userSettingsDlg.SaveUserSettings();
	}

	/*
	CStdioFile userSettings;
	userSettings.Open("C:\\dxFiles\\COACH4ASDresourceFiles\\UserSettings.txt",CFile::modeReadWrite);
	CString s;
	s.Format("%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n"
	,monitor_n_Prompt.userSettingsDlg.levelOnePrompt,monitor_n_Prompt.userSettingsDlg.levelTwoPrompt
	,monitor_n_Prompt.userSettingsDlg.levelThreePrompt,monitor_n_Prompt.userSettingsDlg.levelFourPrompt
	,monitor_n_Prompt.userSettingsDlg.levelFivePrompt,monitor_n_Prompt.userSettingsDlg.firstPromptTime
	,monitor_n_Prompt.userSettingsDlg.nextPromptTime,monitor_n_Prompt.userSettingsDlg.nPrompts

	,monitor_n_Prompt.userSettingsDlg.priming,monitor_n_Prompt.userSettingsDlg.use_attention_grabbers
	,monitor_n_Prompt.userSettingsDlg.rewardallsteps

	,monitor_n_Prompt.userSettingsDlg.useCountdown,monitor_n_Prompt.userSettingsDlg.scrubTime
	,monitor_n_Prompt.userSettingsDlg.rinseTime

	,monitor_n_Prompt.userSettingsDlg.startOver,monitor_n_Prompt.userSettingsDlg.resoap);

	userSettings.WriteString(s);
	userSettings.Close();

	monitor_n_Prompt.userSettingsDlg.alreadyentered_usersettings = true;
	*/
	//	monitor_n_Prompt.userSettingsDlg.prompttypes[0] = monitor_n_Prompt.userSettingsDlg.levelOnePrompt;
	//	monitor_n_Prompt.userSettingsDlg.prompttypes[1] = monitor_n_Prompt.userSettingsDlg.levelTwoPrompt;
	//	monitor_n_Prompt.userSettingsDlg.prompttypes[2] = monitor_n_Prompt.userSettingsDlg.levelThreePrompt;
	//	monitor_n_Prompt.userSettingsDlg.prompttypes[3] = monitor_n_Prompt.userSettingsDlg.levelFourPrompt;
	//	monitor_n_Prompt.userSettingsDlg.prompttypes[4] = monitor_n_Prompt.userSettingsDlg.levelFivePrompt;
	//}

	//
	//////monitor_n_Prompt.userSettingsDlg.LoadDefaultUserSettings();
	////monitor_n_Prompt.userSettingsDlg.DestroyWindow();
	////monitor_n_Prompt.userSettingsDlg.Create(IDD_USERSETTINGS_DIALOG, NULL);
	//////dx may 16, 2012: maximized dialog
	////monitor_n_Prompt.userSettingsDlg.ModifyStyle(WS_CAPTION,0);
	//////monitor_n_Prompt.userSettingsDlg.ShowWindow(SW_SHOW);
	////monitor_n_Prompt.userSettingsDlg.ShowWindow(SW_SHOWMAXIMIZED);
	////monitor_n_Prompt.userSettingsDlg.SetWindowPos(&monitor_n_Prompt.userSettingsDlg.wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);	//dx may 23, 2012: make the dialog on top

}

void CAutismCOACHDoc::wait(float seconds)
{
	clock_t endwait;
	endwait = clock () + seconds * CLOCKS_PER_SEC ;
	while (clock() < endwait) {}
}

void CAutismCOACHDoc::ReportHandPositions()
{
	//CMainFrame* mainframe = (CMainFrame*) AfxGetApp()->m_pMainWnd;

	//mainframe->ShowHandPositions(observe[LEFT_HANDLOC_OBSERVATION], observe[RIGHT_HANDLOC_OBSERVATION]);

	ShowHandPositions(observe[LEFT_HANDLOC_OBSERVATION], observe[RIGHT_HANDLOC_OBSERVATION]);
}

void CAutismCOACHDoc::ShowHandPositions(int l, int r)
{
	static char* regionName[] = {	"away",			// 0		// check enum in Defines.h
		"sink",			// 1
		"water",		// 2
		"tap",			// 3
		"soap",			// 4
		"towel",		// 5
		"???"	};		// 6

	if ( (l<0) || (l>5) )
		l = 6;

	if ( (r<0) || (r>5) )
		r = 6;

	//pass to monitor_n_Prompt;

	lh_Obs = regionName[l];
	rh_Obs = regionName[r];
}



void CAutismCOACHDoc::TerminatePrompting()
{	
	view->KillTimer(TIMER_ID_PROMPT);
	view->KillTimer(TIMER_ID_SYSTEM_PAUSE); //dx dec 04, 2012: sometimes, user completes the last step at the very time system switches to pause

	//dxnote nov 26, 2012: take care of putting the variables to their initial values
	camera_running = false;
	//reinitialize variables
	monitor_n_Prompt.reinitialize();
	prompting = false;
	
	//dxnote oct 28, 2012: need to do updateToolBarHeight and drawing of the black screen before pFrame->OnFileStoppromptingsession();
	// or else, getToolBarHeight returns 0 and toolBar is partially blocked by the black screen drawn
	//dx oct 28, 2012: don't block the menu tool bar
	CMainFrame* pFrame_temp = (CMainFrame*)AfxGetMainWnd();	
	pFrame_temp->updateToolBarHeight();
	LONG barHeight = pFrame_temp->getToolBarHeight(); 
	//dx may 16, 2012: display blank screen on CG interface side
	CDC* pDC = AfxGetMainWnd()->GetDC();
	CBrush brush;
	brush.CreateSolidBrush(RGB(0,0,0));
	RECT rect;
	//rect.top = 0;
	rect.top = barHeight; //dx oct 28, 2012
	rect.bottom = FIRSTMONITORHEIGHT;
	rect.left = 0;
	rect.right = FIRSTMONITORWIDTH;
	pDC->FillRect(&rect,&brush);


	//dx may 15, 2012: change tool bar
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->OnFileStoppromptingsession();

	closePrompts(); //dx aug 30, 2012
	//Show the blank screen
	monitor_n_Prompt.ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\blackscreen.bmp");

	//dx dec 03, 2012: should be killed in UpdatePromptingInterface instead (more organized, and so that it works in scrubTraining where userStateChanged flag is arbitrarily set
	////kill scrubbingLoop_workerThread and videoWritting_workerThread
	//monitor_n_Prompt.scrubbingLoop_die = true;
	//monitor_n_Prompt.initializeScrubbingLoop_GRABBING_MODE();

#ifdef USE_MT_VIDEO_WRITER	
	writeFrameWorker_die = true; //dx sep 22, 2012: telling the workerThread to die
	if (TWriteFrameWorker != NULL)
		WaitForSingleObject(TWriteFrameWorker->m_hThread, INFINITE);
	TWriteFrameWorker = NULL;
#endif //USE_MT_VIDEO_WRITER

	//dx dec 02, 2012: kill the scrubbingLoop_workerThread
	monitor_n_Prompt.scrubbingLoop_die = true;
	monitor_n_Prompt.initializeScrubbingLoop_GRABBING_MODE();
	if (TScrubbingLoopWorker != NULL)
		WaitForSingleObject(TScrubbingLoopWorker->m_hThread, INFINITE); //this line needs to procede release of captures and MTCleanUp to make sure the shared resources (captures and tracker) aren't destroyed before the worker exits
	TScrubbingLoopWorker = NULL;

	if (capture!= NULL) { 
		cvReleaseCapture( &capture ); 
		capture = NULL; 
	}
	if (captureWhole!= NULL) { 
		cvReleaseCapture( &captureWhole ); 
		captureWhole = NULL; 
	}
	if (fileCapture != NULL) {
		cvReleaseCapture( &fileCapture );
		fileCapture = NULL;
	}
	
#ifndef USE_MT_VIDEO_WRITER //if using MT worker thread for video writers, then their releases will be done in the worker thread
	//release the video writers
	if (writer!=NULL)
	{
		cvReleaseVideoWriter(&writer);
		writer = NULL;
	}
	if (writerWhole!=NULL)
	{
		cvReleaseVideoWriter(&writerWhole);
		writerWhole = NULL;
	}
	if (writerSkinImg!=NULL)
	{
		cvReleaseVideoWriter(&writerSkinImg);
		writerSkinImg = NULL;
	}

	if (writerAnnotated!=NULL)
	{
		cvReleaseVideoWriter(&writerAnnotated);
		writerAnnotated = NULL;
	}
	if (writerWholeAnnotated!=NULL)
	{
		cvReleaseVideoWriter(&writerWholeAnnotated);
		writerWholeAnnotated = NULL;
	}
	if (writerSkinImgAnnotated!=NULL)
	{
		cvReleaseVideoWriter(&writerSkinImgAnnotated);
		writerSkinImgAnnotated = NULL;
	}
#endif // !USE_MT_VIDEO_WRITER //if using MT worker thread for video writers, then their releases will be done in the worker thread

	MTCleanUp();
	//CleanUp();

	////dxdebug oct 19, 2012:
	//if (writerSkinImagesBundle != NULL) {
	//	cvReleaseVideoWriter(&writerSkinImagesBundle); 
	//	writerSkinImagesBundle = NULL; 
	//}

	//dx sep 20, 2012:
#ifdef USE_PS3_FOR_CURRENT_FRAME
	// Stop camera capture
	CLEyeCameraStop(_cam);
	// Destroy camera object
	CLEyeDestroyCamera(_cam);
	_cam = NULL;
#endif // USE_PS3_FOR_CURRENT_FRAME

	//dx may 16, 2012: display blank screen on kid's side
	monitor_n_Prompt.showPicPromptDlg.ShowWindow (SW_HIDE);
	monitor_n_Prompt.showPicSchedDlg.ShowWindow (SW_HIDE);
	monitor_n_Prompt.showTimerDlg.ShowWindow (SW_HIDE);

	//dx may 23, 2012: prompt views on CG side
	monitor_n_Prompt.showPicPromptDlg_CG.ShowWindow (SW_HIDE);
	monitor_n_Prompt.showPicSchedDlg_CG.ShowWindow (SW_HIDE);
	monitor_n_Prompt.showTimerDlg_CG.ShowWindow (SW_HIDE);




	//dx debug may 11, 2012: commented out
	//monitor_n_Prompt.showPicSchedDlg.DestroyWindow();
	//monitor_n_Prompt.showPicPromptDlg.DestroyWindow();
	//monitor_n_Prompt.showTimerDlg.DestroyWindow();
	////crashes if run the following command: //monitor_n_Prompt.ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\blackscreen.bmp");

	//dx debug may 11, 2012: commented out
	//monitor_n_Prompt.taskdone = true;

	monitor_n_Prompt.logFile.open(logFileName, ios::app);
	time (&monitor_n_Prompt.logtime);
	dif = difftime(monitor_n_Prompt.logtime, monitor_n_Prompt.trialstartingtime);
	monitor_n_Prompt.logFile << dif << "\t" << "\t" << "\t" << "\t" << "\t" << "\t"<< "\t" << "\t" << "The task has been completed" << endl;
	monitor_n_Prompt.logFile.close();
	//view->SetTimer(TIMER_ID_GRAB_IMAGES, TIMER_ID_CAMERA_PERIOD, 0); //dx may 15, 2012: commented out because video display should start with start button and stop with stop button

	//Nov. 16, 2011 - Justin
	actionModule.USRForStep[0] = monitor_n_Prompt.numPromptsGivenForEachState[0];
	actionModule.USRForStep[1] = monitor_n_Prompt.numPromptsGivenForEachState[1];
	actionModule.USRForStep[2] = monitor_n_Prompt.numPromptsGivenForEachState[2];
	actionModule.USRForStep[3] = monitor_n_Prompt.numPromptsGivenForEachState[3];
	actionModule.USRForStep[4] = monitor_n_Prompt.numPromptsGivenForEachState[4];

	//Debugging - Dec. 6, 2011
	ofstream testdata;
	testdata.open("C:\\dxFiles\\COACH4ASDresourceFiles\\Testdata.txt", ofstream::app);
	testdata << "USRForStep[0]: " << actionModule.USRForStep[0] << endl;
	testdata << "USRForStep[1]: " << actionModule.USRForStep[1] << endl;
	testdata << "USRForStep[2]: " << actionModule.USRForStep[2] << endl;
	testdata << "USRForStep[3]: " << actionModule.USRForStep[3] << endl;
	testdata << "USRForStep[4]: " << actionModule.USRForStep[4] << endl;
	testdata.close();

	//Nov. 11, 2011 - Justin
	//Print user history
	actionModule.EndPrompting();	
	//Show startupMenuDlg
	//startupMenuDlg.ShowWindow(SW_SHOW);		

	//dxnote sep 27, 2012: moved this to the workerThread right before it exits to ensure the last frame recorded is the same
	////dxnote sep 27, 2012: but it doesn't seem to work though. cameraFramesStampsFile always records 3 more frames after videoLog is ended
	////cameraFramesStampsFile.close(); 
}

void CAutismCOACHDoc::CallCaregiver()
{
	logFile_D.open(logFileName, ios::app);
	time (&monitor_n_Prompt.logtime);
	dif = difftime(monitor_n_Prompt.logtime, monitor_n_Prompt.trialstartingtime);
	logFile_D << dif << "\t" << "\t" << "\t" << "Caregiver has been alerted" << endl;
	logFile_D.close();

	PlaySound("C:\\Windows\\Media\\Afternoon\\Windows Error.wav", NULL, SND_FILENAME | SND_SYNC );

	OnFilePauseSystem();//Jan. 27, 2011
}

void CAutismCOACHDoc::TransitionToNextState()
{
	//if (monitor_n_Prompt.transitioning)
	//{
	//	//if state has changed...
	//	if (monitor_n_Prompt.state_has_changed)
	//	{
	//		//if state has incremented (rather than decremented)...
	//		if (monitor_n_Prompt.fwd_state_transition)
	//		{
	//			//If not rewarding completion of each step...
	//			if (monitor_n_Prompt.userSettingsDlg.rewardallsteps != REWARDALLSTEPS)
	//				//do not call RewardCompletionofStep, but set priming flag to true
	//				monitor_n_Prompt.ready2prime = true;
	//			//If rewarding completion of each step, then call RewardCompletionofStep()
	//			else if (monitor_n_Prompt.userSettingsDlg.rewardallsteps == REWARDALLSTEPS)
	//			{
	//				RewardCompletionofStep();
	//			}
	//		}
	//		//If this is not a forward transition - ie. child has moved backwards in the task by walking off/rinsing off too soon
	//		else if (!monitor_n_Prompt.fwd_state_transition)
	//		{
	//			//if the user must start over, play the start over prompt...
	//			if (monitor_n_Prompt.startover)
	//			{
	//				PlaySound(NULL, 0, 0);
	//				//mciSendString("close GoodJobPrompt", NULL, 0, 0);//Feb. 11, 2011
	//				mciSendString("open \"C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Verbal Prompts\\startover.wav\" type MPEGVideo alias StartOverPrompt", NULL, 0, 0);
	//				mciSendString("play StartOverPrompt from 0", NULL, 0, 0);
	//				StartOverSound = true;
	//				monitor_n_Prompt.startover = false;
	//			}
	//			//Otherwise just set priming flag to true
	//			else
	//				monitor_n_Prompt.ready2prime = true;//do nothing for now	
	//		}
	//		//Reset flags
	//		monitor_n_Prompt.fwd_state_transition = false;
	//		monitor_n_Prompt.state_has_changed = false;
	//	}
	//	/************************************************************************
	//	**Close Sounds/Videos that have stopped and move on to next sound/video**
	//	************************************************************************/

	//	char buffer [128];
	//	//close any priming video if it has stopped playing and show the picture prompt
	//	mciSendString("status myPrimingVideo mode", buffer, 128, 0);
	//		if (strcmp (buffer, "stopped") == 0 )
	//	{
	//		monitor_n_Prompt.showPicPromptDlg.ShowWindow(SW_SHOW);
	//		mciSendString("close myPrimingVideo", NULL, 0, 0);
	//		monitor_n_Prompt.transitioning = false;
	//		time(&monitor_n_Prompt.lastprompttime);//Note time Priming video was closed
	//	}

	//	//close the "start over" prompt if it has stopped playing
	//	if(StartOverSound)
	//	{
	//		mciSendString("status StartOverPrompt mode", buffer, 128, 0);
	//		if (strcmp (buffer, "stopped") == 0 )
	//		{
	//			mciSendString("close StartOverPrompt", NULL, 0, 0);
	//			StartOverSound = false;
	//			//set priming flag to true
	//			monitor_n_Prompt.ready2prime = true;
	//		}
	//	}
	//	
	//	//close the "good job" prompt if it has stopped playing
	//	else if(GoodJobSound)
	//	{
	//		//char buffer [128];
	//		mciSendString("status GoodJobPrompt mode", buffer, 128, 0);
	//			if (strcmp (buffer, "stopped") == 0 )
	//		{
	//			mciSendString("close GoodJobPrompt", NULL, 0, 0);
	//			GoodJobSound = false;
	//			
	//			//set priming flag to true
	//			monitor_n_Prompt.ready2prime = true;

	//		}
	//	}
	//	
	//	/*********************************************************************************
	//	** If it is time to prime, (or time to prompt for the next step in the task)... **
	//	*********************************************************************************/

	//	if ( monitor_n_Prompt.ready2prime )
	//	{
	//		//If task has been completed...
	//		if (monitor_n_Prompt.userState == TOTALNUMBEROFSTEPS)
	//		{
	//			TerminatePrompting();
	//			return;
	//		}
	//		//If the priming option has been selected, then call Prime()...
	//		if(monitor_n_Prompt.userSettingsDlg.priming == PRIME)
	//		{					
	//			Prime();
	//			//reset  priming flag
	//			monitor_n_Prompt.ready2prime = false;
	//		}
	//		//Otherwise, do nothing and just reset flags
	//		else
	//		{
	//			monitor_n_Prompt.ready2prime = false;
	//			monitor_n_Prompt.transitioning = false;
	//			time(&monitor_n_Prompt.lastprompttime);//Note time that user moved on to new state
	//		}
	//	}
	//}
}

void CAutismCOACHDoc::OnFilePauseSystem()
{
	//dx nov 11, 2012: make sure not in initialization stage
	if (!prompting)
		return;

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->OnFilePauseSystem();

	//dx dec 02, 2012: kill the scrubbingLoop_workerThread
	monitor_n_Prompt.scrubbingLoop_die = true;
	monitor_n_Prompt.initializeScrubbingLoop_GRABBING_MODE();
	if (TScrubbingLoopWorker != NULL)
		WaitForSingleObject(TScrubbingLoopWorker->m_hThread, INFINITE); //this wait is needed before killing the prompt loop and start the pause loop because pause loop has its own frame grabbing code, which interfere with recordVideos? in scrubbingLoop_workerThread
	TScrubbingLoopWorker = NULL;

	//dx may 09, 2012:
	CString s;
	s.Format("CG pause start,%d",monitor_n_Prompt.userState);
	monitor_n_Prompt.dLog.Write(s);

	PlaySound("C:\\WINDOWS\\Media\\ding.wav", NULL, SND_FILENAME | SND_ASYNC);

	logFile_D.open(logFileName, ios::app);
	time (&monitor_n_Prompt.logtime);
	dif = difftime(monitor_n_Prompt.logtime, monitor_n_Prompt.trialstartingtime);
	logFile_D << dif << "\t" << "\t" << "\t" << "\t" << "\t" << "\t" << "\t" << "\t" << "Paused System" << endl;
	logFile_D.close();
	
	////dx aug 29, 2012: make the default updateToState on pause dialogue to be the current userState
	//CString currentUserState;
	//currentUserState.Format("%d", monitor_n_Prompt.userState);
	////monitor_n_Prompt.pauseSystemDlg.updateToState.SetWindowTextA(currentUserState);
	//monitor_n_Prompt.pauseSystemDlg.currentUserState = currentUserState; //dxnote oct 28, 2012: needs to be before we create the pauseSystemDlg for the currentUserState text to be displayed correctly
	monitor_n_Prompt.pauseSystemDlg.userState = (CPauseSystemDlg::USER_STATE) monitor_n_Prompt.userState; //dx oct 28, 2012

	monitor_n_Prompt.pauseSystemDlg.DestroyWindow();
	monitor_n_Prompt.pauseSystemDlg.Create(IDD_PAUSESYSTEM_DIALOG, NULL);
	//dx may 16, 2012: maximized dialog
	//monitor_n_Prompt.pauseSystemDlg.ShowWindow(SW_SHOW);		
	monitor_n_Prompt.pauseSystemDlg.ModifyStyle(WS_CAPTION,0);
	monitor_n_Prompt.pauseSystemDlg.ShowWindow(SW_SHOWMAXIMIZED);
	monitor_n_Prompt.pauseSystemDlg.SetWindowPos(&monitor_n_Prompt.pauseSystemDlg.wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);	//dx may 23, 2012: make the dialog on top
	//monitor_n_Prompt.pauseSystemDlg.ShowWindow(SW_SHOW);//dxdebug aug 30, 2012:

	//dx aug 30, 2012: close all prompts
	closePrompts();
	monitor_n_Prompt.ShowPicturePromptImage("C:\\dxFiles\\COACH4ASDresourceFiles\\Media For COACH for ASD\\Picture Prompts\\blackscreen.bmp");//dx aug 31, 2012

	view->KillTimer(TIMER_ID_PROMPT);
	view->SetTimer(TIMER_ID_SYSTEM_PAUSE, TIMER_ID_CAMERA_PERIOD, 0);
}

void CAutismCOACHDoc::OnFileStarttracker()
{
	if (prompting)
		return;

	OnFileStartcamera();//Will initialize tracker and load calibration
	//Initialize the tracker and calibration//October 19, Justin
	MTCleanUp();
	//CleanUp();
	InitializeTrackerandCalibration();

	//Load calibration file
	//CString filename = userDataDlg.calibrationFileName;
	//calibDlg.SetTracker(tracker); 
	//calibDlg.LoadCalib(filename);

#ifdef USE_MT_VIDEO_WRITER
	writeFrameWorker_die = false; //dx sep 25, 2012: need this so the image created in recordVideosHelper can be released in the worker thread
#endif //USE_MT_VIDEO_WRITER

	//Start timer event
	//view->SetTimer(TIMER_ID_STARTTRACKER, TIMER_TRACKER_PERIOD, 0);
	view->KillTimer(TIMER_ID_GRAB_IMAGES); //dx aug 31, 2012
	view->SetTimer(TIMER_ID_STARTTRACKER, TIMER_ID_CAMERA_PERIOD, 0);
}

void CAutismCOACHDoc::GetBackground(int region)
{
	//Get left tap centre and radius
	//Go to currentframe, from x = centre - r, y = centre - r to x = centre + r, y = centre + r
	//and copy the values to an array/IplImage

	//REGION_TAP_LEFT - white
	int xc = calibDlg.regions[region].centreX;
	int yc = calibDlg.regions[region].centreY;
	int radius = calibDlg.regions[region].radius;

	int startx = xc - radius;
	int starty = yc - radius;
	int stopx = xc + radius;
	int stopy = yc + radius;
	int width = stopx - startx + 1;
	int height = stopy - starty + 1;

	//Set background for left hand

	//cvShowImage("currentFrame", currentFrame);
	cvSetImageROI( currentFrame, cvRect(startx, starty, width, height) );//Jan 7
	//cvShowImage("currentFrameROI", currentFrame);

	if (region == 1)//left tap
	{
		if (trainingWaterDetector)
		{
			gray_backgroundImage_left = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth, 1);
			cvCvtColor(currentFrame,gray_backgroundImage_left,CV_BGR2GRAY);
			//cvSaveImage("C:\\dxFiles\\COACH4ASDresourceFiles\\BackgroundImageLeft.jpg", gray_backgroundImage_left);
			cvSaveImage("C:\\dxFiles\\COACH4ASDresourceFiles\\BackgroundImageLeft.jpg", gray_backgroundImage_left);
		}
		else
		{
			IplImage* tempImage = cvLoadImage("C:\\dxFiles\\COACH4ASDresourceFiles\\BackgroundImageLeft.jpg");
			gray_backgroundImage_left = cvCreateImage(cvGetSize(tempImage), tempImage->depth, 1);
			cvCvtColor(tempImage,gray_backgroundImage_left,CV_BGR2GRAY);
			safe_release_image(tempImage);
		}
	}
	else if (region ==2)//right tap
	{
		if (trainingWaterDetector)
		{
			gray_backgroundImage_right = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth, 1);
			cvCvtColor(currentFrame,gray_backgroundImage_right,CV_BGR2GRAY);
			//cvShowImage("Backgroundright", gray_backgroundImage_right);
			//cvShowImage("currentFrame", currentFrame);
			//cvSaveImage("C:\\dxFiles\\COACH4ASDresourceFiles\\BackgroundImageRight.jpg", gray_backgroundImage_right);
			cvSaveImage("C:\\dxFiles\\COACH4ASDresourceFiles\\BackgroundImageRight.jpg", gray_backgroundImage_right);
		}
		else
		{
			IplImage* tempImage = cvLoadImage("C:\\dxFiles\\COACH4ASDresourceFiles\\BackgroundImageRight.jpg");
			gray_backgroundImage_right = cvCreateImage(cvGetSize(tempImage), tempImage->depth, 1);
			cvCvtColor(tempImage,gray_backgroundImage_right,CV_BGR2GRAY);
			safe_release_image(tempImage);
		}
	}
	cvResetImageROI(currentFrame);
}

void CAutismCOACHDoc::GetRegionForeground(int region)//LEFT = 1, RIGHT = 2
{
	//REGION_TAP
	int xc = calibDlg.regions[region].centreX;
	int yc = calibDlg.regions[region].centreY;
	int radius = calibDlg.regions[region].radius;

	int startx = xc - radius;
	int starty = yc - radius;
	int stopx = xc + radius;
	int stopy = yc + radius;
	int width = stopx - startx + 1;
	int height = stopy - starty + 1;

	//Create an image for the foreground for left hand
	cvSetImageROI( currentFrame, cvRect(startx, starty, width, height) );

	gray_currentFrame = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth, 1);

	if (region == 1)//left tap
	{
		safe_release_image(gray_foregroundImage_left);//This line MUST preceed the next!
		gray_foregroundImage_left = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth, 1);
		cvCvtColor(currentFrame, gray_currentFrame,CV_BGR2GRAY);
		cvResetImageROI(currentFrame);
		//Get the foreground
		cvAbsDiff(gray_currentFrame, gray_backgroundImage_left, gray_foregroundImage_left);		
#ifdef DEBUG	//dxdebug oct 24, 2012:
		cvSaveImage("C:\\dxFiles\\COACH4ASDresourceFiles\\waterDetector_left_gray_backgroundImage.jpg", gray_backgroundImage_left);
		cvSaveImage("C:\\dxFiles\\COACH4ASDresourceFiles\\waterDetector_left_gray_currentFrame.jpg", gray_currentFrame);
		cvSaveImage("C:\\dxFiles\\COACH4ASDresourceFiles\\waterDetector_left_gray_foregroundImage.jpg", gray_foregroundImage_left);			
#endif // DEBUG
	}
	else if (region == 2)//left tap
	{
		safe_release_image(gray_foregroundImage_right);//This line MUST preceed the next!
		gray_foregroundImage_right = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth, 1);
		cvCvtColor(currentFrame, gray_currentFrame,CV_BGR2GRAY);
		cvResetImageROI(currentFrame);
		//Get the foreground
		cvAbsDiff(gray_currentFrame, gray_backgroundImage_right, gray_foregroundImage_right);//Code crashes here because tap regions were just reset
#ifdef DEBUG	//dxdebug oct 24, 2012:
		cvSaveImage("C:\\dxFiles\\COACH4ASDresourceFiles\\waterDetector_right_gray_backgroundImage.jpg", gray_backgroundImage_right);
		cvSaveImage("C:\\dxFiles\\COACH4ASDresourceFiles\\waterDetector_right_gray_currentFrame.jpg", gray_currentFrame);
		cvSaveImage("C:\\dxFiles\\COACH4ASDresourceFiles\\waterDetector_right_gray_foregroundImage.jpg", gray_foregroundImage_right);
#endif // DEBUG
	}
}

void CAutismCOACHDoc::OnFileSwitchImage()
{
	if (videoplaytype == 3)
		videoplaytype = 1;
	else if (videoplaytype == 1)
		videoplaytype = 2;
	else if (videoplaytype == 2)
		videoplaytype = 3;
}
void CAutismCOACHDoc::OnFileToggleWaterStatus()
{
	if (monitor_n_Prompt.wateron == false)
		monitor_n_Prompt.wateron = true;
	else if (monitor_n_Prompt.wateron == true)
		monitor_n_Prompt.wateron = false;
}

bool CAutismCOACHDoc::IsAlignByte(int count, int width, int widthstep)
{
	int column = (count) % widthstep; 
	if ( column > (width - 1) )
		return true;
	return false;
}

int CAutismCOACHDoc::Magnitude(int number)
{
	if (number < 0)
		number = -1*number;
	return number;
}

void CAutismCOACHDoc::OnFileSkipScrubAndRinseDetection()
{

	if (!opticalFlow.RUBBINGHANDSDETECTEDBYOPTICALFLOW)
		opticalFlow.RUBBINGHANDSDETECTEDBYOPTICALFLOW = true;
	else
		opticalFlow.RUBBINGHANDSDETECTEDBYOPTICALFLOW = false;

	monitor_n_Prompt.skipscrub_n_rinsetracking = true;
}

void CAutismCOACHDoc::InitializeWaterDetectorIfNeeded()
{
	if (waterDetectorIsInitialized == true)
	{
		// do nothing
	}
	else
	{
		int waterX = calibDlg.regions[REGION_SINK].centreX;
		int waterY = calibDlg.regions[REGION_SINK].centreY;
		float waterR = calibDlg.regions[REGION_SINK].radius;
		waterDetector.Init(waterX, waterY, waterR);

		waterDetectorIsInitialized = true;
	}
}

void CAutismCOACHDoc::OnFileTrainWaterDetector()
{
	StartTrainingWaterDetector();
}

void CAutismCOACHDoc::SaveTapImagesandData(CString imagetype)
{
	//Save Images
	cvSaveImage(BWImage_left[testCase], gray_backgroundImg_left);
	cvSaveImage(BWImage_right[testCase], gray_backgroundImg_right);

	//Save Data
	if (imagetype == "foreground")
	{
		//		WriteImageData2File(gray_rawdiffImg_left, BWImageData_left[testCase]);
		//		WriteImageData2File(gray_rawdiffImg_right, BWImageData_right[testCase]);
	}
	if (imagetype == "background")
	{
		WriteImageData2File(gray_backgroundImg_left, BWImageData_left[testCase]);
		WriteImageData2File(gray_backgroundImg_right, BWImageData_right[testCase]);
	}
}
void CAutismCOACHDoc::SaveMainImage()
{
	//save the colour image to file
	cvSaveImage(MainImage[testCase], currentFrame);

}
void CAutismCOACHDoc::SaveSkinImage()
{
	//save the skin image to file
	cvSaveImage(SkinImage[testCase], skinImg);
}

//void CAutismCOACHDoc::WriteImageData2File(IplImage* &writeImg, CString side)
//{
//	//Open the file for writing
//
//	ofstream imagedatafile;
//
//	if (side == "left")
//		imagedatafile.open(BWImageData_left[testCase]);
//	if (side == "right")
//		imagedatafile.open(BWImageData_right[testCase]);
//	
//	for (int count = 0; count < writeImg->imageSize; count++)
//	{
//		if( !IsAlignByte(count, writeImg->width, writeImg->widthStep) )
//		{
//			//write data to file
//			int imgdatum = Magnitude( (int)writeImg->imageData[count] );
//
//			imagedatafile << imgdatum << endl;
//		}
//	}
//	//close file
//	imagedatafile.close();
//}

void CAutismCOACHDoc::WriteImageData2File(IplImage* &writeImg, CString filename)
{
	//Open the file for writing

	ofstream imagedatafile;

	imagedatafile.open(filename);

	for (int count = 0; count < writeImg->imageSize; count++)
	{
		if( !IsAlignByte(count, writeImg->width, writeImg->widthStep) )
		{
			//write data to file
			int imgdatum = Magnitude( (int)writeImg->imageData[count] );

			imagedatafile << imgdatum << endl;
		}
	}
	//close file
	imagedatafile.close();
}

void CAutismCOACHDoc::GetTapFeatures(IplImage* tapImg, CString side)
{
	for (int count = 0; count < tapImg->imageSize; count++)
	{
		if( !IsAlignByte(count, tapImg->width, tapImg->widthStep) )
		{
			//count number of elements in each bin of histogram
			//8 bins for 129 possible values - each bin holds 16 values (17 in last bin)

			int imgdatum = Magnitude( (int)tapImg->imageData[count] );

			if (side == "left") 
			{					
				if (imgdatum > 128-1*16)
					waterDetector.bins_left[7] ++;
				else if (imgdatum > 128-2*16)
					waterDetector.bins_left[6] ++;
				else if (imgdatum > 128-3*16)
					waterDetector.bins_left[5] ++;
				else if (imgdatum > 128-4*16)
					waterDetector.bins_left[4] ++;
				else if (imgdatum > 128-5*16)
					waterDetector.bins_left[3] ++;
				else if (imgdatum > 128-6*16)
					waterDetector.bins_left[2] ++;
				else if (imgdatum > 128-7*16)
					waterDetector.bins_left[1] ++;
				else if (imgdatum >= 128-8*16)
					waterDetector.bins_left[0] ++;
			}
			else if (side == "right")
			{			
				if (imgdatum > 128-1*16)
					waterDetector.bins_right[7] ++;
				else if (imgdatum > 128-2*16)
					waterDetector.bins_right[6] ++;
				else if (imgdatum > 128-3*16)
					waterDetector.bins_right[5] ++;
				else if (imgdatum > 128-4*16)
					waterDetector.bins_right[4] ++;
				else if (imgdatum > 128-5*16)
					waterDetector.bins_right[3] ++;
				else if (imgdatum > 128-6*16)
					waterDetector.bins_right[2] ++;
				else if (imgdatum > 128-7*16)
					waterDetector.bins_right[1] ++;
				else if (imgdatum >= 128-8*16)
					waterDetector.bins_right[0] ++;
			}
		}
	}
}

void CAutismCOACHDoc::ResetTapFeatures()
{
	waterDetector.bins_left[0] = waterDetector.bins_left [1] = waterDetector.bins_left [2] = waterDetector.bins_left [3] = waterDetector.bins_left [4] = waterDetector.bins_left [5] = waterDetector.bins_left [6] = waterDetector.bins_left [7] = 0;
	waterDetector.bins_right[0] = waterDetector.bins_right [1] = waterDetector.bins_right [2] = waterDetector.bins_right [3] = waterDetector.bins_right [4] = waterDetector.bins_right [5] = waterDetector.bins_right [6] = waterDetector.bins_right [7] = 0;
}

void CAutismCOACHDoc::GetWaterStatusbyWaterFeatures()
{
	//dx oct 23, 2012: this shouldn't be used; use GetWaterStatus() instead
	//if (nwater_obs > 0)
	//	monitor_n_Prompt.wateron = true;

	//return;


	//double probability = 0;

	////double weight1 = 1;
	////double weight2 = 1;
	////double weight3 = 1;
	////double weight4 = 1;
	////double weight5 = 1;
	////double weight6 = 1;
	////double weight7 = 1;
	////double weight8 = 1;
	////double weight9 = 1;
	////double weight10 = 1;

	//double weights[10] = {7.4435, 7.4435, -12.54, 0.43715, -0.0026699, -12.54, 0.43715, -0.0026699, -0.0000052866, -0.0000052866};


	//probability += waterDetector.entropy1 * weights[0];
	//probability += waterDetector.entropy2 * weights[1];
	//probability += waterDetector.firstOrderDerivativeHistFirstMoment * weights[2];
	//probability += waterDetector.firstOrderDerivativeHistSecondMoment * weights[3];
	//probability += waterDetector.firstOrderDerivativeHistThirdMoment * weights[4];
	//probability += waterDetector.secondOrderDerivativeHistFirstMoment * weights[5];
	//probability += waterDetector.secondOrderDerivativeHistSecondMoment * weights[6];
	//probability += waterDetector.secondOrderDerivativeHistThirdMoment * weights[7];
	//probability += waterDetector.maskedRegion1Size * weights[8];
	//probability += waterDetector.maskedRegion2Size * weights[9];


	//double e = 2.1873;
	//probability = -1*probability;
	//probability = 1 / (1 + pow(e, probability));

	//if (probability > 0.5)
	//	monitor_n_Prompt.wateron = true;//add to monitor and prompt
	//else
	//	monitor_n_Prompt.wateron = false;//add to monitor and prompt
}

void CAutismCOACHDoc::GetWaterStatus(int numtapleftobs, int numtaprightobs)//Dec. 15, 2010
{
	if (first_iteration)
	{
		first_iteration = false;
		tapDetectorL.predictProbabilityOfBackgroundChange_init(currentFrame, false);
		tapDetectorR.predictProbabilityOfBackgroundChange_init(currentFrame, false);

		return;
	}
	else
	{
		bool backgroundChanged = false;
		bool noHandOcclusion = (numtapleftobs == 0) && (numtaprightobs == 0);
		if (noHandOcclusion)
			backgroundChanged = (tapDetectorL.predictProbabilityOfBackgroundChange(currentFrame) > 0.5) || (tapDetectorR.predictProbabilityOfBackgroundChange(currentFrame) > 0.5);
		//dxnote oct 25, 2012: it was stupid of me to use waterOn as the log variable logging the wateron.  it's confusing and dangerous.
		monitor_n_Prompt.wateron = backgroundChanged; //dx oct 25, 2012
	}
}

void CAutismCOACHDoc::GatherTrainingData()
{
	if(testCase%100 == 0)
		PlaySound("C:\\WINDOWS\\Media\\ding.wav", NULL, SND_FILENAME | SND_ASYNC);

	if (testCase == 0)
	{
		////dx debug oct 24, 2012:
		//cvSaveImage("C:\\dxFiles\\COACH4ASDresourceFiles\\REGION_TAP_LEFT_currentFrame_beforeDetectorInit.jpg", currentFrame);

		tapDetectorL.predictProbabilityOfBackgroundChange_init(currentFrame, true);
		tapDetectorR.predictProbabilityOfBackgroundChange_init(currentFrame, true);

		faucetOcclusionDetector.predictProbabilityOfBackgroundChange_init(currentFrame, true);
		towelLiftDetector.predictProbabilityOfBackgroundChange_init(currentFrame, true);


		GetBackground(1);
		GetBackground(2);

		//Print header in FeatureFile
		waterDetector.ComputeVideoFeatures(currentFrame, skinImageRaw, testCase, true, groundtruth_label);//Feb. 23, 2011
		ResetTapFeatures();
		//Sleep(100);
		testCase++;
		return;
	}
	else
	{
		if (!userPromptedToContinueWaterDetectionTraining)//Feb. 23, 2011
		{
			userPromptedToContinueWaterDetectionTraining = true;
			trainWaterDetectorDlg.Create(IDD_TRAINWATERDETECTOR, NULL);
			trainWaterDetectorDlg.ShowWindow(SW_SHOW);
		}
		else if (trainWaterDetectorDlg.userReadyToContinue)
		{
			tapDetectorL.setGt_label(trainWaterDetectorDlg.waterDetectionCategory);
			tapDetectorL.predictProbabilityOfBackgroundChange(currentFrame);
			tapDetectorR.setGt_label(trainWaterDetectorDlg.waterDetectionCategory);
			tapDetectorR.predictProbabilityOfBackgroundChange(currentFrame);

			faucetOcclusionDetector.setGt_label(trainWaterDetectorDlg.waterDetectionCategory);
			faucetOcclusionDetector.predictProbabilityOfBackgroundChange(currentFrame);

			towelLiftDetector.setGt_label(trainWaterDetectorDlg.waterDetectionCategory);
			towelLiftDetector.predictProbabilityOfBackgroundChange(currentFrame);
			

			groundtruth_label = trainWaterDetectorDlg.waterDetectionCategory;//Feb. 23, 2011

			GetRegionForeground(1);
			GetRegionForeground(2);

			//Get the tap features
			GetTapFeatures(gray_foregroundImage_left, "left");
			GetTapFeatures(gray_foregroundImage_right, "right");

			//Print features to FeatureFile
			waterDetector.ComputeVideoFeatures(currentFrame, skinImageRaw, testCase, true, groundtruth_label);//Feb. 23, 2011
			ResetTapFeatures();

			testCase++;
		}
	}
}

void CAutismCOACHDoc::CheckHandPositions(int userState, IplImage *currentFrame, IplImage *skinImg)//Dec. 21//Jan 5 - 1
{
	if ( userState < 5 )//see if soap is being used
	{
		CheckToSeeIfGettingSoap();
	}

	//if ( userState == 2 )/*|| (userState == 3)*//*dx aug 24, 2012: I don't think we need to check if the user is scrubbing or not if he is already rinsing hands*/ //If the user has applied soap to the hands, check to see if they are scrubbing
	//if (userState ==2 || userState == 3) //dx nov 12, 2012: added userState == 3 so we can check numblobsdetected in rinsing, too
	if (userState == 2) //dx nov 29, 2012: we don't need numblobsdetected in rinsing anyways
	{
		ofstream loopRateFile ("C:\\dxFiles\\COACH4ASDresourceFiles\\loopRate.txt", ios::app);
		DWORD loopStartTime, loopNowTime;
		loopRateFile << "\tIn CheckHandPositions: " << endl;
		loopStartTime = GetTickCount();

		//Show optical flow
		//Copy the blue channel from skinImg to splitImg
		cvSplit(skinImg, splitImg, NULL, NULL, NULL);
		//threshold the splitImg		
		cvThreshold(splitImg, thresholdImg, 100, 255, CV_THRESH_BINARY);
		//Dilate the thresholdImg
		cvMorphologyEx( thresholdImg, thresholdImg, 0, 0, CV_MOP_CLOSE, 20 );//closing dilates first then erodes
		cvMorphologyEx( thresholdImg, thresholdImg, 0, 0, CV_MOP_OPEN,  3 );

		loopNowTime = GetTickCount();
		loopRateFile << "\t" << loopNowTime - loopStartTime << ", done split, threshold, and morphologyEx" << endl;
		loopStartTime = GetTickCount();

		//Get the number of blobs detected in thresholdImg
		//CBlob *currentBlob;
		//blobs = CBlobResult(thresholdImg, NULL, 0);
		//int numblobsdetected = blobs.GetNumBlobs();

		////dxdebug may 30, 2012
		//int numblobsdetected = CBlobResult(thresholdImg, NULL, 0).GetNumBlobs();

		CBlobResult blobs;
		//blobs = CBlobResult();
		blobs = CBlobResult(thresholdImg, NULL, 0);
		//int numblobsdetected = 0;
		numblobsdetected = 0; //dx nov 12, 2012: moved numblobsdetected from a temporary var to a member var, so decision can check numblobsdetected during scrubbing and rinsing
		numblobsdetected = blobs.GetNumBlobs();

		loopNowTime = GetTickCount();
		loopRateFile << "\t" << loopNowTime - loopStartTime << ", done blobresult" << endl;
		loopStartTime = GetTickCount();

		//Mask currentFrame with thresholdImg when copying to ShowImg
		cvZero(ShowImg);
		cvCopy(currentFrame, ShowImg, thresholdImg); //dxnote sep 15, 2012: so opticalFlow operates on the currentFrame (which is the colour image of the webcam), but is thresholded by the blobs image of the hand

		loopNowTime = GetTickCount();
		loopRateFile << "\t" << loopNowTime - loopStartTime << ", done cvCopy" << endl;
		loopStartTime = GetTickCount();

		//Call DoOpticalFlow with ShowImg
		opticalFlow.DoOpticalFlow(ShowImg);//Jan 5 - 2//Jan 7

		loopNowTime = GetTickCount();
		loopRateFile << "\t" << loopNowTime - loopStartTime << ", done DoOpticalFlow" << endl;
		loopStartTime = GetTickCount();

		loopRateFile.close();

		bool tempOpticalFlowResult; //dx oct 19, 2012
		//If rubbing was detected by optical flow
		if (opticalFlow.RUBBINGHANDSDETECTEDBYOPTICALFLOW )
		{
			//monitor_n_Prompt.RUBBINGHANDSDETECTEDBYOPTICALFLOW = true;//Jan 5 - 4
			tempOpticalFlowResult = true;
		}
		//Else, rubbing hands were not detected
		else
		{
			//PlaySound("C:\\WINDOWS\\Media\\ding.wav", NULL, SND_FILENAME | SND_ASYNC);
			//monitor_n_Prompt.RUBBINGHANDSDETECTEDBYOPTICALFLOW = false;
			tempOpticalFlowResult = false;
		}

		if (numblobsdetected != 1)//January 10, 2012 //sep 16, 2012
			//if (numblobsdetected <= 3)//dxtodo aug 24, 2012: too noisy, changed from 1 to 3
				////If 0, or more than 1 blob - rule out possibility of rubbing hands
					//else
		{
			////opticalFlow.RUBBINGHANDSDETECTEDBYOPTICALFLOW = false; //dxnote oct 19, 2012: i think this was a typo, meant to change the variable in monitor_n_Prompt
			//monitor_n_Prompt.RUBBINGHANDSDETECTEDBYOPTICALFLOW = false;
			tempOpticalFlowResult = false;
		}
		scrubbingDetectorResults.push_back(tempOpticalFlowResult);

		//dx may 26, 2012: commented out the scrubbing bypass
		////dx may 11, 2012: try:
		//monitor_n_Prompt.RUBBINGHANDSDETECTEDBYOPTICALFLOW = true;

		//For Debugging
		//ofstream numscrubfeats;
		//numscrubfeats.open("C:\\dxFiles\\COACH4ASDresourceFiles\\ScrbFeats.txt", ios::app);
		//numscrubfeats << opticalFlow.numfeaturesinlast7frames << "\t" << clock() << endl;
		//numscrubfeats.close();
	}
//	if (userState == 3 || userState == 4 )//If the user has finished scrubbing or rinsing...
//	{
//		if (!monitor_n_Prompt.dried)
//		{
//			CheckToSeeIfHandsDried();
//		}
//	}
}

void CAutismCOACHDoc::CheckToSeeIfGettingSoap()
{
	/*Hands must be at soap dispenser for a certain amount of time
	without leaving in order for gettingsoap to be considered true

	When the hands enter the soap dispenser regions, start timing.
	While the hands remain at the soap dispenser, check to see how much time has elapsed
	If enough time has gone by, mark gettingsoap as true
	gettingsoap remains false if hands have not been in soap regions for enough time
	gettingsoap is reset to false whenever hands are not in soap regions*/

	//if hands are at the soap dispenser...
	if((nsoap_obs > 0)&&(nsoapspout_obs > 0) )//&&(nliquidsoap_obs > 0) )
	{
		//if the hands WERE NOT at the soap dispenser in the last frame...
		if (!monitor_n_Prompt.hands_were_at_soap_dispenser_in_last_frame)
		{
			//start timing
			monitor_n_Prompt.hands_were_at_soap_dispenser_in_last_frame = true;
			time (&startsoaptime);
		}

		//if the hands WERE at the soap dispenser in the last frame...
		else
		{
			//How much time has gone by?
			time (&currenttime);
			dif = difftime(currenttime, startsoaptime);

			//Mark gettingsoap flag true if enough time has gone by
			//if(dif > monitor_n_Prompt.done_getting_soap_time)//Jan 7
			if(dif > monitor_n_Prompt.userSettingsDlg.soapTime) //dx oct 29, 2012: make soapTime adjustable by user
				monitor_n_Prompt.gettingsoap = true;//status of gettingsoap is set to true
			else
				monitor_n_Prompt.gettingsoap = false;//status of gettingsopap remains false
		}
	}
	//if the hands are not at the soap dispenser...
	else
	{
		monitor_n_Prompt.gettingsoap = false;//status of gettingsopap remains false /reset to false
		monitor_n_Prompt.hands_were_at_soap_dispenser_in_last_frame = false;
	}
}


//void CAutismCOACHDoc::CheckToSeeIfHandsDried()
//{
//	//Because hand on towel detection is poor...first establish if hands are really touching the towel:
//	if( ntowel_obs > 0 )
//	{
//		//scan image for red blobs
//		//Copy the red channel from skinImg to splitImg
//		cvSplit(skinImg, NULL, NULL, splitImg, NULL);
//		//threshold the splitImg		
//		cvThreshold(splitImg, thresholdImg, 100, 255, CV_THRESH_BINARY);
//
//		//cvShowImage("Split and Thresholded Image",thresholdImg);
//
//		//Get the number of blobs detected in thresholdImg
//		//CBlob *currentBlob;
//		//blobs = CBlobResult(thresholdImg, NULL, 0);
//		//int numblobsdetected = blobs.GetNumBlobs();
//
//		////dxdebug may 30, 2012
//		//int numblobsdetected = CBlobResult(thresholdImg, NULL, 0).GetNumBlobs();
//		CBlobResult blobs;
//		blobs = CBlobResult(thresholdImg, NULL, 0);
//		int numblobsdetected = 0;
//		numblobsdetected = blobs.GetNumBlobs();
//
//		//if towel is not visible
//		if (numblobsdetected == 0)
//		{
//			ntowel_obs = 0;
//		}
//	}
//
//
//	/*Hands must be touching towel continuously for a certain amount
//	of time in order for 'dried' to be considered true
//
//	When the hands first touch the towel, start timing.
//	While the hands remain with the towel, check to see how much time has elapsed.
//	If enough time has gone by, mark 'dried' as true.
//	'dried' remains false if hands have not been touching towel for enough time.
//	'dried' is reset to false whenever hands are not touching towel. */
//
//	//if hands are at the towel...
//	if( ntowel_obs > 0 )
//	{
//		//if the hands WERE NOT touching the towel in the last frame...
//		//if (!monitor_n_Prompt.hands_were_at_towel_in_last_frame)
//		//{
//		//start timing
//		//	monitor_n_Prompt.hands_were_at_towel_in_last_frame = true;
//		if (!monitor_n_Prompt.hands_were_at_towel_in_last_frame)
//		{
//			monitor_n_Prompt.hands_were_at_towel_in_last_frame = true;
//			time (&monitor_n_Prompt.drytime);
//		}
//		//}
//
//		//if the hands WERE touching the towel in the last frame...
//		//else
//		//{
//		//How much time has gone by?
//		time (&currenttime);
//		dif = difftime(currenttime, monitor_n_Prompt.drytime);
//
//		//Mark 'dried' true if enough time has gone by
//		if(dif > monitor_n_Prompt.done_drying_time)
//		{
//			monitor_n_Prompt.dried = true;//status of 'dried' is set to true
//			//PlaySound("C:\\WINDOWS\\Media\\notify.wav", NULL, SND_FILENAME | SND_ASYNC);//Mar. 11, 2011
//		}
//		else
//			monitor_n_Prompt.dried = false;//status of 'dried' remains false
//		//}
//	}
//	//if the hands are not touching the towel...
//	else
//	{
//		monitor_n_Prompt.dried = false;//status of gettingsopap remains false /reset to false
//		//	monitor_n_Prompt.hands_were_at_towel_in_last_frame = false;
//	}
//}

void CAutismCOACHDoc::ReadInMostRecentUserData()
{
	ifstream mostrecentuserdata;
	string line;
	mostrecentuserdata.open("C:\\dxFiles\\COACH4ASDresourceFiles\\MostRecentUserData.txt");
	if (mostrecentuserdata.is_open())
	{
		getline(mostrecentuserdata, line);
		userDataDlg.participantName = line.c_str();//Get participant name

		getline(mostrecentuserdata, line);
		userDataDlg.trialDate = line.c_str();//Get trial date

		getline(mostrecentuserdata, line);
		userDataDlg.trialNumber = atoi(line.c_str());//Get trial number

		getline(mostrecentuserdata, line);
		userDataDlg.trialStartTime = line.c_str();//Get trial start time

		getline(mostrecentuserdata, line);
		userDataDlg.calibrationFileName = line.c_str();//Get calibration file

		mostrecentuserdata.close();
	}
	else cout << "Unable to open file";//for debug purposes only
	mostrecentuserdata.close();
	userDataDlg.DoModal();
}
void CAutismCOACHDoc::OnFileManuallyIncrementState()//Dec. 1 - This code is for debugging purposes only
{
	monitor_n_Prompt.userState++;
	monitor_n_Prompt.state_has_changed = true;

	//if (monitor_n_Prompt.userState == 2)//commented out January 12, 2012
	//{
	//	time (&monitor_n_Prompt.lastcounttime);
	//	monitor_n_Prompt.showTimerDlg.ResizeLargeImage(FIRSTMONITORWIDTH + SECONDMONITORWIDTH/2 - 100, SECONDMONITORHEIGHT - 200 - SECONDMONITORHEIGHT/5, 200, 200);
	//}


	//if (monitor_n_Prompt.userState == TOTALNUMBEROFSTEPS)
	//{
	//	TerminatePrompting();
	//view->KillTimer(TIMER_ID_PROMPT);
	//
	//monitor_n_Prompt.showPicSchedDlg.DestroyWindow();
	//monitor_n_Prompt.showPicPromptDlg.DestroyWindow();
	//monitor_n_Prompt.showTimerDlg.DestroyWindow();

	////Nov. 16, 2011 - Justin
	//actionModule.USRForStep[0] = 5;
	//actionModule.USRForStep[1] = 4;
	//actionModule.USRForStep[2] = 3;
	//actionModule.USRForStep[3] = 2;
	//actionModule.USRForStep[4] = 1;

	////Nov. 11, 2011 - Justin
	////Print user history
	//actionModule.EndPrompting();
	//}
}

void CAutismCOACHDoc::OnFileStoppromptingsession(void)
{
	//dx nov 11, 2012: make sure not in initialization stage
	if (!prompting)
		return;
	
	TerminatePrompting();	
	
	//closePrompts(); //dx aug 30, 2012
	//dx aug 30, 2012: premature stop dialog
	if(prematureStopDlg.DoModal() == IDOK)
	{
		CString prematureStopReason;
		prematureStopReason.Format("premature stop,%d", prematureStopDlg.reasonPremature);
		monitor_n_Prompt.dLog.Write(prematureStopReason);
	}

	//dx may 11, 2012: making the stop and restart function
	monitor_n_Prompt.sessionEndLog();

}

//dx may 24, 2012:
void CAutismCOACHDoc::OnViewPromptView(void)
{
	//dx nov 11, 2012: make sure not in initialization stage
	if (!prompting)
		return;

	if(monitor_n_Prompt.showPromptInCG)
	{
		monitor_n_Prompt.showPromptInCG = FALSE;
		monitor_n_Prompt.showPicSchedDlg_CG.ShowWindow(SW_HIDE);
		monitor_n_Prompt.showPicPromptDlg_CG.ShowWindow(SW_HIDE);
		monitor_n_Prompt.showTimerDlg_CG.ShowWindow(SW_HIDE);
	}
	else
	{
		monitor_n_Prompt.showPromptInCG = TRUE;
		monitor_n_Prompt.showPicSchedDlg_CG.ShowWindow(SW_SHOW);
		monitor_n_Prompt.showPicPromptDlg_CG.ShowWindow(SW_SHOW);
		//monitor_n_Prompt.showTimerDlg_CG.ShowWindow(SW_SHOW); //dx may 24, 2012: commented out because this cause a white window to occur in the middle of scrn
	}
}


//dx dec 02, 2012:
void CAutismCOACHDoc::basicAnnotation(bool write2File, string loopState)
{//timestamp, session number, regions
	
	//copy to the annotated version
	CSingleLock lock(&csVideoFrame); //For the sake of scrubbingLoop_workerThread
	lock.Lock();
	cvCopyImage(currentFrame, currentFrame_annotationCopy);
	cvCopyImage(skinImg, skinImg_annotationCopy);
	cvCopyImage(currentFrameWhole, currentFrameWhole_annotationCopy);
	lock.Unlock();

	//Initialise font
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_DUPLEX ,0.3f, 0.3f, 0, 1);
	CvScalar regioncolor = CV_RGB(0,255,0);

	if (loopState.compare("prompt")==0 || loopState.compare("pause")==0) //dxnote sep 25, 2012: shouldn't do anything but put regions and time on the currentFrame_annotationCopy, etc. if not prompting/pause (i.e. if in tracker/trainWater [note: trainScrubbing don't call this function, trainWater and tracker calls it to annotate the regions])
	{	
		//annotate session number
		CvPoint topTextPosL = cvPoint(5, 10);
		CString topText;
		topText.Format("session %d",monitor_n_Prompt.sessionNumber);
		cvPutText(currentFrame_annotationCopy, topText, topTextPosL, &font, regioncolor);
		cvPutText(currentFrameWhole_annotationCopy, topText, topTextPosL, &font, regioncolor);
		cvPutText(skinImg_annotationCopy, topText, topTextPosL, &font, regioncolor);

		//time stamp
		CvPoint topTextPosR = cvPoint(450, 10);
		//CString timeStamp = monitor_n_Prompt.dLog.GetTimeStamp();
		CString timeStamp = monitor_n_Prompt.timeStamp2CStr(); //dx dec 01, 2012

		//topText.Format("session %d",monitor_n_Prompt.sessionNumber);
		cvPutText(currentFrame_annotationCopy, timeStamp, topTextPosR, &font, regioncolor);
		cvPutText(currentFrameWhole_annotationCopy, timeStamp, topTextPosR, &font, regioncolor);
		cvPutText(skinImg_annotationCopy, timeStamp, topTextPosR, &font, regioncolor);
		
		//write to file: note only writting to file if in prompting/pause
		if (write2File)
		{
#ifdef USE_MT_VIDEO_WRITER
			writeFrameTasksQueue_addTask(writer, currentFrame_annotationCopy);
			writeFrameTasksQueue_addTask(writerWhole, currentFrameWhole_annotationCopy);
			writeFrameTasksQueue_addTask(writerSkinImg, skinImg_annotationCopy);
			//WaitForSingleObject(TWriteFrameWorker->m_hThread, INFINITE);
#else
			int writeFrameResult;
			writeFrameResult = cvWriteFrame(writer, currentFrame_annotationCopy);
			assert(writeFrameResult==1);
			writeFrameResult = cvWriteFrame(writerWhole, currentFrameWhole_annotationCopy);
			assert(writeFrameResult==1);
			writeFrameResult = cvWriteFrame(writerSkinImg, skinImg_annotationCopy);
			assert(writeFrameResult==1);
#endif // USE_MT_VIDEO_WRITER			
		}
	}
	
	//annotate regions
	ShowRegions(loopState, currentFrame_annotationCopy, skinImg_annotationCopy, currentFrameWhole_annotationCopy);
}
void CAutismCOACHDoc::detailedAnnotation(bool write2File, string loopState)
{//this calls basicAnnotation, also adds: log msg, and CV outputs: nObsvns, scrubbing/rinsing/faucet occlusion/towel lift/towel interaction 
	basicAnnotation(write2File, loopState);

	//Initialise font
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_DUPLEX ,0.3f, 0.3f, 0, 1); 
	CvScalar regioncolor = CV_RGB(0,255,0);

	//Annotate log msg
	CvPoint bottomLTextPos = cvPoint(5, imgSize.height-10);
	CString bottomLText;
	bottomLText = monitor_n_Prompt.dLog.lastLogMessage;
	cvPutText(currentFrame_annotationCopy, bottomLText, bottomLTextPos, &font, regioncolor);
	cvPutText(currentFrameWhole_annotationCopy, bottomLText, bottomLTextPos, &font, regioncolor);
	cvPutText(skinImg_annotationCopy, bottomLText, bottomLTextPos, &font, regioncolor);
	
	if (loopState.compare("pause")==0)
	{
		//display System Paused:
		CvPoint bottomRTextPos1 = cvPoint(5, imgSize.height-30);
		CString bottomRText1;
		bottomRText1.Format("System paused!");
		cvPutText(currentFrame_annotationCopy, bottomRText1, bottomRTextPos1, &font, regioncolor);
		cvPutText(currentFrameWhole_annotationCopy, bottomRText1, bottomRTextPos1, &font, regioncolor);
		cvPutText(skinImg_annotationCopy, bottomRText1, bottomRTextPos1, &font, regioncolor);
	}
	else if(loopState.compare("prompt")==0)
	{
		if (monitor_n_Prompt.userState != 2) //dx nov 27, 2012: annotate CV obsvns if not in scrubbing step (since scrubbing step doesn't have image processing results by frame (a bundle of frames are processed together)
		{
			//CV obsnvs:
			CvPoint bottomRTextPos1 = cvPoint(5, imgSize.height-30);
			CvPoint bottomRTextPos2 = cvPoint(5, imgSize.height-20);
			CString bottomRText1,bottomRText2;
			bottomRText1.Format("current state: %d",monitor_n_Prompt.nextState_l);
			bottomRText2.Format("waterOn: %d, ntowel: %d, nsoap: %d, nsoapspout: %d, nltap: %d, nrtap: %d, nwater: %d, nsink: %d"
				, monitor_n_Prompt.waterOn, monitor_n_Prompt.ntowel, monitor_n_Prompt.nsoap, monitor_n_Prompt.nsoapspout, monitor_n_Prompt.nltap
				, monitor_n_Prompt.nrtap, monitor_n_Prompt.nwater, monitor_n_Prompt.nsink);	

			cvPutText(currentFrame_annotationCopy, bottomRText1, bottomRTextPos1, &font, regioncolor);
			cvPutText(currentFrame_annotationCopy, bottomRText2, bottomRTextPos2, &font, regioncolor);
			cvPutText(currentFrameWhole_annotationCopy, bottomRText1, bottomRTextPos1, &font, regioncolor);
			cvPutText(currentFrameWhole_annotationCopy, bottomRText2, bottomRTextPos2, &font, regioncolor);	
			cvPutText(skinImg_annotationCopy, bottomRText1, bottomRTextPos1, &font, regioncolor);
			cvPutText(skinImg_annotationCopy, bottomRText2, bottomRTextPos2, &font, regioncolor);

			//hand position, towel position, towel-hand distance (in drying step alone):
			CvScalar posColour = CV_RGB(0,255,0);
			//hand position
			//->display all hand's connected components' positions
			for (int i=0; i<tracker->numcc; i++)
			{
				int x, y;
				tracker->ccspots[i].getCentroid(x, y);
				CvPoint pt = cvPoint(x*2, y*2);
				//cvCircle(currentFrame_annotationCopy, pt, 10, posColour, 1);
				cvCircle(skinImg_annotationCopy, pt, 10, posColour, 1);
				CString ccTxt;
				ccTxt.Format("Hand_%d", i);
				//cvPutText(currentFrame_annotationCopy, ccTxt, pt, &font, regioncolor);
				cvPutText(skinImg_annotationCopy, ccTxt, pt, &font, regioncolor);
			}	
			//towel position

			//->display mtp (mean towel point)
			if (tracker->mtp.x == 1e+10 || tracker->mtp.y == 1e+10)
				int a =1;
			CvPoint towelPos = cvPoint(tracker->mtp.x*2, tracker->mtp.y*2); //dxnote nov 27, 2012: don't forget to times two for the coordinates since the currentFrame_annotationCopy is 2 times the resolution of the image used by the handTracker
			//cvCircle(currentFrame_annotationCopy, towelPos, 10, posColour, 1);
			cvCircle(skinImg_annotationCopy, towelPos, 10, posColour, 1);
			//cvPutText(currentFrame_annotationCopy, "Towel Tracker", towelPos, &font, regioncolor);
			cvPutText(skinImg_annotationCopy, "Towel tracker", towelPos, &font, regioncolor);

			////dxdebug nov 28, 2012
			////->display all towel's connected components' positions
			//for (int i=0; i<tracker->tnumcc; i++)
			//{
			//	int x, y;
			//	tracker->tccspots[i].getCentroid(x, y);
			//	CvPoint pt = cvPoint(x*2, y*2);
			//	//cvCircle(currentFrame_annotationCopy, pt, 10, posColour, 1);
			//	cvCircle(skinImg_annotationCopy, pt, 10, posColour, 1);
			//	CString ccTxt;
			//	if (i == tracker->tcomp)
			//		ccTxt.Format("Towel_component_%d", i);
			//	else
			//		ccTxt.Format("Towel_%d", i);
			//	//cvPutText(currentFrame_annotationCopy, ccTxt, pt, &font, regioncolor);
			//	cvPutText(skinImg_annotationCopy, ccTxt, pt, &font, regioncolor);
			//}	


			////dxdebug nov 27, 2012: display all the specks (all the particles) of towel
			//for (int i=0; i<tracker->tpf->num_particles; i++)
			//{
			//	CvPoint pt = cvPoint(tracker->tpf->ptc[i]->xo*2, tracker->tpf->ptc[i]->yo*2);
			//	cvCircle(skinImg_annotationCopy, pt, 5, CV_RGB(0,255,0), 1);
			//	//CvPoint pt1 = cvPoint(tracker->tpf->ptc_old[i]->xo*2, tracker->tpf->ptc_old[i]->yo*2);
			//	//cvCircle(skinImg_annotationCopy, pt1, 1, CV_RGB(0,255,0), 1);
			//	//CvPoint pt2 = cvPoint(tracker->tpf->ptc_new[i]->xo*2, tracker->tpf->ptc_new[i]->yo*2);
			//	//cvCircle(skinImg_annotationCopy, pt2, 1, CV_RGB(0,0,255), 1);
			//}
		}
		else if (monitor_n_Prompt.userState == 2) //dx oct 18, 2012
		{
			//dx aug 31, 2012: display numDistFeaturesActivated (instead of HistDistances.  Easier to see, more useful too)
			CString s2;
			//s2.Format("numDistFeaturesActivated: %d %d", opticalFlow.numDistFeaturesActivated1, opticalFlow.numDistFeaturesActivated2);
			s2.Format("anglesActivated: %d %d %d, probabilityOfScrubbing: %f", opticalFlow.anglesActivatedS, opticalFlow.anglesActivated, opticalFlow.anglesActivatedL, opticalFlow.probabilityOfScrubbing);
			cvPutText(currentFrame_annotationCopy, s2, cvPoint(5, 30), &font, regioncolor); //dx dec 02, 2012
			cvPutText(currentFrameWhole_annotationCopy, s2, cvPoint(5, 30), &font, regioncolor);
			cvPutText(skinImg_annotationCopy, s2, cvPoint(5, 30), &font, regioncolor);

			CString s;
			s.Format("scrubTimer: %d", monitor_n_Prompt.scrubTimer.getRemainingTotalTime());

			//nov 08, 2012:
			deque<bool> tempResults =  scrubbingDetectorResults;
			s += "  Results: ";
			CString s1;
			while(!tempResults.empty())
			{
				s1.Format("%d ", tempResults.front());
				tempResults.pop_front();
				s += s1;
			}

			//dx aug 30, 2012: display if scrubbing is detected
			//if (monitor_n_Prompt.SCRUBBINGROUTINEHASSTARTED /*||  loopState.compare("tracker") == 0*/)				
			if (monitor_n_Prompt.scrubTimer.activityStarted())
			{ //dxnote nov 06, 2012: even when opticalFlow.probabilityOfScrubbing >= 0.5, i.e. opticalFlow.SCRUBBINGROUTINEHASSTARTED is true, monitor_n_Prompt.SCRUBBINGROUTINEHASSTARTED may still be false ===> when more than one blob is detected

				if(monitor_n_Prompt.RUBBINGHANDSDETECTEDBYOPTICALFLOW)
				{
					s += "  SCRUBBING DETECTED!!";
				}
			}
			cvPutText(currentFrame_annotationCopy, s, cvPoint(180, 10), &font, regioncolor); //dx dec 02, 2012
			cvPutText(currentFrameWhole_annotationCopy, s, cvPoint(180, 10), &font, regioncolor);
			cvPutText(skinImg_annotationCopy, s, cvPoint(180, 10), &font, regioncolor);
		}

		
		if (monitor_n_Prompt.userState == 3 && monitor_n_Prompt.rinseTimer.activityStarted())
		{
			CString s;
			CvPoint pos = cvPoint(180, 10);
			s.Format("rinseTimer: %d", monitor_n_Prompt.rinseTimer.getRemainingTotalTime());
			cvPutText(currentFrame_annotationCopy, s, pos, &font, regioncolor);
			cvPutText(currentFrameWhole_annotationCopy, s, pos, &font, regioncolor);
			cvPutText(skinImg_annotationCopy, s, pos, &font, regioncolor);
		}

		//dx nov 24, 2012
		if (monitor_n_Prompt.userState == 0 || monitor_n_Prompt.userState == 3)
		{//dx nov 24, 2012: if in wetting or rinsing, display faucetDetection
			CString s;
			CvPoint pos = cvPoint(270, 10);
			s.Format("faucetOccluded: %d", monitor_n_Prompt.faucetOccluded);
			cvPutText(currentFrame_annotationCopy, s, pos, &font, regioncolor);
			cvPutText(currentFrameWhole_annotationCopy, s, pos, &font, regioncolor);
			cvPutText(skinImg_annotationCopy, s, pos, &font, regioncolor);
		}
		else if (monitor_n_Prompt.userState == 4)
		{//dx nov 24, 2012: if in drying, display towelLifted
			CString s11;
			CvPoint pos = cvPoint(270, 10);
			s11.Format("towelLifted: %d", monitor_n_Prompt.towelLifted);
			cvPutText(currentFrame_annotationCopy, s11, pos, &font, regioncolor);
			cvPutText(currentFrameWhole_annotationCopy, s11, pos, &font, regioncolor);
			cvPutText(skinImg_annotationCopy, s11, pos, &font, regioncolor);			

			//not dried / dried!
			//past five frames of towel_interactions
			CString s;
			if(monitor_n_Prompt.dryTimer.getTotalTimerDone())
				s.Format("dried!");
			else
				s.Format("not dried");
			deque<bool> tempResults =  towelInteractionFramesQueue;
			s += "  Results: ";
			CString s1;
			while(!tempResults.empty())
			{
				s1.Format("%d ", tempResults.front());
				tempResults.pop_front();
				s += s1;
			}
			if(monitor_n_Prompt.TOWELINTERACTIONDETECTED)
			{
				s += "  TOWEL INTERACTION DETECTED!!";
			}

			cvPutText(currentFrame_annotationCopy, s, cvPoint(200, 20), &font, regioncolor);
			cvPutText(currentFrameWhole_annotationCopy, s, cvPoint(200, 20), &font, regioncolor);
			cvPutText(skinImg_annotationCopy, s, cvPoint(200, 20), &font, regioncolor);
		}
		//}
		//if (loopState.compare("trainWater")!=0) //dx nov 22, 2012: if we aren't in trainWater. since adding tasks would create iplimages without releasing them in trainWater loop (?should i also do this for tracker?)
		//{
		////dxdebug nov 24, 2012:
		//if (!videoWriter.isOpened())
		//	videoWriter.open("C:\\dxFiles\\COACH4ASDresourceFiles\\testLog\\test.avi", CV_FOURCC('I','4','2','0'), fps, Size(w, h), true);
		//if (videoWriter.isOpened())
		//	videoWriter.write(currentFrame_annotationCopy);
		//	//videoWriter << currentFrame_annotationCopy;
		//else
		//	assert(false); //videoWriter not opened

		////dxnote nov 24, 2012: don't use cvShowImage!!... they cause the loop to be rerun more than once.  so even after terminatePrompting is called, the code after this is still executed once through the loop, causing errors since pointers are already nulled and currentFrame_annotationCopy is no longer a valid image
		//////dxdebug nov 24, 2012:
		////cvShowImage("videoAnnotated", currentFrame_annotationCopy);
		////cvWaitKey(1);
	}

	//write video file
	if (write2File)
	{
#ifdef USE_MT_VIDEO_WRITER
		writeFrameTasksQueue_addTask(writerAnnotated, currentFrame_annotationCopy);
		writeFrameTasksQueue_addTask(writerWholeAnnotated, currentFrameWhole_annotationCopy);
		writeFrameTasksQueue_addTask(writerSkinImgAnnotated, skinImg_annotationCopy);
#else
		int writeFrameResult;
		writeFrameResult = cvWriteFrame(writerAnnotated, currentFrame_annotationCopy);
		assert(writeFrameResult==1);
		writeFrameResult = cvWriteFrame(writerWholeAnnotated, currentFrameWhole_annotationCopy);
		assert(writeFrameResult==1);
		writeFrameResult = cvWriteFrame(writerSkinImgAnnotated, skinImg_annotationCopy);
		assert(writeFrameResult==1);
#endif //USE_MT_VIDEO_WRITER
	}
}

//dx aug 30, 2012:
void CAutismCOACHDoc::closePrompts()
{
	//Close any prompting videos that may be playing, stop any sounds
	mciSendString("close myVideo", NULL, 0, 0);
	mciSendString("close myPrimingVideo", NULL, 0, 0);//January 12, 2012
	//dx may 23, 2012: show prompt view on CG side
	mciSendString("close myVideo_CG", NULL, 0, 0);
	mciSendString("close myPrimingVideo_CG", NULL, 0, 0);

	//PlaySound(NULL, 0, 0);
	//mciSendString("close StartOverPrompt", NULL, 0, 0);//dx may 23, 2012: not used
	mciSendString("close GoodJobPrompt", NULL, 0, 0);
	mciSendString("close VerbalPrompt", NULL, 0, 0);
}

void CAutismCOACHDoc::OnTrainScrubbing(void)
{
	//set prompting params to be: 
	//set user state to 2 (done getting soap)
	// must have prompts playing and image processing going during the recording of scrubbings/non-scrubbing to make sure the CPU is occupied to the same degree, thus having same frame rate as during actual prompting session
	// can't allow vision to increment userState:
	//		=> either set countdown timer to very high value 
	//		   or periodically reset userStateChanged flag to false (will work if we use another loop for trainScrubbing separate from TIMER_ID_PROMPT but make sure it has faster loop rate)
	// wanna capture videos while not having any promptings first, and then capture videos while having constant promptings
	// use userStateChanged to initiate priming again to mark the end of recording of scrubbing and start of recording of non-scrubbing

	////Initialize the camera
	//OnFileStartcamera();
	//MTCleanUp();
	////CleanUp();
	//InitializeTrackerandCalibration();
	OnFileStartpromptingsession(); //dxnote sep 17, 2012: seems like just running the lines above creates problems when doing cvReleaseCapture at the end, so just call the whole function for now.

	//monitor_n_Prompt.sessionNumber = monitor_n_Prompt.dLog.GetSessionNumber()+1;
	////actually try adding nonVerbose flag to Logging, then quote everything as if(!nonVerbose)
	monitor_n_Prompt.dLog.nonVerbose = true;
	monitor_n_Prompt.trainScrubbing_justStarted = true;

	//dxnote oct 29, 2012: to make things work:
	if (monitor_n_Prompt.userState == 0 )
	{
		monitor_n_Prompt.userState = 2;
		//monitor_n_Prompt.state_has_changed = false; //don't need this apparently 
	}		
	//monitor_n_Prompt.scrub_timer = 1000; //dx oct 29, 2012: initialize the timer so that in prompt loop's UpdateState the timer is not recognized as finished

	view->SetTimer(TIMER_ID_TRAINSCRUBBING, (int) (1000/(1000/TIMER_ID_CAMERA_PERIOD - 5)), 0); //loop rate = 10 Hz

	prompting = true;
	view->KillTimer(TIMER_ID_GRAB_IMAGES);
	view->SetTimer(TIMER_ID_PROMPT, TIMER_ID_CAMERA_PERIOD, 0);//fps = 15
}

#ifdef USE_MT_VIDEO_WRITER
UINT CAutismCOACHDoc::writeFrameWorker_start(LPVOID p) //the MT's entry point
{
	CAutismCOACHDoc *me = (CAutismCOACHDoc *)p;

	//dxtodo sep 22, 2012: make it thread safe	
	int ntasks = 0;
	//while(!me->writeFrameWorker_die || ntasks!=0) //ntasks==0 is essential so we have a soft shutdown, i.e. the thread won't die immediately after writeFrameWorker_die is issued, but will wait until it does everything on the workQueue first
	while(!me->writeFrameWorker_die) //actually, we should have a hard shutdown since onFileStopPrompting will cvReleaseVideoWriter the writer right away, and we can't go on writting using that writer anymore
	{
		CSingleLock lock(&me->csWriteFrameTasksQueue);

		lock.Lock();
		ntasks = me->writeFrameTasksQueue.size(); 
		lock.Unlock();

		////dxdebug
		//if(ntasks==0)
		//	int a = 0;

		for(int i=0; i<ntasks; i++)
		{
			lock.Lock();
			writeFrameTask task = me->writeFrameTasksQueue.front();
			me->writeFrameTasksQueue.pop_front();
			lock.Unlock();

			CvVideoWriter *writer = task.writer;
			IplImage *imageFrame = task.imageFrame;

			////dxdebug nov 24, 2012:
			//cvShowImage("videoWriter task", imageFrame);
			//cvWaitKey(1);

			assert(writer != NULL);
			assert(imageFrame != NULL);
			cvWriteFrame(writer, imageFrame);
			cvReleaseImage(&imageFrame);
		}
	}

	//dx nov 24, 2012: moved from terminatePrompting to here
	//dx sep 22, 2012: making sure no residue pointers from this session gets remained onto next session since they'd point to wrong places once everything is newed again
	CSingleLock lock(&me->csWriteFrameTasksQueue);
	lock.Lock();
	while(!me->writeFrameTasksQueue.empty())
	{
		writeFrameTask task = me->writeFrameTasksQueue.front();
		me->writeFrameTasksQueue.pop_front();
		IplImage *imageFrame = task.imageFrame;				
		cvReleaseImage(&imageFrame);
	}
	lock.Unlock();

	//dx nov 27, 2012: moved the release of video writers here from TerminatePrompting, because we need to ensure the worker thread isn't using the writers anymore and all frames in me->writeFrameTasksQueue are processed and released
	assert(me->writeFrameTasksQueue.empty());
	if (me->writer!= NULL) { 
		cvReleaseVideoWriter(&me->writer); 
		me->writer = NULL; 
	}
	if (me->writerAnnotated!= NULL) {
		cvReleaseVideoWriter(&me->writerAnnotated);
		me->writerAnnotated = NULL;
	}
	if (me->writerWhole!= NULL) { 
		cvReleaseVideoWriter(&me->writerWhole); 
		me->writerWhole = NULL; 
	}
	if (me->writerWholeAnnotated!= NULL) { 
		cvReleaseVideoWriter(&me->writerWholeAnnotated); 
		me->writerWholeAnnotated = NULL; 
	}
	if (me->writerSkinImg!= NULL) {
		cvReleaseVideoWriter(&me->writerSkinImg);
		me->writerSkinImg = NULL;
	}
	if (me->writerSkinImgAnnotated!= NULL) {
		cvReleaseVideoWriter(&me->writerSkinImgAnnotated);
		me->writerSkinImgAnnotated = NULL;
	}

	//dx sep 27, 2012:
	me->cameraFramesStampsFile.close(); 
	return 0;
}
#endif // USE_MT_VIDEO_WRITER

////dx nov 24, 2012: using the new cv::VideoWriter in opencv-2.4.3
//void CAutismCOACHDoc::writeFrameTasksQueue_addTask(CString videoFileName, IplImage *imageFrame)
//{
//	//IplImage *tempImage = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 3); //can't pass the pointer of the original image to the new thread since we might change the original image in the primary thread and doing criticalSection for those are too burdensome
//	//cvCopyImage(imageFrame, tempImage); 
//	//videoWriterTask_1Frame.imageFrame = tempImage;
//	//videoWriterTask_1Frame.videoFileName = videoFileName;
//
//	//CSingleLock lock(&csvideoWriterTasksQueue);
//	//lock.Lock();
//	//videoWriterTasksQueue.push_back(videoWriterTask_1Frame);
//	//lock.Unlock();
//}


#ifdef USE_MT_VIDEO_WRITER
void CAutismCOACHDoc::writeFrameTasksQueue_addTask(CvVideoWriter *writer, IplImage *imageFrame)
{
	IplImage *tempImage = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 3); //can't pass the pointer of the original image to the new thread, cause we might change the original image in the primary thread, but doing criticalSection for those are too burdensome
	cvCopyImage(imageFrame, tempImage); 
	task.writer = writer;
	task.imageFrame = tempImage;

	CSingleLock lock(&csWriteFrameTasksQueue);
	lock.Lock();
	writeFrameTasksQueue.push_back(task);
	lock.Unlock();
}
#endif //USE_MT_VIDEO_WRITER

void CAutismCOACHDoc::getFaucetOcclusionStatus()
{
	if (faucetOcclusionDetector.first_iteration)
	{
		faucetOcclusionDetector.first_iteration = false;
		faucetOcclusionDetector.predictProbabilityOfBackgroundChange_init(currentFrame, false);
		return;
	}
	else
	{
		monitor_n_Prompt.faucetOccluded = faucetOcclusionDetector.predictProbabilityOfBackgroundChange(currentFrame) > 0.5;
	}
}

void CAutismCOACHDoc::getTowelLiftStatus()
{
	if (towelLiftDetector.first_iteration)
	{
		towelLiftDetector.first_iteration = false;
		towelLiftDetector.predictProbabilityOfBackgroundChange_init(currentFrame, false);
		return;
	}
	else
	{
		monitor_n_Prompt.towelLifted = ! (towelLiftDetector.predictProbabilityOfBackgroundChange(currentFrame) >= 0.5);
	}
}

//dx nov 28, 2012:
double CAutismCOACHDoc::percentageTrueInQueue(deque<bool> queue)
{
	int nTotalFrames = queue.size();
	int nTrueFrames = 0;
	while (!queue.empty())
	{
		nTrueFrames += queue.front();
		queue.pop_front();
	}
	double percentageTrue = 0;
	if (nTotalFrames!=0)
		percentageTrue = (double) nTrueFrames/nTotalFrames;
	return percentageTrue;
}


//dx dec 01, 2012: MT worker thread for opticalFlow processing (thread will be made when userState changes to 2, and will be killed softly when userState changes to be not 2
UINT CAutismCOACHDoc::opticalFlowWorker_start(LPVOID p)//the MT's entry point
{
	CAutismCOACHDoc *pDoc = (CAutismCOACHDoc *)p;

	do //use do/while so that clear queues part can be executed when we are asking the thread to die
	{
		if (pDoc->monitor_n_Prompt.scrubbingLoop_clearQueues)
		{
			pDoc->monitor_n_Prompt.scrubbingLoop_clearQueues = false;
			//clears the imagesBundle
			while (!pDoc->monitor_n_Prompt.imagesBundle.empty())
			{
				cvReleaseImage(&pDoc->monitor_n_Prompt.imagesBundle.front());
				pDoc->monitor_n_Prompt.imagesBundle.pop_front();
			}
			////clears the timeStampBundle, too
			//while (!pDoc->monitor_n_Prompt.timeStampBundle.empty())
			//	pDoc->monitor_n_Prompt.timeStampBundle.pop_front();
			//clears the local copies of the currentFrame and skinImg bundles
			while (!pDoc->imagesBundleForProcessing.empty())
			{
				cvReleaseImage(&pDoc->imagesBundleForProcessing.front());
				pDoc->imagesBundleForProcessing.pop_front();
			}
			while (!pDoc->skinImagesBundleForProcessing.empty())
			{
				cvReleaseImage(&pDoc->skinImagesBundleForProcessing.front());
				pDoc->skinImagesBundleForProcessing.pop_front();	
			}
			continue; //this is needed so initializeScrubbingLoop_GRABBING_MODE can be used when telling the thread to die, if not, we'd need to go through the rest of the loop before exiting the thread
		}

		//GRABBING_MODE
		if (pDoc->monitor_n_Prompt.scrubbingLoop_mode == pDoc->monitor_n_Prompt.GRABBING_MODE)
		{
			if (pDoc->monitor_n_Prompt.bundleCount < SCRUBBING_IMAGE_BUNDLE_SIZE)
			{
				pDoc->monitor_n_Prompt.bundleCount++;

				CSingleLock lock(&pDoc->csVideoFrame);
				lock.Lock();
				pDoc->RetrieveCamImage();//This will invalidate and call ondraw	
				lock.Unlock();
				cvWaitKey(1);
				if (pDoc->cameraFramesStampsFile.is_open()) //dxnote dec 01, 2012: could be already closed by videoWriter worker thread as the thread exits
					pDoc->cameraFramesStampsFile << pDoc->monitor_n_Prompt.timeStamp2CStr() << endl;

				//dx oct 18, 2012: maintains the imagesBundle - a queue of image frames
				IplImage *temp = cvCreateImage(cvSize(pDoc->imageWidth, pDoc->imageHeight), IPL_DEPTH_8U, 3);
				cvCopyImage(pDoc->currentFrame, temp);
				pDoc->monitor_n_Prompt.imagesBundle.push_back(temp);										
				while (pDoc->monitor_n_Prompt.imagesBundle.size() > SCRUBBING_IMAGE_BUNDLE_SIZE)
				{
					assert(pDoc->monitor_n_Prompt.imagesBundle.size() == SCRUBBING_IMAGE_BUNDLE_SIZE+1);
					cvReleaseImage(&pDoc->monitor_n_Prompt.imagesBundle.front());
					pDoc->monitor_n_Prompt.imagesBundle.pop_front();
				}
				////put the timeStamp onto a bundle as well
				//pDoc->monitor_n_Prompt.timeStampBundle.push_back(pDoc->monitor_n_Prompt.timeStampStruct);
			}

			else if (pDoc->monitor_n_Prompt.bundleCount == SCRUBBING_IMAGE_BUNDLE_SIZE) //else if instead of if here to let the while condition (worker thread die flag) and clear queue flag to be checked more often
			{//change to processing mode						
				assert(pDoc->monitor_n_Prompt.imagesBundle.size() == pDoc->monitor_n_Prompt.bundleCount);
				//assert(pDoc->monitor_n_Prompt.imagesBundle.size() == pDoc->monitor_n_Prompt.timeStampBundle.size());
				
				pDoc->monitor_n_Prompt.scrubbingLoop_mode = pDoc->monitor_n_Prompt.PROCESSING_MODE;
				pDoc->opticalFlow.firstcall = true;// clear opticalFlow's frame variables

				pDoc->scrubbingDetectorResults.clear(); //clear the resultsQueue

				//grabs a local copy of imagesBundle for processing
				deque<IplImage *> tempQueue = pDoc->monitor_n_Prompt.imagesBundle; 
				while (!pDoc->imagesBundleForProcessing.empty())
				{//empty imagesBundleForProcessing
					cvReleaseImage(&pDoc->imagesBundleForProcessing.front());
					pDoc->imagesBundleForProcessing.pop_front();
				}
				//copy all frames in tempQueue to imagesBundleForProcessing
				while (!tempQueue.empty())
				{
					IplImage *temp = cvCreateImage(cvSize(pDoc->imageWidth, pDoc->imageHeight), IPL_DEPTH_8U, 3);
					cvCopyImage(tempQueue.front(), temp);					
					pDoc->imagesBundleForProcessing.push_back(temp);
					tempQueue.pop_front(); //dxnote oct 18, 2012: no need to release images from tempQueue, they'll be released in imagesBundle when its size exceeds specified size
				}
				//prepare the queue for storing skinImages processed from frames in imagesBundle
				while (!pDoc->skinImagesBundleForProcessing.empty())
				{//clear the queue of skinImages
					cvReleaseImage(&pDoc->skinImagesBundleForProcessing.front());
					pDoc->skinImagesBundleForProcessing.pop_front();
				}
				while (pDoc->skinImagesBundleForProcessing.size() < pDoc->imagesBundleForProcessing.size())
				{//and create the images
					IplImage *emptyTemp = cvCreateImage(cvSize(pDoc->imageWidth, pDoc->imageHeight), IPL_DEPTH_8U, 3);
					pDoc->skinImagesBundleForProcessing.push_back(emptyTemp);
				}
			}
		}

		//PROCESSING_MODE
		else if (pDoc->monitor_n_Prompt.scrubbingLoop_mode == pDoc->monitor_n_Prompt.PROCESSING_MODE)
		{
			if (!pDoc->imagesBundleForProcessing.empty())
			{
				assert(pDoc->imagesBundleForProcessing.size() == pDoc->skinImagesBundleForProcessing.size());

				//do the image processing
				pDoc->ProcessOneImage(pDoc->imagesBundleForProcessing.front(), pDoc->skinImagesBundleForProcessing.front()); //dxnote oct 19, 2012: i don't need to pass by reference here, since i'm passing pointers to images anyways
				pDoc->CheckHandPositions(2, pDoc->imagesBundleForProcessing.front(), pDoc->skinImagesBundleForProcessing.front());

				//reload the images from the bundle for video recording
				CSingleLock lock(&pDoc->csVideoFrame);
				lock.Lock();
				cvCopyImage(pDoc->skinImagesBundleForProcessing.front(), pDoc->skinImg);
				if (pDoc->videoplaytype == 3)
					cvCopyImage(pDoc->imagesBundleForProcessing.front(), pDoc->currentFrame);
				lock.Unlock();
				//release the images in the bundles that are processed
				cvReleaseImage(&pDoc->imagesBundleForProcessing.front()); //dxnote oct 19, 2012: for debug, i wrote into file using writeFrameTasksQueue_addTask. i can release the images here though, since writeFrameTasksQueue_addTask makes a new copy of the images
				cvReleaseImage(&pDoc->skinImagesBundleForProcessing.front());
				pDoc->imagesBundleForProcessing.pop_front();
				pDoc->skinImagesBundleForProcessing.pop_front();

				////reload the timeStamps to the time the image was saved in the bundle for skinImg videoLog
				//pDoc->monitor_n_Prompt.timeStampStruct = pDoc->monitor_n_Prompt.timeStampBundle.front();
				//pDoc->monitor_n_Prompt.timeStampBundle.pop_front();

				//dxnote oct 19: if I use the lines commented out below using cvShowImage, it seems to make the prompt loop run multiple times during the image processing steps in the while loop. don't know why...
				////dxdebug oct 19, 2012:
				//CHAR _windowName [256];
				//strcpy(_windowName, "test1");
				//cvNamedWindow(_windowName, CV_WINDOW_AUTOSIZE);
				//cvShowImage(_windowName, pDoc->skinImagesBundleForProcessing.front());
				//cvWaitKey(1);


				////dxdebug oct 19, 2012:
				//pDoc->writeFrameTasksQueue_addTask(pDoc->writerSkinImagesBundle, pDoc->skinImagesBundleForProcessing.front());
				////pDoc->writeFrameTasksQueue_addTask(pDoc->writerSkinImagesBundle, pDoc->imagesBundleForProcessing.front());
				////pDoc->writeFrameTasksQueue_addTask(pDoc->writerSkinImagesBundle, pDoc->skinImg);				
			}

			else if (pDoc->imagesBundleForProcessing.empty())
			{
				assert(pDoc->imagesBundleForProcessing.size() == pDoc->skinImagesBundleForProcessing.size());

				//get the results from the processing session	
				//say we detect scrubbing in the bundle of images if the percentage of frames marked as scrubbing exceeds a threshold
				pDoc->monitor_n_Prompt.RUBBINGHANDSDETECTEDBYOPTICALFLOW = (pDoc->percentageTrueInQueue(pDoc->scrubbingDetectorResults) >= PERCENTAGE_OF_FRAMES_SCRUBBING); //dx nov 28, 2012

				////dxdebug oct 20, 2012: the scrub timer is not counting down in a consistent rate when setting pDoc->monitor_n_Prompt.RUBBINGHANDSDETECTEDBYOPTICALFLOW = true
				//pDoc->monitor_n_Prompt.RUBBINGHANDSDETECTEDBYOPTICALFLOW = true;

				//change to grabbing mode and initialize variables
				pDoc->monitor_n_Prompt.initializeScrubbingLoop_GRABBING_MODE();
			}
		}
	}
	while (!pDoc->monitor_n_Prompt.scrubbingLoop_die);

	return 0;
}