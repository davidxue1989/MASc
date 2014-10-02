
// WoZStudyDoc.cpp : implementation of the CWoZStudyDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "WoZStudy.h"
#endif

#include "WoZStudyDoc.h"
#include "WoZStudyView.h"

#include "NAO\NAO_prompts.h"
#include "COACH\Video_prompts.h"

#include "COACH/defines.h"

#include <sstream>
#include "WideCharMultiByteConverter.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWoZStudyDoc

IMPLEMENT_DYNCREATE(CWoZStudyDoc, CDocument)

BEGIN_MESSAGE_MAP(CWoZStudyDoc, CDocument)
END_MESSAGE_MAP()


// CWoZStudyDoc construction/destruction

CWoZStudyDoc::CWoZStudyDoc()
//: m_pVideoBuffer(NULL)
//, m_pImageBuffer(NULL)
//, m_depthType(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX)
//, m_colorType(NUI_IMAGE_TYPE_COLOR)
//, m_depthRes(NUI_IMAGE_RESOLUTION_320x240)
//, m_colorRes(NUI_IMAGE_RESOLUTION_640x480)
//, m_bNearMode(TRUE)
//, m_bSeatedSkeletonMode(FALSE)
{
	gazeDetector = NULL;
	CGAttentionDetector = NULL;
	taskStartDetector = NULL;
	taskEndDetector = NULL;
	CGTaskDetector = NULL;
	prompt = NULL;
	m_ExitCameraGrabThread = false;
	m_hCameraGrabThread = NULL;
}

CWoZStudyDoc::~CWoZStudyDoc()
{
}

BOOL CWoZStudyDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;	

	//documentClosed = false;

	//get a pointer to the view object
	POSITION posV = GetFirstViewPosition();
	CWoZStudyView * view = (CWoZStudyView *) GetNextView(posV);
	ASSERT_VALID(view);
	if (!view) {
		AfxMessageBox(_T("Failed to get view!"));
		OnCloseDocument();
	}
	
	//detectors
	if (NULL == gazeDetector)
	{
		gazeDetector = new Detector(view, L"Gaze Detector", L"Detect if child is looking at prompting agent.");
		gazeDetector->Create(Detector::IDD, view);
	}
	if (NULL == CGAttentionDetector)
	{
		CGAttentionDetector = new Detector(view, L"CG Attention Detector", L"Detect if CG has finished directing child's attention.");
		CGAttentionDetector->Create(Detector::IDD, view);
	}
	if (NULL == taskStartDetector)
	{
		taskStartDetector = new Detector(view, L"Task Start Detector", L"Detect if child has started task.");
		taskStartDetector->Create(Detector::IDD, view);
	}
	if (NULL == taskEndDetector)
	{
		taskEndDetector = new Detector(view, L"Task End Detector", L"Detect if child has ended task.");
		taskEndDetector->Create(Detector::IDD, view);
	}
	if (NULL == CGTaskDetector)
	{
		CGTaskDetector = new Detector(view, L"CG Task Detector", L"Detect if CG has finished intervening task.");
		CGTaskDetector->Create(Detector::IDD, view);
	}
	
	taskProgress.Create(TaskProgress::IDD, view);
	promptStateSwitcher.Create(PromptStateSwitcher::IDD, view);

	////frametimestamp log
	//m_LogFrameTimeStamp.openLog(L".\\Log_FrameTimeStamp.txt", L"");
	//m_LogFrameTimeStamp.writeSessionStart(-1);

	//start prompting
#ifdef NAO_PROMPT
		prompt = new NAO_prompts();
#else
		prompt = new Video_prompts(view);
