// MainFrm.cpp : implementation of the CMainFrame class
//This is a test!

#include "stdafx.h"
#include "AutismCOACH.h"
#include "AutismCOACHDoc.h"


#include "MainFrm.h"
#include "DeveloperPWDlg.h"

#define DEVPW "123"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	
	ON_COMMAND(ID_VIEW_DEVELOPERVIEW, &CMainFrame::OnViewDeveloperView)
	ON_COMMAND(ID_VIEW_CAREGIVERVIEW, &CMainFrame::OnViewCareGiverView)


END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	//handPosDlg.DestroyWindow();
	//handPosDlg.Create(IDD_HANDPOS_DIALOG, NULL);
	//handPosDlg.ShowWindow(SW_SHOW);
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	//dx nov 05, 2012: commented out, don't display the statusBar
	//if (!m_wndStatusBar.Create(this) ||
	//	!m_wndStatusBar.SetIndicators(indicators,
	//	  sizeof(indicators)/sizeof(UINT)))
	//{
	//	TRACE0("Failed to create status bar\n");
	//	return -1;      // fail to create
	//}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	//EnableDocking(CBRS_ALIGN_ANY);
	//DockControlBar(&m_wndToolBar); //dx may 15, 2012: this was commented out by Matthew because it will hinder the disabling of tool bar buttons
	
	//dx may 15, 2012:
	//ModifyStyle(WS_MAXIMIZEBOX,0);
	//ModifyStyle(WS_MINIMIZEBOX,0);
	ModifyStyle(WS_CAPTION,0);
	ShowWindow(SW_SHOWMAXIMIZED);

	InitCtrls(false);

