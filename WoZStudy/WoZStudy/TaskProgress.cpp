// TaskProgress.cpp : implementation file
//

#include "stdafx.h"
#include "WoZStudy.h"
#include "TaskProgress.h"
#include "afxdialogex.h"


// TaskProgress dialog

IMPLEMENT_DYNAMIC(TaskProgress, CDialogEx)

TaskProgress::TaskProgress(CWnd* pParent /*=NULL*/)
: CDialogEx(TaskProgress::IDD, pParent)
, taskChanged(false)
{
}

TaskProgress::~TaskProgress()
{
}

void TaskProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(TaskProgress, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_INTRO, &TaskProgress::OnBnClickedButtonIntro)
	ON_BN_CLICKED(IDC_BUTTON_TURNONWATER, &TaskProgress::OnBnClickedButtonTurnonwater)
	ON_BN_CLICKED(IDC_BUTTON_WETHANDS, &TaskProgress::OnBnClickedButtonWethands)
	ON_BN_CLICKED(IDC_BUTTON_SQUEEZEOUTSOAP, &TaskProgress::OnBnClickedButtonSqueezeoutsoap)
	ON_BN_CLICKED(IDC_BUTTON_SCRUBHANDS, &TaskProgress::OnBnClickedButtonScrubhands)
	ON_BN_CLICKED(IDC_BUTTON_RINSEHANDS, &TaskProgress::OnBnClickedButtonRinsehands)
	ON_BN_CLICKED(IDC_BUTTON_TURNOFFWATER, &TaskProgress::OnBnClickedButtonTurnoffwater)
	ON_BN_CLICKED(IDC_BUTTON_DRYHANDS, &TaskProgress::OnBnClickedButtonDryhands)
	ON_BN_CLICKED(IDC_BUTTON_ALLDONE, &TaskProgress::OnBnClickedButtonAlldone)
	ON_BN_CLICKED(IDC_BUTTON_SYSTEMPAUSE, &TaskProgress::OnBnClickedButtonSystempause)
	ON_BN_CLICKED(IDC_BUTTON_IMMEDIATEPAUSE, &TaskProgress::OnBnClickedButtonImmediatepause)
	ON_BN_CLICKED(IDC_BUTTON_EMERGENCYSTOP, &TaskProgress::OnBnClickedButtonEmergencystop)
END_MESSAGE_MAP()


// TaskProgress message handlers



void TaskProgress::start() {
	CButton* pButton = (CButton*) GetDlgItem(IDC_BUTTON_SYSTEMPAUSE);
	pButton->SetCheck(true);
	ShowWindow(SW_SHOW);
	int W = GetSystemMetrics(SM_CXSCREEN);
	int H = GetSystemMetrics(SM_CYSCREEN);
	SetWindowPos(NULL, W*0.0, 0, W*0.2, H*0.4, NULL);
	ModifyStyle(WS_BORDER, 0);

	taskChanged = true;
	currentTask = SystemPause;
	lastTask = SystemPause;
}

void TaskProgress::updateDisplay() {
	int buttonId = -1;
	switch (currentTask)
	{
	case Intro:
		buttonId = IDC_BUTTON_INTRO;
		break;
	case TurnOnWater:
		buttonId = IDC_BUTTON_TURNONWATER;
		break;
	case WetYourHands:
		buttonId = IDC_BUTTON_WETHANDS;
		break;
	case GetSomeSoap:
		buttonId = IDC_BUTTON_SQUEEZEOUTSOAP;
		break;
	case ScrubYourHands:
		buttonId = IDC_BUTTON_SCRUBHANDS;
		break;
	case RinseYourHands:
		buttonId = IDC_BUTTON_RINSEHANDS;
		break;
	case TurnOffWater:
		buttonId = IDC_BUTTON_TURNOFFWATER;
		break;
	case DryYourHands:
		buttonId = IDC_BUTTON_DRYHANDS;
		break;
	case SystemPause:
		buttonId = IDC_BUTTON_SYSTEMPAUSE;
		break;
	case AllDone:
		buttonId = IDC_BUTTON_ALLDONE;
		break;
	case EmergencyStop:
		buttonId = IDC_BUTTON_EMERGENCYSTOP;
	default:
		ASSERT(FALSE);
		return;
	}
	CheckRadioButton(IDC_BUTTON_INTRO, IDC_BUTTON_EMERGENCYSTOP, buttonId);
}

