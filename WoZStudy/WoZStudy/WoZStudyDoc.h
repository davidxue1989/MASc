
// WoZStudyDoc.h : interface of the CWoZStudyDoc class
//


#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

#include "Logging.h"
#include "COACH/PromptDecisionMaker.h"

#include "Detector.h"
#include "TaskProgress.h"
#include "PromptStateSwitcher.h"
#include "COACH\Prompts.h"

class CWoZStudyDoc : public CDocument
{
protected: // create from serialization only
	CWoZStudyDoc();
	DECLARE_DYNCREATE(CWoZStudyDoc)


public:
	virtual void OnCloseDocument();
	void cleanUp();
	HWND m_hWnd;
	//BOOL                        m_bNearMode;
	//BOOL                        m_bSeatedSkeletonMode;
	VideoCapture				m_VidCapOverhead;
	VideoCapture				m_VidCapScene;
	VideoWriter					m_VidWrtOverhead;
	VideoWriter					m_VidWrtOverheadAnnotated;
	VideoWriter					m_VidWrtScene;

	//Logging						m_LogFrameTimeStamp;
	PromptDecisionMaker			promptDecisionMaker;
	
	Detector					*gazeDetector;
	Detector					*CGAttentionDetector;
	Detector					*taskStartDetector;
	Detector					*taskEndDetector;
	Detector					*CGTaskDetector;
	TaskProgress taskProgress;
	PromptStateSwitcher promptStateSwitcher;

	static void PromptDecisionMakerCallingBack(PVOID pVoid, CString msg);

	Prompts *prompt;


	bool m_ExitCameraGrabThread;
	HANDLE m_hCameraGrabThread;
	static DWORD WINAPI CameraGrabStaticThread(PVOID lpParam)
	{
		CWoZStudyDoc* context = static_cast<CWoZStudyDoc*>(lpParam);
		if (context)
		{
			return context->CameraGrabThread();
		}
		return 0;
	};
	DWORD WINAPI CameraGrabThread();


// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CWoZStudyDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
