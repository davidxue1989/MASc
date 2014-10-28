
// WoZStudy.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "WoZStudy.h"
#include "MainFrm.h"

#include "WoZStudyDoc.h"
#include "WoZStudyView.h"
#include "COACH\defines.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWoZStudyApp

BEGIN_MESSAGE_MAP(CWoZStudyApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CWoZStudyApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)


	ON_COMMAND(ID_COACH_TASK0, &OnBnClickedButtonIntro)
	ON_COMMAND(ID_COACH_TASK1, &OnBnClickedButtonTurnonwater)
	ON_COMMAND(ID_COACH_TASK2, &OnBnClickedButtonWethands)
	ON_COMMAND(ID_COACH_TASK3, &OnBnClickedButtonSqueezeoutsoap)
	ON_COMMAND(ID_COACH_TASK4, &OnBnClickedButtonScrubhands)
	ON_COMMAND(ID_COACH_TASK5, &OnBnClickedButtonRinsehands)
	ON_COMMAND(ID_COACH_TASK6, &OnBnClickedButtonTurnoffwater)
	ON_COMMAND(ID_COACH_TASK7, &OnBnClickedButtonDryhands)
	ON_COMMAND(ID_COACH_TASK8, &OnBnClickedButtonAlldone)

	ON_COMMAND(ID_COACH_ACTION1, &OnBnClickedButtonFirstag)
	ON_COMMAND(ID_COACH_ACTION2, &OnBnClickedButtonReward)
	ON_COMMAND(ID_COACH_ACTION3, &OnBnClickedButtonLetuscontinue)

	ON_COMMAND(ID_COACH_TASK_DELETE, &OnBnClickedButtonDelete)
	ON_COMMAND(ID_COACH_STOP, &OnBnClickedButtonStop)

END_MESSAGE_MAP()


// CWoZStudyApp construction

CWoZStudyApp::CWoZStudyApp()
{
	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("WoZStudy.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CWoZStudyApp object

CWoZStudyApp theApp;


// CWoZStudyApp initialization

BOOL CWoZStudyApp::InitInstance()
{
	CWinApp::InitInstance();

	m_haccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));


	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)


	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CWoZStudyDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CWoZStudyView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	//dxchanged
	m_pMainWnd->ModifyStyle(WS_CAPTION, 0); //get rid of the title bar
	m_pMainWnd->SetWindowPos(NULL, 0, 0, FIRSTMONITORWIDTH / 2, FIRSTMONITORHEIGHT, NULL); //make the width half of screen
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand


	//m_haccel = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_COACH));
	

	return TRUE;
}

BOOL CWoZStudyApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	if (m_haccel)
	{
		if (::TranslateAccelerator(m_pMainWnd->m_hWnd, m_haccel, lpMsg))
			return(TRUE);
	}

	return CWinApp::ProcessMessageFilter(code, lpMsg);
}

CDocument * CWoZStudyApp::getDoc() {
	CDocument* pDoc = NULL;

	CWnd* pWndMain = AfxGetMainWnd();
	ASSERT(pWndMain);
	ASSERT(pWndMain->IsKindOf(RUNTIME_CLASS(CFrameWnd)) &&
		!pWndMain->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd))); // Not an SDI app.

	pDoc = ((CFrameWnd*) pWndMain)->GetActiveDocument();
	return pDoc;
}

