#pragma once

#include "cv.h"
#include "highgui.h"
#include "Resource.h"
#include "CvvImage.h"

// CShowImageDlg dialog

class CShowImageDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowImageDlg)

public:
	CShowImageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CShowImageDlg();

//Dialog members
protected:
	CvvImage cvvImage;
	IplImage* img;
public:
	IplImage* largeImg;
	//CDC*        m_pDC;
	int m_pixelsX, m_pixelsY, xpos, ypos;
	void SetImage(IplImage* im){img = im;}

public:
	void ResizeLargeImage(int Xpos, int Ypos, int width, int height);

// Dialog Data
	enum { IDD = IDD_SHOWIMAGE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};