//// place a slider bar on the video toolbar
//	m_sliderVideo.DestroyWindow();
//	RECT rc;
//	//m_wndVideoPosToolBar.GetItemRect(0, &rc);
//	rc.left = VIDEO_SLIDER_BAR_LEFT;
//	rc.right = VIDEO_SLIDER_BAR_RIGHT;
//	rc.top = 0;
//	rc.bottom = 20;
//	VERIFY (m_sliderVideo.Create(WS_CHILD | WS_VISIBLE, rc, &m_wndVideoPosToolBar, 1));
//	m_sliderVideo.ClearTics(true);
//	SetFrameCount(1);
//
//
//
//	// place a static text that says 'frame'
//	rc.left = FRAME_COUNT_STATIC_LEFT;
//	rc.right = FRAME_COUNT_STATIC_RIGHT;
//	rc.top = 2;
//	rc.bottom = 20;
//	VERIFY (staticText1.Create("frame", WS_CHILD | WS_VISIBLE, rc, &m_wndVideoPosToolBar, 1));
//	staticText1.EnableWindow(false);
//	//staticText1.SetWindowTextA("frame");
//
//	// place an edit box that will display the current frame #
//	m_editVideoPos.DestroyWindow();
//	rc.left = FRAME_COUNT_EDITBOX_LEFT;
//	rc.right = FRAME_COUNT_EDITBOX_RIGHT;
//	rc.top = 2;
//	rc.bottom = 20;
//	VERIFY (m_editVideoPos.Create(WS_CHILD | WS_VISIBLE, rc, &m_wndVideoPosToolBar, 1));
//	m_editVideoPos.EnableWindow(false);
//	m_editVideoPos.SetWindowTextA("1");
//
//	
//	// place "left hand" and "right hand" static texts on the standard toolbar + two edit boxes to show the hand positions
//	//RECT rc;
//	// first add a separator at the end 
//	//m_wndStandardToolBar.InsertButton(8, ID_SEPARATOR); // 8 = # buttons so far
//		
//	// left hand static text
//	rc.left = 205;
//	rc.right = 220;
//	rc.top = 2;
//	rc.bottom = 17;
//	VERIFY (staticText2.Create("lh:", WS_CHILD | WS_VISIBLE, rc, &m_wndStandardToolBar, 1));
//	staticText2.EnableWindow(false);
//
//	
//	// left hand edit box
//	m_editLeftHandPos.DestroyWindow();
//	rc.left = 223;
//	rc.right = 259;
//	rc.top = 2;
//	rc.bottom = 17;
//	VERIFY (m_editLeftHandPos.Create(WS_CHILD | WS_VISIBLE, rc, &m_wndStandardToolBar, 1));
//	m_editLeftHandPos.EnableWindow(false);
//	m_editLeftHandPos.SetWindowTextA("");
//
//	// right hand static text
//	rc.left = 265;
//	rc.right = 280;
//	rc.top = 2;
//	rc.bottom = 17;
//	VERIFY (staticText3.Create("rh:", WS_CHILD | WS_VISIBLE, rc, &m_wndStandardToolBar, 1));
//	staticText3.EnableWindow(false);
//
//	
//	// right hand edit box
//	m_editRightHandPos.DestroyWindow();
//	rc.left = 283;
//	rc.right = 319;
//	rc.top = 2;
//	rc.bottom = 17;
//	VERIFY (m_editRightHandPos.Create(WS_CHILD | WS_VISIBLE, rc, &m_wndStandardToolBar, 1));
//	m_editRightHandPos.EnableWindow(false);
//	m_editRightHandPos.SetWindowTextA("");
//
//	// video playing speed static text
//	rc.left = 225;
//	rc.right = 260;
//	rc.top = 2;
//	rc.bottom = 17;
//	VERIFY (staticText4.Create("skip:", WS_CHILD | WS_VISIBLE, rc, &m_wndVideoToolBar, 1));
//	staticText4.EnableWindow(false);
//
//	// video playing speed edit box
//	m_editVideoPlayingSpeed.DestroyWindow();
//	rc.left = 260;
//	rc.right = 270;
//	rc.top = 2;
//	rc.bottom = 17;
//	VERIFY (m_editVideoPlayingSpeed.Create(WS_CHILD | WS_VISIBLE, rc, &m_wndVideoToolBar, 1));
//	m_editVideoPlayingSpeed.EnableWindow(false);
//	m_editVideoPlayingSpeed.SetWindowTextA("0");
//
//	pDoc = NULL;
//
//	// Prevent Mainframe Window from Being ReSized 
//	// source: http://support.microsoft.com/default.aspx/kb/133256 )
//    DWORD style = ::GetWindowLong(this->m_hWnd, GWL_STYLE);
//	// Remove the thick frame style and the Minimize, Maximize buttons
//    style &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
//    ::SetWindowLong(this->m_hWnd, GWL_STYLE, style);
	

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	
	//dx may 15, 2012: want to make the window with no system menu (close, min, max) and maximized
	//cs.style = WS_ACTIVECAPTION|WS_OVERLAPPED|WS_MAXIMIZE|WS_MAXIMIZEBOX|WS_SYSMENU;
	//cs.style = WS_THICKFRAME|WS_MINIMIZE;
	cs.style = 0;

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::InitCtrls(bool isDeveloper)
{

	if(systemState == ID_FILE_STARTPROMPTINGSESSION || systemState == ID_FILE_PAUSESYSTEM) //dxnote sep 18, 2012: this wont' ever be entered; the things are done in updateToolbar
	{
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STARTPROMPTINGSESSION,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_PAUSESYSTEM,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STOPPROMPTINGSESSION,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_ENTERUSERSETTINGS,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_VIEW_PROMPTVIEW,0);
	}
	else
	{
		//m_wndToolBar.SetButtonInfo(1, ID_SEPARATOR, TBBS_SEPARATOR, 12);
		//m_wndToolBar.SetButtonInfo(4, ID_SEPARATOR, TBBS_SEPARATOR, 12); //dx sep 18, 2012: can't get this separator thing to work... :(

		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STARTPROMPTINGSESSION,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_PAUSESYSTEM,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STOPPROMPTINGSESSION,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_ENTERUSERSETTINGS,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_VIEW_PROMPTVIEW,1);
	}

	if (isDeveloper) 
	{
		/*Button*/
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_CALIBRATE,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_SWITCHIMAGE,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STARTTRACKER,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STARTDATACOLLECTION,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_SKIPSCRUBRINSEDETECTION,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_VIEW_DEVELOPERVIEW,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_VIEW_CAREGIVERVIEW,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_TRAINSCRUBBING,0);
		/*Menu*/
		m_wndMenu.DestroyMenu(); //dx nov 11, 2012: need to destroy menu first, or else assertion error
		m_wndMenu.LoadMenuA(IDR_MAINFRAME_MENU);
		SetMenu(&m_wndMenu);
	}else
	{
		/*Button*/

		//m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STARTPROMPTINGSESSION,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_CALIBRATE,1);
		//m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_ENTERUSERSETTINGS,1);
		//m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_PAUSESYSTEM,1);
		//m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STOPPROMPTINGSESSION,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_SWITCHIMAGE,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STARTTRACKER,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STARTDATACOLLECTION,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_SKIPSCRUBRINSEDETECTION,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_VIEW_DEVELOPERVIEW,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_VIEW_CAREGIVERVIEW,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_TRAINSCRUBBING,1);

		/*Menu*/
		//m_wndMenu.LoadMenuA(IDR_MAINFRAME_MENU_USER);
		SetMenu(0); //dx may 12, 2012: don't display a menu for user
	}  

	//dx jul 07, 2012: updating the toolbar after switching between CG and developer view during a trial
	updateToolbar();

}

