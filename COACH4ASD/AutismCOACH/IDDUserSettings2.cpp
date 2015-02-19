// IDDUserSettings2.cpp : implementation file
//

#include "stdafx.h"
#include "AutismCOACH.h"
#include "IDDUserSettings2.h"
#include "afxdialogex.h"


// IDDUserSettings2 dialog

IMPLEMENT_DYNAMIC(IDDUserSettings2, CDialog)

IDDUserSettings2::IDDUserSettings2(CWnd* pParent /*=NULL*/)
	: CDialog(IDDUserSettings2::IDD, pParent)
{
	firstPromptTime = 0;
	nextPromptTime = 0;
	nPrompts = 0;
	priming = 0;
	use_attention_grabbers = 0;
	rewardallsteps = 0;
}

IDDUserSettings2::~IDDUserSettings2()
{
}

void IDDUserSettings2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//DDX_Text(pDX, IDC_EDIT_FIRSTPROMPT, firstPromptTime);
	//DDX_Text(pDX, IDC_EDIT_BETWEENPROMPTS, nextPromptTime);
	//DDX_Text(pDX, IDC_EDIT_NPROMPTS, nPrompts);
	DDX_Radio(pDX, IDC_RADIO1, priming);
	DDX_Radio(pDX, IDC_RADIO3, use_attention_grabbers);	
	DDX_Radio(pDX, IDC_RADIO11, rewardallsteps);
}

BOOL IDDUserSettings2::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// disable IDPREV, enable IDNEXT
	//this->GetDlgItem(IDC_BUTTON_PREV)->EnableWindow(TRUE);
	this->GetDlgItem(IDC_BUTTON_PREV)->EnableWindow(FALSE);
	this->GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(TRUE);

	font.DeleteObject();
	font.CreateFontA(30, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");//dxtodo sep 18 ,2012: debug fails here

	GetDlgItem(IDC_STATIC12)->SetFont(&font);
	//GetDlgItem(IDC_STATIC14)->SetFont(&font);
	GetDlgItem(IDC_STATIC13)->SetFont(&font);	
	GetDlgItem(IDC_STATIC15)->SetFont(&font);
	GetDlgItem(IDC_STATIC6)->SetFont(&font);
	GetDlgItem(IDC_STATIC8)->SetFont(&font);
	GetDlgItem(IDC_STATIC10)->SetFont(&font);

	//dx nov 10, 2012
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", firstPromptTime);
	GetDlgItem(IDC_STATIC_firstPrompt)->SetWindowTextA(ctrs_staticText);
	ctrs_staticText.Format("%dsec", nextPromptTime);
	GetDlgItem(IDC_STATIC_nextPromptTime)->SetWindowTextA(ctrs_staticText);
	ctrs_staticText.Format("%d", nPrompts);
	GetDlgItem(IDC_STATIC_nPrompts)->SetWindowTextA(ctrs_staticText);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(IDDUserSettings2, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PREV, &IDDUserSettings2::OnBnClickedButtonPrev)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &IDDUserSettings2::OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_BUTTON1, &IDDUserSettings2::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON32, &IDDUserSettings2::OnBnClickedButton32)
	ON_BN_CLICKED(IDC_BUTTON33, &IDDUserSettings2::OnBnClickedButton33)
	ON_BN_CLICKED(IDC_BUTTON34, &IDDUserSettings2::OnBnClickedButton34)
	ON_BN_CLICKED(IDC_BUTTON35, &IDDUserSettings2::OnBnClickedButton35)
	ON_BN_CLICKED(IDC_BUTTON36, &IDDUserSettings2::OnBnClickedButton36)
END_MESSAGE_MAP()


// IDDUserSettings2 message handlers


void IDDUserSettings2::OnBnClickedButtonPrev()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	EndDialog(IDC_BUTTON_PREV);
}


void IDDUserSettings2::OnBnClickedButtonNext()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	EndDialog(IDC_BUTTON_NEXT);
}


void IDDUserSettings2::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here	
	UpdateData(true); //temporarily save the data
	firstPromptTime++;	
	if (firstPromptTime > 30)
		firstPromptTime = 30;
	UpdateData(false); //refresh the page based on new data
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", firstPromptTime);
	GetDlgItem(IDC_STATIC_firstPrompt)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings2::OnBnClickedButton32()
{
	// TODO: Add your control notification handler code here	
	UpdateData(true);
	firstPromptTime--;
	if (firstPromptTime < 0)
		firstPromptTime = 0;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", firstPromptTime);
	GetDlgItem(IDC_STATIC_firstPrompt)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings2::OnBnClickedButton33()
{
	// TODO: Add your control notification handler code here		
	UpdateData(true);
	nextPromptTime++;
	if (nextPromptTime > 30)
		nextPromptTime = 30;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", nextPromptTime);
	GetDlgItem(IDC_STATIC_nextPromptTime)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings2::OnBnClickedButton34()
{
	// TODO: Add your control notification handler code here	
	UpdateData(true);
	nextPromptTime--;
	if (nextPromptTime < 0)
		nextPromptTime = 0;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", nextPromptTime);
	GetDlgItem(IDC_STATIC_nextPromptTime)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings2::OnBnClickedButton35()
{
	// TODO: Add your control notification handler code here	
	UpdateData(true);
	nPrompts++;
	if (nPrompts > 10)
		nPrompts = 10;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%d", nPrompts);
	GetDlgItem(IDC_STATIC_nPrompts)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings2::OnBnClickedButton36()
{
	// TODO: Add your control notification handler code here	
	UpdateData(true);
	nPrompts--;
	if (nPrompts < 1)
		nPrompts = 1;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%d", nPrompts);
	GetDlgItem(IDC_STATIC_nPrompts)->SetWindowTextA(ctrs_staticText);
}