std::wstring TaskProgress::getCurrentTaskString() {
	std::wstring taskStr = L"";
	switch (currentTask)
	{
	case SystemPause:
		taskStr = L"SystemPause";
		break;
	case Intro:
		taskStr = L"Intro";
		break;
	case TurnOnWater:
		taskStr = L"TurnOnWater";
		break;
	case WetYourHands:
		taskStr = L"WetYourHands";
		break;
	case GetSomeSoap:
		taskStr = L"GetSomeSoap";
		break;
	case ScrubYourHands:
		taskStr = L"ScrubYourHands";
		break;
	case RinseYourHands:
		taskStr = L"RinseYourHands";
		break;
	case TurnOffWater:
		taskStr = L"TurnOffWater";
		break;
	case DryYourHands:
		taskStr = L"DryYourHands";
		break;
	case AllDone:
		taskStr = L"AllDone";
		break;
	case EmergencyStop:
		taskStr = L"EmergencyStop";
		break;
	case ImmediatePause:
		taskStr = L"ImmediatePause";
		break;
	default:
		ASSERT(FALSE);
		return taskStr;
		break;
	}
	return taskStr;
}

void TaskProgress::incrementTask() {
	ASSERT(currentTask != AllDone && currentTask != SystemPause);
	taskChanged = true;
	lastTask = currentTask;
	currentTask = USERTASK(currentTask + 1);
	updateDisplay();
}

void TaskProgress::OnBnClickedButtonIntro()
{
	taskChanged = true;
	lastTask = currentTask;
	currentTask = Intro;
}


void TaskProgress::OnBnClickedButtonTurnonwater()
{
	taskChanged = true;
	lastTask = currentTask;
	currentTask = TurnOnWater;
}


void TaskProgress::OnBnClickedButtonWethands()
{
	taskChanged = true;
	lastTask = currentTask;
	currentTask = WetYourHands;
}


void TaskProgress::OnBnClickedButtonSqueezeoutsoap()
{
	taskChanged = true;
	lastTask = currentTask;
	currentTask = GetSomeSoap;
}


void TaskProgress::OnBnClickedButtonScrubhands()
{
	taskChanged = true;
	lastTask = currentTask;
	currentTask = ScrubYourHands;
}


void TaskProgress::OnBnClickedButtonRinsehands()
{
	taskChanged = true;
	lastTask = currentTask;
	currentTask = RinseYourHands;
}


void TaskProgress::OnBnClickedButtonTurnoffwater()
{
	taskChanged = true;
	lastTask = currentTask;
	currentTask = TurnOffWater;
}


void TaskProgress::OnBnClickedButtonDryhands()
{
	taskChanged = true;
	lastTask = currentTask;
	currentTask = DryYourHands;
}


void TaskProgress::OnBnClickedButtonAlldone()
{
	taskChanged = true;
	lastTask = currentTask;
	currentTask = AllDone;
}


void TaskProgress::OnBnClickedButtonSystempause()
{
	taskChanged = true;
	if (currentTask != SystemPause && currentTask != ImmediatePause && currentTask != EmergencyStop)
		lastTask = currentTask;
	currentTask = SystemPause;
}


void TaskProgress::OnBnClickedButtonImmediatepause()
{
	taskChanged = true;
	if (currentTask != SystemPause && currentTask != ImmediatePause && currentTask != EmergencyStop)
		lastTask = currentTask;
	currentTask = ImmediatePause;
}


void TaskProgress::OnBnClickedButtonEmergencystop()
{
	taskChanged = true;
	if (currentTask != SystemPause && currentTask != ImmediatePause && currentTask != EmergencyStop)
		lastTask = currentTask;
	currentTask = EmergencyStop;
}
