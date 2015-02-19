// ShowImageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AutismCOACH.h"
#include "ShowImageDlg.h"
#include "cv.h"


// CShowImageDlg dialog

IMPLEMENT_DYNAMIC(CShowImageDlg, CDialog)

CShowImageDlg::CShowImageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShowImageDlg::IDD, pParent)
{
	img = NULL;
	largeImg = NULL;

	//m_pDC = GetDC();

	//m_pDC->GetDeviceCaps(HORZRES);2960
    //m_pDC->GetDeviceCaps(VERTRES);1024
	//1280,1024/5

}
void CShowImageDlg::ResizeLargeImage(int Xpos, int Ypos, int width, int height)
{
	xpos = Xpos;
	ypos = Ypos;
	largeImg = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);
}

CShowImageDlg::~CShowImageDlg()
{
	if(largeImg != NULL)
		cvReleaseImage(&largeImg);
}

void CShowImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CShowImageDlg, CDialog)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CShowImageDlg message handlers

void CShowImageDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	cvResize(img, largeImg);

	cvvImage.CopyOf(largeImg);//copy to display on CDC
	cvvImage.Show(this->GetDC()->m_hDC, 0, 0, largeImg->width, largeImg->height, 0, 0);
	cvvImage.Destroy();

	//cvShowImage("test", largeImg);//dxdebug nov 22, 2012
	//cvWaitKey(1);

	HWND hWndChild = this->m_hWnd;
	::MoveWindow(hWndChild, xpos,ypos,largeImg->width,largeImg->height,TRUE);
	SetWindowPos(&wndTopMost, xpos, ypos, largeImg->width, largeImg->height, SWP_NOMOVE | SWP_NOSIZE); //dx may 23, 2012: make the dialog on top
}