// ShowImageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WoZStudy.h"
#include "ShowImageDlg.h"
#include "afxdialogex.h"


// ShowImageDlg dialog

IMPLEMENT_DYNAMIC(ShowImageDlg, CDialogEx)

ShowImageDlg::ShowImageDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(ShowImageDlg::IDD, pParent)
	, painted(false)
{

}

ShowImageDlg::~ShowImageDlg()
{
}

void ShowImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ShowImageDlg, CDialogEx)
END_MESSAGE_MAP()


// ShowImageDlg message handlers

void ShowImageDlg::hideImg() {
	ShowWindow(SW_HIDE);
}

void ShowImageDlg::ShowImg(Mat &img, RECT *pRect)
{
	ShowWindow(SW_SHOW);
	//if (!painted) {
		SetWindowPos(&wndTopMost, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, NULL);

		pRect->right -= pRect->left;
		pRect->left = 0;
		pRect->bottom -= pRect->top;
		pRect->top = 0;
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
		//painted = true;
	//}

	return;
}