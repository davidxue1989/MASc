// PauseSystemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AutismCOACH.h"
#include "PauseSystemDlg.h"

#define confirmation_message "You've selected:"

// CPauseSystemDlg dialog

IMPLEMENT_DYNAMIC(CPauseSystemDlg, CDialog)

CPauseSystemDlg::CPauseSystemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPauseSystemDlg::IDD, pParent)
{
	systemResume = false;
	//currentUserState = "0";
}

CPauseSystemDlg::~CPauseSystemDlg()
{
}

void CPauseSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_EDIT3, reasonForPause);
	//DDX_Control(pDX, IDC_EDIT1, updateToState);
}


BEGIN_MESSAGE_MAP(CPauseSystemDlg, CDialog)
	//ON_BN_CLICKED(IDC_BUTTON11, &CPauseSystemDlg::OnBnClickedButton11)
	//ON_BN_CLICKED(IDC_BUTTON12, &CPauseSystemDlg::OnBnClickedButton12)
	//ON_BN_CLICKED(IDC_BUTTON13, &CPauseSystemDlg::OnBnClickedButton13)
	//ON_BN_CLICKED(IDC_BUTTON14, &CPauseSystemDlg::OnBnClickedButton14)
	//ON_BN_CLICKED(IDC_BUTTON15, &CPauseSystemDlg::OnBnClickedButton15)
	ON_BN_CLICKED(IDC_BUTTON2, &CPauseSystemDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CPauseSystemDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CPauseSystemDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CPauseSystemDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CPauseSystemDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CPauseSystemDlg::OnBnClickedButton7)
	//ON_BN_CLICKED(IDC_BUTTON1, &CPauseSystemDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON8, &CPauseSystemDlg::OnBnClickedButton8)
END_MESSAGE_MAP()

//dx oct 28, 2012:
BOOL CPauseSystemDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString userStateCStr;
	switch (userState)
	{
	case BEGINNING:
		userStateCStr = "wetting hands";
		break;

	case HANDS_WET:
		userStateCStr = "soaping hands";
		break;

	case GOT_SOAP:
		userStateCStr = "scrubbing hands";
		break;

	case DONE_SCRUBBING:
		userStateCStr = "rinsing hands";
		break;

	case DONE_RINSING:
		userStateCStr = "drying hands";
		break;

	case HANDS_DRIED:
		userStateCStr = "leaving the sink";//dx nov 28, 2012
		break;

	default:
		assert(false);
	}		
	
	pTextCtrl_CurrentUserState = new CStatic;

	//get the position of the text beside of which this is to be displayed
	CRect rect;
	CWnd *pWnd = GetDlgItem(IDC_STATIC_stuckOn);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect); 
	int width = rect.Width();
	rect.left += width;
	rect.right += width;
	//get the style
	DWORD style = pWnd->GetStyle();
	//get the font
	CFont *pfont = pWnd->GetFont();
	//change it to bold
	LOGFONT lf; 
	pfont->GetLogFont(&lf);
	lf.lfWeight = lf.lfWeight*2;
	m_font.DeleteObject(); //dxnote oct 28, 2012: need to delete the font first before creating a new one
	m_font.CreateFontIndirect(&lf);

	pTextCtrl_CurrentUserState->Create(userStateCStr, style, rect, this);
	pTextCtrl_CurrentUserState->SetFont(&m_font);

	return true;
}
BOOL CPauseSystemDlg::OnClose()
{
	CDialog::OnClose();
	delete pTextCtrl_CurrentUserState;
	pTextCtrl_CurrentUserState = NULL;
	return true;
}

// CPauseSystemDlg message handlers

//
//void CPauseSystemDlg::OnBnClickedButton11()
//{
//	reasonForPause.SetWindowTextA("Help Choose Reward");
//}

//void CPauseSystemDlg::OnBnClickedButton12()
//{
//	reasonForPause.SetWindowTextA("Washroom Assistance");
//}
//
//void CPauseSystemDlg::OnBnClickedButton13()
//{
//	reasonForPause.SetWindowTextA("Emergency");
//}
//
//void CPauseSystemDlg::OnBnClickedButton14()
//{
//	reasonForPause.SetWindowTextA("Other");
//}
//
//void CPauseSystemDlg::OnBnClickedButton15()
//{
//	reasonForPause.SetWindowTextA("System Alert");
//}

void CPauseSystemDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s BEGINNING", confirmation_message);
	if (AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		userState = BEGINNING;
		systemResume = true;
		Close();
	}
}

void CPauseSystemDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s HANDS_WET", confirmation_message);
	if (AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		userState = HANDS_WET;
		systemResume = true;
		Close();
	}
}

void CPauseSystemDlg::OnBnClickedButton4()
{	
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s GOT_SOAP", confirmation_message);
	if (AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		userState = GOT_SOAP;
		systemResume = true;
		Close();
	}
}

void CPauseSystemDlg::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s DONE_SCRUBBING", confirmation_message);
	if (AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		userState = DONE_SCRUBBING;
		systemResume = true;
		Close();
	}
}

void CPauseSystemDlg::OnBnClickedButton6()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s DONE_RINSING", confirmation_message);
	if (AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		userState = DONE_RINSING;
		systemResume = true;
		Close();
	}
}

void CPauseSystemDlg::OnBnClickedButton7()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s HANDS_DRIED", confirmation_message);
	if (AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		userState = HANDS_DRIED;
		systemResume = true;
		Close();
	}
}

//void CPauseSystemDlg::OnBnClickedButton1()//RETURN button
//{
//	
//	systemResume = true;
//	
//	//Get data from dialog
//	//If reason for pause was Help Choose Reward - get reward
//	//Log reason
//	//enter new state
//
//}

void CPauseSystemDlg::Close()
{	
	OnClose();
	OnOK();
}


void CPauseSystemDlg::OnBnClickedButton8()
{
	// TODO: Add your control notification handler code here
	CString reasonString;
	reasonString.Format("%s ALL_DONE_LEFT_THE_SINK", confirmation_message);
	if (AfxMessageBox(reasonString, MB_YESNO) == IDYES)
	{
		userState = ALL_DONE_LEFT_THE_SINK;
		systemResume = true;
		Close();
	}
}
