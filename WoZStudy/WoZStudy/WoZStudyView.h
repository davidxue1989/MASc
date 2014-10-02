
// WoZStudyView.h : interface of the CWoZStudyView class
//

#pragma once

#include "WoZStudyDoc.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;


class CWoZStudyView : public CView
{

public:
	CWoZStudyDoc* pDoc;
	void ShowImg(Mat &img, RECT *pRect); //draws Mat image to screen
	Mat m_OverheadImage;
	Mat m_OverheadImageAnnotated;
	Mat m_SceneImage;
	void grabFrames();
	void saveVideos();
	void drawTimestamp(Mat &image);
	void displayImages(Mat overheadImage, Mat sceneImage);
	//void log();

protected: // create from serialization only
	CWoZStudyView();
	DECLARE_DYNCREATE(CWoZStudyView)

// Attributes
public:
	CWoZStudyDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CWoZStudyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

#ifndef _DEBUG  // debug version in WoZStudyView.cpp
inline CWoZStudyDoc* CWoZStudyView::GetDocument() const
   { return reinterpret_cast<CWoZStudyDoc*>(m_pDocument); }
#endif

