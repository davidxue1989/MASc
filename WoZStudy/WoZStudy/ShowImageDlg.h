#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

// ShowImageDlg dialog

class ShowImageDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ShowImageDlg)

public:
	ShowImageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~ShowImageDlg();
	void ShowImg(Mat &img, RECT *pRECT);
	void hideImg();

	bool painted;

// Dialog Data
	enum { IDD = IDD_DIALOG_SHOWIMAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