void CMainFrame::OnViewDeveloperView(void)
{
	//dialog for developer password
	CDeveloperPWDlg pwDlg;
	if (pwDlg.DoModal() == IDOK)
	{
		if (pwDlg.password==DEVPW)
		{
			InitCtrls(true);
		}else
		{
			AfxMessageBox("Password Incorrect !!!");
		}
	}
}

void CMainFrame::OnViewCareGiverView(void)
{
	InitCtrls(false);
}




void CMainFrame::OnFileStartpromptingsession(void)
{
	systemState = ID_FILE_STARTPROMPTINGSESSION;
	updateToolbar();
}

void CMainFrame::OnFileStoppromptingsession(void)
{	
	systemState = ID_FILE_STOPPROMPTINGSESSION;	
	updateToolbar();
}


void CMainFrame::OnFilePauseSystem(void)
{
	systemState = ID_FILE_PAUSESYSTEM;
	//updateToolbar();  	//same as OnFileStartpromptingsession, so don`t need to do anything since we can only get here right after starting prompt session
	////toolbar
	//m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STARTPROMPTINGSESSION,1);
	//m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_PAUSESYSTEM,0);
	//m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STOPPROMPTINGSESSION,0);
	//m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_ENTERUSERSETTINGS,1);

	////menu items
	//m_wndMenu.EnableMenuItem(ID_FILE_STARTPROMPTINGSESSION,MF_GRAYED);
	//m_wndMenu.EnableMenuItem(ID_FILE_PAUSESYSTEM,MF_ENABLED);
	//m_wndMenu.EnableMenuItem(ID_FILE_STOPPROMPTINGSESSION,MF_ENABLED);
	//m_wndMenu.EnableMenuItem(ID_FILE_ENTERUSERSETTINGS,MF_GRAYED);
}


void CMainFrame::updateToolbar(void)
{
	if (systemState == ID_FILE_STARTPROMPTINGSESSION || systemState == ID_FILE_PAUSESYSTEM)
	{
		//toolbar
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STARTPROMPTINGSESSION,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_PAUSESYSTEM,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STOPPROMPTINGSESSION,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_ENTERUSERSETTINGS,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_VIEW_PROMPTVIEW,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_VIEW_DEVELOPERVIEW,1);
		//dx may 12, 2012: menu items don't get disabled here. probably because MFC updates UI and restores the greying out effects
		// will just hide menu bar entirely
		//menu items
		//m_wndMenu.EnableMenuItem(ID_FILE_STARTPROMPTINGSESSION,MF_GRAYED| MF_BYCOMMAND);
		//m_wndMenu.EnableMenuItem(ID_FILE_PAUSESYSTEM,MF_GRAYED| MF_BYCOMMAND);
		//m_wndMenu.EnableMenuItem(ID_FILE_STOPPROMPTINGSESSION,MF_GRAYED| MF_BYCOMMAND);
		//m_wndMenu.EnableMenuItem(ID_FILE_ENTERUSERSETTINGS,MF_GRAYED| MF_BYCOMMAND);
		m_wndToolBar.UpdateWindow(); //dx oct 28, 2012: to make sure the tool bar is redrawn right away
	}

	else if (systemState == ID_FILE_STOPPROMPTINGSESSION)
	{
		//toolbar
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STARTPROMPTINGSESSION,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_PAUSESYSTEM,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_STOPPROMPTINGSESSION,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_FILE_ENTERUSERSETTINGS,0);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_VIEW_PROMPTVIEW,1);
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_VIEW_DEVELOPERVIEW,0);
		m_wndToolBar.UpdateWindow(); //dx oct 28, 2012: to make sure the tool bar is redrawn right away
	}	
}

//dx oct 28, 2012:
void CMainFrame::updateToolBarHeight()
{
	RECT rect_temp;
	m_wndToolBar.GetItemRect(1,&rect_temp);
	toolBarHeight = rect_temp.bottom - rect_temp.top;
	if (toolBarHeight == 0) //dx dec 13, 2012: quick fix for incorrect video size displayed during tracking loop (for reason that more toolBar buttons are displayed during tracking loop, as opposed to the prompting loop)
	{
		m_wndToolBar.GetItemRect(0,&rect_temp);
		toolBarHeight = rect_temp.bottom - rect_temp.top;
	}
}
LONG CMainFrame::getToolBarHeight()
{
	return toolBarHeight;
}

void CMainFrame::disableToolBarButtons(bool disable)
{
	UINT itemID;
	for (int i=0; i<m_wndToolBar.GetCount(); i++)
	{
		m_wndToolBar.GetToolBarCtrl().EnableButton(m_wndToolBar.GetItemID(i), !disable);
	}	
	m_wndToolBar.UpdateWindow();
}