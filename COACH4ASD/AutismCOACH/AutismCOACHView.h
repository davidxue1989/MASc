// AutismCOACHView.h : interface of the CAutismCOACHView class
//

#pragma once
#include <iostream>
#include <fstream>
#include "MainFrm.h"
#include <time.h>
#include <deque>
#include "CvvImage.h"

using namespace std;

enum
{
	//TIMER_ID__PROCESS_ONE_FRAME = 1,
	//TIMER_ID__INITIALIZE_POINTERS = 2,
	TIMER_ID_GRAB_IMAGES = 2,
	TIMER_ID_TRAINWATERDETECTOR = 3,//Justin - October 20, 2011
	TIMER_ID_PROMPT = 4,//Justin - April 16
	TIMER_ID_SYSTEM_PAUSE = 6,
	TIMER_ID_STARTTRACKER = 7,
	//TIMER_ID_STARTTESTING = 8, //dx may 01, 2012: commented out due to unused
	TIMER_ID_TRAINSCRUBBING = 8,
};

#define TIMER_ID_CAMERA_PERIOD 67 //15 fps
//#define TIMER_ID_CAMERA_PERIOD 100 //dxtodo sep 21, 2012: should be set to the slowest frame rate tolerable by the vision alg (may be varied in different stages (scrubbing detection may require diff fps than say rinse detection)
//dxnote sep 21, 2012: RetrieveCamImage with PS3 CLEye takes 0, 15, 16 or 31 millisec (if fps = 30) or 0, 15, 16 (if fps = 60), 
// while with webcam it takes 0 msec for mfc loop rate = 15 fps (67 (+11?) msec), or 0, 15, 16, 31, 32, 63 ms for loop rate = 20 fps (50ms)
#define TIMER_TRACKER_PERIOD 67


class CAutismCOACHView : public CView
{
protected: // create from serialization only
	CAutismCOACHView();
	DECLARE_DYNCREATE(CAutismCOACHView)

// Attributes
public:
	CAutismCOACHDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CAutismCOACHView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

//Everything above added to the project by default
/////////////////////////////////////////////////////////////////////////
//Everything below added to the project by Justin, except after **
//protected:
public:
	CAutismCOACHDoc* pDoc;//in order to access the functions of the doc class from the view class
	CvvImage cvvImage; // temp data for showing current image on the CDC (Undocumented class)
		
	IplImage  *showFrame;
	
	//dx sep 17, 2012:
	CTime tic, toc;
	CTimeSpan tspan;

	//struct FRAME {
	//	deque<float> queueDistances;
	//	deque<float> queueAngles;
	//	deque<int> nAngleBinsActivated;
	//};
	//deque<FRAME> queueScrubbingFrames, queueNonScrubbingFrames;
	deque<COpticalFlow::FRAME> queueScrubbingFrames, queueNonScrubbingFrames; //dx oct 29, 2012: COpticalFlow::FRAME is needed for use of COpticalFlow::countNAngleBinsActivated()
	deque<COpticalFlow::FRAME> getScrubTrainingData(const char *fileName);
	//dx oct 29, 2012: moved countNAngleBinsActivated to COpticalFlow
	//void countNAngleBinsActivated(deque<FRAME> &queueFrames, int nDistanceBins, int nAngleBins, deque<int> binDistanceBoundaries);

	//dx sep 21, 2012:
	DWORD loopStartTime;


public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	//Methods
	void WriteNumBlobs2StatusBar();
	//void GetCamImage();

	//dx may 07, 2012: updating the nobservation variables in pDoc->monitor_n_Prompt
	void WriteNumBlobs2LogVars();

//	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	
	//dxtry dec 01, 2012
	//friend void CAutismCOACHDoc::setTimeStamp();
};

#ifndef _DEBUG  // debug version in AutismCOACHView.cpp
inline CAutismCOACHDoc* CAutismCOACHView::GetDocument() const
   { return reinterpret_cast<CAutismCOACHDoc*>(m_pDocument); }
#endif