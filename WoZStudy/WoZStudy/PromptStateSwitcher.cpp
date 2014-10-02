// PromptStateSwitcher.cpp : implementation file
//

#include "stdafx.h"
#include "WoZStudy.h"
#include "PromptStateSwitcher.h"
#include "afxdialogex.h"


// PromptStateSwitcher dialog

IMPLEMENT_DYNAMIC(PromptStateSwitcher, CDialogEx)

PromptStateSwitcher::PromptStateSwitcher(CWnd* pParent /*=NULL*/)
: CDialogEx(PromptStateSwitcher::IDD, pParent)
, stateChanged(false)
, currentState(Other)
{
}

PromptStateSwitcher::~PromptStateSwitcher()
{
}

void PromptStateSwitcher::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PromptStateSwitcher, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_FIRSTPROMPT, &PromptStateSwitcher::OnBnClickedButtonFirstprompt)
	ON_BN_CLICKED(IDC_BUTTON_FIRSTAG, &PromptStateSwitcher::OnBnClickedButtonFirstag)
	ON_BN_CLICKED(IDC_BUTTON_SECONDAG, &PromptStateSwitcher::OnBnClickedButtonSecondag)
	ON_BN_CLICKED(IDC_BUTTON_SECONDPROMPT, &PromptStateSwitcher::OnBnClickedButtonSecondprompt)
	ON_BN_CLICKED(IDC_BUTTON_REWARD, &PromptStateSwitcher::OnBnClickedButtonReward)
	ON_BN_CLICKED(IDC_BUTTON_LETUSCONTINUE, &PromptStateSwitcher::OnBnClickedButtonLetuscontinue)
END_MESSAGE_MAP()


// PromptStateSwitcher message handlers

void PromptStateSwitcher::start() {
	ShowWindow(SW_SHOW);
	int W = GetSystemMetrics(SM_CXSCREEN);
	int H = GetSystemMetrics(SM_CYSCREEN);
	SetWindowPos(NULL, W*0.0, H*0.4, W*0.2, H*0.3, NULL);
	ModifyStyle(WS_BORDER, 0);

	stateChanged = false;
	currentState = Other;
	updateDisplay();
}

void PromptStateSwitcher::updateDisplay() {
	int buttonId = -1;
	switch (currentState)
	{
	case FirstPrompt:
		buttonId = IDC_BUTTON_FIRSTPROMPT;
		break;
	case FirstAG:
		buttonId = IDC_BUTTON_FIRSTAG;
		break;
	case SecondAG:
		buttonId = IDC_BUTTON_SECONDAG;
		break;
	case SecondPrompt:
		buttonId = IDC_BUTTON_SECONDPROMPT;
		break;
	case Reward:
		buttonId = IDC_BUTTON_REWARD;
		break;
	case LetUsContinue:
		buttonId = IDC_BUTTON_LETUSCONTINUE;
		break;
	case Other:
		//buttonId = -1;
		break;
	default:
		ASSERT(FALSE);
		return;
	}
	CheckRadioButton(IDC_BUTTON_FIRSTPROMPT, IDC_BUTTON_LETUSCONTINUE, buttonId);
}


void PromptStateSwitcher::OnBnClickedButtonFirstprompt()
{
	stateChanged = true;
	currentState = FirstPrompt;
}


void PromptStateSwitcher::OnBnClickedButtonFirstag()
{
	stateChanged = true;
	currentState = FirstAG;
}


void PromptStateSwitcher::OnBnClickedButtonSecondag()
{
	stateChanged = true;
	currentState = SecondAG;
}


void PromptStateSwitcher::OnBnClickedButtonSecondprompt()
{
	stateChanged = true;
	currentState = SecondPrompt;
}


void PromptStateSwitcher::OnBnClickedButtonReward()
{
	stateChanged = true;
	currentState = Reward;
}


void PromptStateSwitcher::OnBnClickedButtonLetuscontinue()
{
	stateChanged = true;
	currentState = LetUsContinue;
}