#endif

		promptDecisionMaker.start(gazeDetector, CGAttentionDetector, taskStartDetector, taskEndDetector, CGTaskDetector, &taskProgress, &promptStateSwitcher, PromptDecisionMakerCallingBack, this, prompt);

	////create the image buffers
	//m_pImageBuffer = FTCreateImage();
	//m_pVideoBuffer = FTCreateImage();

	//video captures
	m_VidCapScene.open(0);
	if (!m_VidCapScene.isOpened()) {
		AfxMessageBox(_T("Failed to open video capture 0"));
		OnCloseDocument();
	}
	m_VidCapOverhead.open(1);
	//m_VidCapOverhead.open("..\\vidOverhead_session_1_14-05-12_16-15-17-5398671_2014-05-12-16-15-29-094_CIMG1163.avi");
	if (!m_VidCapOverhead.isOpened()) {
		AfxMessageBox(_T("Failed to open video capture 1"));
		OnCloseDocument();
	}

	std::wstringstream oss;
	std::string str;
	int sessionN = promptDecisionMaker.log.getSessionNumber();
	CString timeStamp = promptDecisionMaker.log.TimeStampToString(false);

	Size sz1(m_VidCapOverhead.get(CV_CAP_PROP_FRAME_WIDTH), m_VidCapOverhead.get(CV_CAP_PROP_FRAME_HEIGHT));
	oss.str(L"");
	oss << L".\\vidOverhead_session_" << sessionN << L"_" << timeStamp.GetString() << L".avi";
	str = WideCharMultiByteConverter::WideChar2MultiByte(oss.str().c_str());
	m_VidWrtOverhead.open(str, CV_FOURCC('M', 'J', 'P', 'G'), 15, sz1);

	Size sz3(m_VidCapOverhead.get(CV_CAP_PROP_FRAME_WIDTH), m_VidCapOverhead.get(CV_CAP_PROP_FRAME_HEIGHT));
	oss.str(L"");
	oss << L".\\vidOverheadAnnotated_session_" << sessionN << L"_" << timeStamp.GetString() << L".avi";
	str = WideCharMultiByteConverter::WideChar2MultiByte(oss.str().c_str());
	m_VidWrtOverheadAnnotated.open(str, CV_FOURCC('M', 'J', 'P', 'G'), 15, sz3);

	Size sz2(m_VidCapScene.get(CV_CAP_PROP_FRAME_WIDTH), m_VidCapScene.get(CV_CAP_PROP_FRAME_HEIGHT));
	oss.str(L"");
	oss << L".\\vidScene_session_" << sessionN << L"_" << timeStamp.GetString() << L".avi";
	str = WideCharMultiByteConverter::WideChar2MultiByte(oss.str().c_str());
	m_VidWrtScene.open(str, CV_FOURCC('M', 'J', 'P', 'G'), 15, sz1);

	if (
		!m_VidWrtOverhead.isOpened()
		|| !m_VidWrtScene.isOpened()
		) {
		AfxMessageBox(_T("Failed to open video writers!"));
		OnCloseDocument();
	}
	
	////find out which codecs are supported
	//VideoWriter vidDepth1(".\\KinectDepth1.avi", CV_FOURCC('P', 'I', 'M', '1'), 15, Size(depthWidth, depthHeight));
	//VideoWriter vidDepth2(".\\KinectDepth2.avi", CV_FOURCC('M', 'J', 'P', 'G'), 15, Size(depthWidth, depthHeight));
	//VideoWriter vidDepth3(".\\KinectDepth3.avi", CV_FOURCC('M', 'P', '4', '2'), 15, Size(depthWidth, depthHeight));
	//VideoWriter vidDepth4(".\\KinectDepth4.avi", CV_FOURCC('D', 'I', 'V', '3'), 15, Size(depthWidth, depthHeight));
	//VideoWriter vidDepth5(".\\KinectDepth5.avi", CV_FOURCC('D', 'I', 'V', 'X'), 15, Size(depthWidth, depthHeight));
	//VideoWriter vidDepth6(".\\KinectDepth6.avi", CV_FOURCC('U', '2', '6', '3'), 15, Size(depthWidth, depthHeight));
	//VideoWriter vidDepth7(".\\KinectDepth7.avi", CV_FOURCC('I', '2', '6', '3'), 15, Size(depthWidth, depthHeight));
	//VideoWriter vidDepth8(".\\KinectDepth8.avi", CV_FOURCC('F', 'L', 'V', '1'), 15, Size(depthWidth, depthHeight));
	//bool a1 = vidDepth1.isOpened();
	//bool a2 = vidDepth2.isOpened();
	//bool a3 = vidDepth3.isOpened();
	//bool a4 = vidDepth4.isOpened();
	//bool a5 = vidDepth5.isOpened();
	//bool a6 = vidDepth6.isOpened();
	//bool a7 = vidDepth7.isOpened();
	//bool a8 = vidDepth8.isOpened();
	
	//start the camera grab loop thread
	m_hCameraGrabThread = CreateThread(NULL, 0, CameraGrabStaticThread, (PVOID)this, 0, 0);

	//start the mfc loop
	view->SetTimer(2, 67, 0); //15fps

	//namedWindow("dx", CV_WINDOW_AUTOSIZE);
	
	return true;
}



