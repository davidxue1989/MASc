// PrematureStopDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AutismCOACH.h"
#include "PrematureStopDlg.h"
#include "afxdialogex.h"

#define confirmation_message "You've selected:"

// PrematureStopDlg dialog

IMPLEMENT_DYNAMIC(PrematureStopDlg, CDialog)

PrematureStopDlg::PrematureStopDlg(CWnd* pParent /*=NULL*/)
	: CDialog(PrematureStopDlg::IDD, pParent)
{

}

PrematureStopDlg::~PrematureStopDlg()
{
}

void PrematureStopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Radio(pDX, IDC_RADIO1, reason);
}


BEGIN_MESSAGE_MAP(PrematureStopDlg, CDialog)
	//ON_BN_CLICKED(IDC_BUTTON_SUBMIT, &PrematureStopDlg::OnBnClickedButtonSubmit)
	ON_BN_CLICKED(IDC_BUTTON3, &PrematureStopDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &PrematureStopDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &PrematureStopDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &PrematureStopDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &PrematureStopDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &PrematureStopDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &PrematureStopDlg::OnBnClickedButton9)
END_MESSAGE_MAP()


// PrematureStopDlg message handlers

//void PrematureStopDlg::OnBnClickedButtonSubmit()
//{
//	// TODO: Add your control notification handler code here
//
//	OnOK();
//}


void PrematureStopDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s SYSTEM_MALFUNCTIONING", confirmation_message);
	if (AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		reasonPremature = SYSTEM_MALFUNCTIONING;
		OnOK();
	}
}


void PrematureStopDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s CHILD_NEEDS_ASSISTANCE", confirmation_message);
	if (AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		reasonPremature = CHILD_NEEDS_ASSISTANCE;
		OnOK();
	}
}


void PrematureStopDlg::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s CHILD_IN_DISTRESS", confirmation_message);
	if (AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		reasonPremature = CHILD_IN_DISTRESS;
		OnOK();
	}
}


void PrematureStopDlg::OnBnClickedButton6()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s CHILD_IS_NOT_RESPONDING", confirmation_message);
	if (AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		reasonPremature = CHILD_IS_NOT_RESPONDING;
		OnOK();
	}
}


void PrematureStopDlg::OnBnClickedButton7()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s EMERGENCY", confirmation_message);
	if(AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		reasonPremature = EMERGENCY;
		OnOK();
	}
}


void PrematureStopDlg::OnBnClickedButton8()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s OTHERS_IMPORTANT", confirmation_message);
	if (AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		reasonPremature = OTHERS_IMPORTANT;
		OnOK();
	}
}


void PrematureStopDlg::OnBnClickedButton9()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s OTHERS_NOT_IMPORTANT", confirmation_message);
	if(AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		reasonPremature = OTHERS_NOT_IMPORTANT;
		OnOK();
	}
}
