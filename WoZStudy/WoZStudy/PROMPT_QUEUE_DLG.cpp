// PROMPT_QUEUE_DLG.cpp : implementation file
//

#include "stdafx.h"
#include "WoZStudy.h"
#include "PROMPT_QUEUE_DLG.h"
#include "afxdialogex.h"


// PROMPT_QUEUE_DLG dialog

IMPLEMENT_DYNAMIC(PROMPT_QUEUE_DLG, CDialogEx)

PROMPT_QUEUE_DLG::PROMPT_QUEUE_DLG(CWnd* pParent /*=NULL*/)
	: CDialogEx(PROMPT_QUEUE_DLG::IDD, pParent)
{

}

PROMPT_QUEUE_DLG::~PROMPT_QUEUE_DLG()
{
}


void PROMPT_QUEUE_DLG::start() {
	ShowWindow(SW_SHOW);
	int W = GetSystemMetrics(SM_CXSCREEN);
	int H = GetSystemMetrics(SM_CYSCREEN);
	SetWindowPos(NULL, W*0.2, H*0.8, W*0.5, H*1.0, NULL);
	ModifyStyle(WS_BORDER, 0);

	updateDisplay(L"");
}

void PROMPT_QUEUE_DLG::updateDisplay(std::wstring text) {
	GetDlgItem(IDC_PROMPT_QUEUE_TEXT)->SetWindowTextW((L"curent queue: " + text).c_str());
}

void PROMPT_QUEUE_DLG::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PROMPT_QUEUE_DLG, CDialogEx)
END_MESSAGE_MAP()


// PROMPT_QUEUE_DLG message handlers
