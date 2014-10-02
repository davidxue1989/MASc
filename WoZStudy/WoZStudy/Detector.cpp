// GazeDetector.cpp : implementation file
//

#include "stdafx.h"
#include "WoZStudy.h"
#include "Detector.h"
#include "afxdialogex.h"


// GazeDetector dialog

IMPLEMENT_DYNAMIC(Detector, CDialogEx)

Detector::Detector(CWnd* pParent, CString tag, CString msg)
: CDialogEx(Detector::IDD, pParent)
, tag(tag)
, msg(msg)
, detected(false)
, started(false)
{
}

Detector::~Detector()
{
}

void Detector::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Detector, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO_detected, &Detector::OnBnClickedRadiodetected)
	ON_BN_CLICKED(IDC_RADIO_not_detected, &Detector::OnBnClickedRadionotdetected)
END_MESSAGE_MAP()


// GazeDetector message handlers



void Detector::OnBnClickedRadiodetected()
{
	CButton* pButton = (CButton*) GetDlgItem(IDC_RADIO_detected);
	if (pButton->GetCheck())
		detected = true;
}


void Detector::OnBnClickedRadionotdetected()
{
	CButton* pButton = (CButton*) GetDlgItem(IDC_RADIO_not_detected);
	if (pButton->GetCheck())
		detected = false;
}

void Detector::start() {
	CButton* pButton = (CButton*) GetDlgItem(IDC_RADIO_not_detected);
	pButton->SetCheck(true);
	pButton = (CButton*) GetDlgItem(IDC_RADIO_detected);
	pButton->SetCheck(false);

	SetWindowTextW(tag);
	CStatic* pText = (CStatic*) GetDlgItem(IDC_TEXT);
	pText->SetWindowTextW(msg);

	ShowWindow(SW_SHOW);
	int W = GetSystemMetrics(SM_CXSCREEN);
	int H = GetSystemMetrics(SM_CYSCREEN);
	SetWindowPos(NULL, W*0.0, H*0.7, W*0.2, H*0.3, NULL);
	ModifyStyle(WS_BORDER, 0);
	started = true;
	detected = false;
}

void Detector::stop() {
	ShowWindow(SW_HIDE);
	started = false;
}