void CWoZStudyDoc::cleanUp() {
	//release the captures (this is needed for the camera to be closed)
	m_VidCapOverhead.release();
	m_VidCapScene.release();

	////release the writers (this is needed for the video files to be written and closed) //dxnote: --> not anymore in the newer opencv versions
	//m_VidWrtOverhead.release();
	//m_VidWrtOverheadAnnotated.release();
	//m_VidWrtScene.release();
	
	//stop prompting
	promptDecisionMaker.end();

	if (gazeDetector) {
		gazeDetector->DestroyWindow();
		delete gazeDetector;
		gazeDetector = NULL;
	}
	if (CGAttentionDetector) {
		CGAttentionDetector->DestroyWindow();
		delete CGAttentionDetector;
		CGAttentionDetector = NULL;
	}
	if (taskStartDetector) {
		taskStartDetector->DestroyWindow();
		delete taskStartDetector;
		taskStartDetector = NULL;
	}
	if (taskEndDetector) {
		taskEndDetector->DestroyWindow();
		delete taskEndDetector;
		taskEndDetector = NULL;
	}
	if (CGTaskDetector) {
		CGTaskDetector->DestroyWindow();
		delete CGTaskDetector;
		CGTaskDetector = NULL;
	}

	//kill timer
	POSITION posV = GetFirstViewPosition();
	CWoZStudyView * view = (CWoZStudyView *) GetNextView(posV);
	ASSERT_VALID(view);
	view->KillTimer(2);

	if (prompt)
		delete prompt;

	////end logging
	//if (m_LogFrameTimeStamp.isSessionStarted())
	//	m_LogFrameTimeStamp.writeSessionEnd();

	//exit camera grab loop thread
	m_ExitCameraGrabThread = true;
	WaitForSingleObject(m_hCameraGrabThread, 1000);
}

void CWoZStudyDoc::OnCloseDocument() {
	cleanUp();
	exit(1);
}

void CWoZStudyDoc::PromptDecisionMakerCallingBack(PVOID pVoid, CString msg) {
	CWoZStudyDoc* pDoc = reinterpret_cast<CWoZStudyDoc*>(pVoid);
	if (pDoc)
	{
		if (msg == L"EndSession") {
			pDoc->OnCloseDocument();
		}
	}
	else
		ASSERT(FALSE);
}

DWORD WINAPI CWoZStudyDoc::CameraGrabThread() {
	VideoCapture vidCap(1);
	if (!vidCap.isOpened()) {
		AfxMessageBox(_T("Failed to open video capture 0"));
		OnCloseDocument();
	}

	std::wstringstream oss;
	std::string str;
	int sessionN = promptDecisionMaker.log.getSessionNumber();
	CString timeStamp = promptDecisionMaker.log.TimeStampToString(false);

	Size sz1(vidCap.get(CV_CAP_PROP_FRAME_WIDTH), vidCap.get(CV_CAP_PROP_FRAME_HEIGHT));
	oss.str(L"");
	oss << L".\\vid2ndThread_session_" << sessionN << L"_" << timeStamp.GetString() << L".avi";
	str = WideCharMultiByteConverter::WideChar2MultiByte(oss.str().c_str());
	VideoWriter vidWrt(str, CV_FOURCC('M', 'J', 'P', 'G'), 15, sz1);

	Mat frame;
	DWORD start = GetTickCount();
	DWORD current;
	double catchUp = 0;
	double period = 1000.0 / 15.0; //for some reason, to achieve 15fps, sleep of (1000 / 16) msec is needed;
	while (!m_ExitCameraGrabThread) {
		vidCap >> frame;

		putText(frame, promptDecisionMaker.log.getTimeStamp_msec(), Point(10, 30), FONT_HERSHEY_PLAIN, 2, Scalar(0, 255, 0));
		putText(frame, promptDecisionMaker.log.getLastMessage(), Point(10, 50), FONT_HERSHEY_PLAIN, 2, Scalar(0, 255, 0));

		//sleep / catchup for maintaining the fps
		current = GetTickCount();
		int passed = current - start;
		double left = period - passed;
		if (left >= 0 && left >= int(catchUp)) {
			Sleep(left - int(catchUp));
			catchUp = catchUp - int(catchUp);
		}
		else
			catchUp = catchUp - left;

		start = GetTickCount();
		vidWrt << frame;
	}

	vidCap.release();
	//vidWrt.release();

	m_hCameraGrabThread = NULL;
	return 0;
}


// CWoZStudyDoc serialization

void CWoZStudyDoc::Serialize(CArchive& ar)
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

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CWoZStudyDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CWoZStudyDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CWoZStudyDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CWoZStudyDoc diagnostics

#ifdef _DEBUG
void CWoZStudyDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWoZStudyDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CWoZStudyDoc commands
