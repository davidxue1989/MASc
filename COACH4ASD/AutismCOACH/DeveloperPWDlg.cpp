// CDeveloperPWDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AutismCOACH.h"
#include "DeveloperPWDlg.h"


// CDeveloperPWDlg dialog

IMPLEMENT_DYNAMIC(CDeveloperPWDlg, CDialog)

CDeveloperPWDlg::CDeveloperPWDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeveloperPWDlg::IDD, pParent)
{

}

CDeveloperPWDlg::~CDeveloperPWDlg()
{
}

void CDeveloperPWDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDeveloperPWDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CDeveloperPWDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDeveloperPWDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDeveloperPWDlg message handlers

void CDeveloperPWDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	GetDlgItemTextA(IDC_EDIT_PW,password);

	OnOK();
}

void CDeveloperPWDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}
