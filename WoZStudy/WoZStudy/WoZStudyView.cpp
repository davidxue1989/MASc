
// WoZStudyView.cpp : implementation of the CWoZStudyView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "WoZStudy.h"
#endif

#include "WoZStudyView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWoZStudyView

IMPLEMENT_DYNCREATE(CWoZStudyView, CView)

BEGIN_MESSAGE_MAP(CWoZStudyView, CView)
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CWoZStudyView construction/destruction

CWoZStudyView::CWoZStudyView()
{
	// TODO: add construction code here
	pDoc = NULL;
}

CWoZStudyView::~CWoZStudyView()
{
}

BOOL CWoZStudyView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CView::PreCreateWindow(cs);
}

// CWoZStudyView drawing

void CWoZStudyView::OnDraw(CDC* /*pDC*/)
{
	if (!pDoc) {
		pDoc = GetDocument();
		ASSERT_VALID(pDoc);
	}

	// TODO: add draw code for native data here
}


// CWoZStudyView diagnostics

#ifdef _DEBUG
void CWoZStudyView::AssertValid() const
{
	CView::AssertValid();
}

void CWoZStudyView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWoZStudyDoc* CWoZStudyView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWoZStudyDoc)));
	return (CWoZStudyDoc*)m_pDocument;
}
#endif //_DEBUG


// CWoZStudyView message handlers



//void CWoZStudyView::log() {
//	//do the logging
//	CString s;
//	s.Format(L"%d", pDoc->m_LogFrameTimeStamp.getNTicks());
//	pDoc->m_LogFrameTimeStamp.Write(s);
//}

void CWoZStudyView::displayImages(Mat overheadImage, Mat sceneImage) {
	//get full screen resolution
	int W = GetSystemMetrics(SM_CXSCREEN);
	int H = GetSystemMetrics(SM_CYSCREEN);
	
	int w2 = overheadImage.size().width;
	int h2 = overheadImage.size().height;
	int w3 = sceneImage.size().width;
	int h3 = sceneImage.size().height;
	RECT r2 = { W*0.2, H*0.4, W*0.2 + H*0.4 / h2 * w2, H*0.8 };
	RECT r3 = { W*0.2, H*0.0, W*0.2 + H*0.4 / h2 * w2, H*0.4 };

	ShowImg(overheadImage, &r2);
	InvalidateRect(&r2, FALSE);
	ShowImg(sceneImage, &r3);
	InvalidateRect(&r3, FALSE);
}

void CWoZStudyView::grabFrames() {
	pDoc->m_VidCapOverhead >> m_OverheadImage;
	flip(m_OverheadImage, m_OverheadImage, 1); //horizontal flip

	m_OverheadImage.copyTo(m_OverheadImageAnnotated);

	pDoc->m_VidCapScene >> m_SceneImage;
	flip(m_SceneImage, m_SceneImage, 1); //horizontal flip
}

void CWoZStudyView::saveVideos() {

	pDoc->m_VidWrtOverhead.write(m_OverheadImage);

	pDoc->m_VidWrtOverheadAnnotated.write(m_OverheadImageAnnotated);

	pDoc->m_VidWrtScene.write(m_SceneImage);
}

void CWoZStudyView::ShowImg(Mat &img, RECT *pRect)
{
	CDC *pDC = GetDC();
	if (pDC && pRect && !img.empty()) {
		uchar buffer[sizeof(BITMAPINFOHEADER) +1024];
		BITMAPINFO* bmi = (BITMAPINFO*) buffer;
		int bmp_w = img.cols, bmp_h = img.rows;

		BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);
		memset(bmih, 0, sizeof(*bmih));
		bmih->biSize = sizeof(BITMAPINFOHEADER);
		bmih->biWidth = bmp_w;
		bmih->biHeight = -bmp_h;// : -abs(height);
		bmih->biPlanes = 1;
		bmih->biBitCount = IPL_DEPTH_8U*img.channels();
		bmih->biCompression = BI_RGB;

		if (IPL_DEPTH_8U*img.channels() == 8) {
			RGBQUAD* palette = bmi->bmiColors;
			int i;
			for (i = 0; i < 256; i++) {
				palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE) i;
				palette[i].rgbReserved = 0;
			}
		}
		if (pRect->right - pRect->left + 1 < bmp_w) {
			SetStretchBltMode(
				pDC->m_hDC,           // handle to device context
				HALFTONE);
		}
		else {
			SetStretchBltMode(
				pDC->m_hDC,           // handle to device context
				COLORONCOLOR);
		}
		if (img.cols % 4 != 0 && img.isContinuous()) {
			IplImage *tempimg = cvCreateImage(img.size(), 8, img.channels());
			img.copyTo(Mat(tempimg));
			StretchDIBits(
				pDC->m_hDC,
				pRect->left, pRect->top, pRect->right - pRect->left + 1, pRect->bottom - pRect->top + 1,
				0, 0, bmp_w, bmp_h,
				tempimg->imageData, bmi, DIB_RGB_COLORS, SRCCOPY);
			cvReleaseImage(&tempimg);
		}
		else {
			StretchDIBits(
				pDC->m_hDC,
				pRect->left, pRect->top, pRect->right - pRect->left + 1, pRect->bottom - pRect->top + 1,
				0, 0, bmp_w, bmp_h,
				img.data, bmi, DIB_RGB_COLORS, SRCCOPY);
		}
	}
	return;
}

void CWoZStudyView::drawTimestamp(Mat &image) {

}

void CWoZStudyView::OnTimer(UINT_PTR nIDEvent)
{
	grabFrames();
	if (
		m_OverheadImage.data == NULL
		|| m_OverheadImageAnnotated.data == NULL
		|| m_SceneImage.data == NULL
		) {
		//if images are not ready yet, don't go on
		CView::OnTimer(nIDEvent);
		return;
	}
	
	//pDoc->m_LogFrameTimeStamp.tick();
	//pDoc->promptDecisionMaker.tick();
	pDoc->promptDecisionMaker.tick2();

	//annotate timestamp on overheadImageAnnotated
	putText(m_OverheadImageAnnotated, pDoc->promptDecisionMaker.log.getTimeStamp_msec(), Point(10, 30), FONT_HERSHEY_PLAIN, 2, Scalar(0, 255, 0));
	//if (pDoc->promptDecisionMaker.log.hasNewMessage()) {
		putText(m_OverheadImageAnnotated, pDoc->promptDecisionMaker.log.getLastMessage(), Point(10, 50), FONT_HERSHEY_PLAIN, 2, Scalar(0, 255, 0));
	//}

	displayImages(m_OverheadImageAnnotated, m_SceneImage);
	saveVideos();
	//log();

	CView::OnTimer(nIDEvent);
}