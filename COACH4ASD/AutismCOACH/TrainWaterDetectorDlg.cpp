// TrainWaterDetectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AutismCOACH.h"
#include "TrainWaterDetectorDlg.h"


// CTrainWaterDetectorDlg dialog

IMPLEMENT_DYNAMIC(CTrainWaterDetectorDlg, CDialog)

CTrainWaterDetectorDlg::CTrainWaterDetectorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTrainWaterDetectorDlg::IDD, pParent)
	, waterDetectionCategory(0)
{
	userReadyToContinue = false;
}

CTrainWaterDetectorDlg::~CTrainWaterDetectorDlg()
{
}

void CTrainWaterDetectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, waterDetectionCategory);
}


BEGIN_MESSAGE_MAP(CTrainWaterDetectorDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CTrainWaterDetectorDlg::OnBnClickedContinue)
END_MESSAGE_MAP()


// CTrainWaterDetectorDlg message handlers

void CTrainWaterDetectorDlg::OnBnClickedContinue()
{
	UpdateData(true);
	userReadyToContinue = true;
	OnOK();
}
