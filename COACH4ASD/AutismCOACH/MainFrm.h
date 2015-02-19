// MainFrm.h : interface of the CMainFrame class
//


#pragma once

#include "AutismCOACHDoc.h"
#include "VidPlayerDefines.h"

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	/////////////////////////////Justin - April 12 ///////////////////////////
	
	///*CSliderCtrl m_sliderVideo;
	//CEdit m_editVideoPos; 
	//CEdit m_editLeftHandPos;
	//CEdit m_editRightHandPos;
	//CEdit m_editVideoPlayingSpeed;
	//CStatic staticText1;
	//CStatic staticText2;
	//CStatic staticText3;
	//CStatic staticText4;
	//void SetVideoFrameToSliderPos();
	//CAutismCOACHDoc* pDoc;
	//void SetDoc(CAutismCOACHDoc* d) { pDoc = d; }*/
	/////////////////////////////Justin - April 12 ///////////////////////////

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  // control bar embedded members
	
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CMenu m_wndMenu;

	//CStatusBar  m_wndStatusBar;
	//CToolBar    m_wndStandardToolBar;
	//CToolBar	m_wndVideoToolBar;
	//CToolBar	m_wndHandPosToolBar;
	//CMyToolBar	m_wndVideoPosToolBar;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	void OnViewDeveloperView(void);
	void InitCtrls(bool isDeveloper);
	void OnViewCareGiverView(void);
	void OnFileStartpromptingsession(void);
	void OnFileStoppromptingsession(void);
	void OnFilePauseSystem(void);
	
	//dx may 12, 2012
	int systemState;
	void updateToolbar(void);

	//dx oct 28, 2012:
	LONG toolBarHeight;
	void updateToolBarHeight();
	LONG getToolBarHeight();
	void disableToolBarButtons(bool disable);
};