void CWoZStudyApp::OnBnClickedButtonIntro()
{
	//((CWoZStudyDoc *)getDoc())->promptDecisionMaker.taskProgress->OnBnClickedButtonIntro();
	((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptQueue.push_back(PromptDecisionMaker::PROMPTS::Intro1);
	((CWoZStudyDoc *) getDoc())->prompt_queue_dlg.updateDisplay(((CWoZStudyDoc *) getDoc())->promptDecisionMaker.getPromptQueueString());
}


void CWoZStudyApp::OnBnClickedButtonTurnonwater()
{
	//((CWoZStudyDoc *) getDoc())->promptDecisionMaker.taskProgress->OnBnClickedButtonTurnonwater();
	((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptQueue.push_back(PromptDecisionMaker::PROMPTS::TurnOnWater1);
	((CWoZStudyDoc *) getDoc())->prompt_queue_dlg.updateDisplay(((CWoZStudyDoc *) getDoc())->promptDecisionMaker.getPromptQueueString());
}


void CWoZStudyApp::OnBnClickedButtonWethands()
{
	//((CWoZStudyDoc *) getDoc())->promptDecisionMaker.taskProgress->OnBnClickedButtonWethands();
	((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptQueue.push_back(PromptDecisionMaker::PROMPTS::WetYourHands1);
	((CWoZStudyDoc *) getDoc())->prompt_queue_dlg.updateDisplay(((CWoZStudyDoc *) getDoc())->promptDecisionMaker.getPromptQueueString());
}


void CWoZStudyApp::OnBnClickedButtonSqueezeoutsoap()
{
	//((CWoZStudyDoc *) getDoc())->promptDecisionMaker.taskProgress->OnBnClickedButtonSqueezeoutsoap();
	((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptQueue.push_back(PromptDecisionMaker::PROMPTS::GetSomeSoap1);
	((CWoZStudyDoc *) getDoc())->prompt_queue_dlg.updateDisplay(((CWoZStudyDoc *) getDoc())->promptDecisionMaker.getPromptQueueString());
}


void CWoZStudyApp::OnBnClickedButtonScrubhands()
{
	//((CWoZStudyDoc *) getDoc())->promptDecisionMaker.taskProgress->OnBnClickedButtonScrubhands();
	((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptQueue.push_back(PromptDecisionMaker::PROMPTS::ScrubYourHands1);
	((CWoZStudyDoc *) getDoc())->prompt_queue_dlg.updateDisplay(((CWoZStudyDoc *) getDoc())->promptDecisionMaker.getPromptQueueString());
}


void CWoZStudyApp::OnBnClickedButtonRinsehands()
{
	//((CWoZStudyDoc *) getDoc())->promptDecisionMaker.taskProgress->OnBnClickedButtonRinsehands();
	((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptQueue.push_back(PromptDecisionMaker::PROMPTS::RinseYourHands1);
	((CWoZStudyDoc *) getDoc())->prompt_queue_dlg.updateDisplay(((CWoZStudyDoc *) getDoc())->promptDecisionMaker.getPromptQueueString());
}


void CWoZStudyApp::OnBnClickedButtonTurnoffwater()
{
	//((CWoZStudyDoc *) getDoc())->promptDecisionMaker.taskProgress->OnBnClickedButtonTurnoffwater();
	((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptQueue.push_back(PromptDecisionMaker::PROMPTS::TurnOffWater1);
	((CWoZStudyDoc *) getDoc())->prompt_queue_dlg.updateDisplay(((CWoZStudyDoc *) getDoc())->promptDecisionMaker.getPromptQueueString());
}


void CWoZStudyApp::OnBnClickedButtonDryhands()
{
	//((CWoZStudyDoc *) getDoc())->promptDecisionMaker.taskProgress->OnBnClickedButtonDryhands();
	((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptQueue.push_back(PromptDecisionMaker::PROMPTS::DryYourHands1);
	((CWoZStudyDoc *) getDoc())->prompt_queue_dlg.updateDisplay(((CWoZStudyDoc *) getDoc())->promptDecisionMaker.getPromptQueueString());
}


void CWoZStudyApp::OnBnClickedButtonAlldone()
{
	//((CWoZStudyDoc *) getDoc())->promptDecisionMaker.taskProgress->OnBnClickedButtonAlldone();
	((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptQueue.push_back(PromptDecisionMaker::PROMPTS::AllDone1);
	((CWoZStudyDoc *) getDoc())->prompt_queue_dlg.updateDisplay(((CWoZStudyDoc *) getDoc())->promptDecisionMaker.getPromptQueueString());
}


void CWoZStudyApp::OnBnClickedButtonFirstag()
{
	//((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptStateSwitcher->OnBnClickedButtonFirstag();
	((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptQueue.push_back(PromptDecisionMaker::PROMPTS::AttentionGrabber1);
	((CWoZStudyDoc *) getDoc())->prompt_queue_dlg.updateDisplay(((CWoZStudyDoc *) getDoc())->promptDecisionMaker.getPromptQueueString());
}


void CWoZStudyApp::OnBnClickedButtonReward()
{
	//((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptStateSwitcher->OnBnClickedButtonReward();
	((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptQueue.push_back(PromptDecisionMaker::PROMPTS::Reward1);
	((CWoZStudyDoc *) getDoc())->prompt_queue_dlg.updateDisplay(((CWoZStudyDoc *) getDoc())->promptDecisionMaker.getPromptQueueString());
}


void CWoZStudyApp::OnBnClickedButtonLetuscontinue()
{
	//((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptStateSwitcher->OnBnClickedButtonLetuscontinue();
	((CWoZStudyDoc *) getDoc())->promptDecisionMaker.promptQueue.push_back(PromptDecisionMaker::PROMPTS::LetUsContinue1);
	((CWoZStudyDoc *) getDoc())->prompt_queue_dlg.updateDisplay(((CWoZStudyDoc *) getDoc())->promptDecisionMaker.getPromptQueueString());
}

void CWoZStudyApp::OnBnClickedButtonDelete() {
	CWoZStudyDoc *pDoc = (CWoZStudyDoc *) getDoc();
	if (!pDoc->promptDecisionMaker.promptQueue.empty()) {
		pDoc->promptDecisionMaker.promptQueue.pop_back();
		pDoc->prompt_queue_dlg.updateDisplay(pDoc->promptDecisionMaker.getPromptQueueString());
	}
	else if (pDoc->promptDecisionMaker.pPrompts->isPrompting()) {
		pDoc->promptDecisionMaker.pPrompts->takeRest();
		pDoc->promptDecisionMaker.rested = true;
	}
}

void CWoZStudyApp::OnBnClickedButtonStop() {

}

//BOOL CWoZStudyApp::PreTranslateMessage(MSG* pMsg) {
//	if (m_haccel) {
//		if (::TranslateAccelerator(*AfxGetMainWnd(), m_haccel, pMsg)) {
//			return(TRUE);
//		}
//	}
//	return  __super::PreTranslateMessage(pMsg);
//
//}


// CWoZStudyApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CWoZStudyApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CWoZStudyApp message handlers



