// StartupMenu.cpp : implementation file
//

#include "stdafx.h"
#include "AutismCOACH.h"
#include "StartupMenu.h"
#include "AutismCOACHDoc.h"


// CStartupMenu dialog

IMPLEMENT_DYNAMIC(CStartupMenu, CDialog)

CStartupMenu::CStartupMenu(CWnd* pParent /*=NULL*/)
	: CDialog(CStartupMenu::IDD, pParent)
{

}

CStartupMenu::~CStartupMenu()
{
}

void CStartupMenu::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CStartupMenu, CDialog)
	ON_BN_CLICKED(IDOK, &CStartupMenu::OnBnClickedEnterUserSettings)
	ON_BN_CLICKED(IDCANCEL, &CStartupMenu::OnBnClickedStartPrompting)
END_MESSAGE_MAP()


// CStartupMenu message handlers

void CStartupMenu::SetPointerToDoc(CDocument* pDoc)
{
	docPointer = pDoc;
}

void CStartupMenu::OnBnClickedEnterUserSettings()
{
	( (CAutismCOACHDoc*)(docPointer) )->OnFileEnterusersettings();
}


void CStartupMenu::OnBnClickedStartPrompting()
{
	( (CAutismCOACHDoc*)(docPointer) )->OnFileStartpromptingsession();
}
