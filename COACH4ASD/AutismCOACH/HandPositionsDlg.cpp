// HandPositionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AutismCOACH.h"
#include "HandPositionsDlg.h"

#include <MMSystem.h> // for playing a .wav file
#pragma comment(lib, "winmm.lib") // for playing a .wav file
#include <windows.h>

#define WETHANDS_BUTTON		119
#define GOTSOAP_BUTTON		111
#define SCRUBBED_BUTTON		 95
#define RINSED_BUTTON		 63
#define DRIEDHANDS_BUTTON	255
#define SOAPDISPENSER 1
#define TOWELDISPENSER 2

// CHandPositionsDlg dialog

IMPLEMENT_DYNAMIC(CHandPositionsDlg, CDialog)

CHandPositionsDlg::CHandPositionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHandPositionsDlg::IDD, pParent)
{
	buttoninput = 0;
}

CHandPositionsDlg::~CHandPositionsDlg()
{
}

void CHandPositionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, leftHandPos);
	DDX_Control(pDX, IDC_EDIT2, rightHandPos);
	DDX_Control(pDX, IDC_EDIT3, caregiverStatus);
	DDX_Control(pDX, IDC_EDIT4, updatedState);
	DDX_Control(pDX, IDC_EDIT5, waterStatus);
	DDX_Control(pDX, IDC_EDIT6, waterAlert);
	DDX_Control(pDX, IDC_EDIT7, ncctowel);
	DDX_Control(pDX, IDC_EDIT8, nccsoap);
	DDX_Control(pDX, IDC_EDIT9, ncctap);
	DDX_Control(pDX, IDC_EDIT10, nccwater);
	DDX_Control(pDX, IDC_EDIT11, nccsink);
}


BEGIN_MESSAGE_MAP(CHandPositionsDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CHandPositionsDlg::OnBnClicked_away_left)
	ON_BN_CLICKED(IDC_BUTTON2, &CHandPositionsDlg::OnBnClickedButton_away_right)
	ON_BN_CLICKED(IDC_BUTTON3, &CHandPositionsDlg::OnBnClickedButton_sink_left)
	ON_BN_CLICKED(IDC_BUTTON4, &CHandPositionsDlg::OnBnClickedButton_sink_right)
	ON_BN_CLICKED(IDC_BUTTON5, &CHandPositionsDlg::OnBnClickedButton_water_left)
	ON_BN_CLICKED(IDC_BUTTON6, &CHandPositionsDlg::OnBnClickedButton_water_right)
	ON_BN_CLICKED(IDC_BUTTON7, &CHandPositionsDlg::OnBnClickedButton_tap_left)
	ON_BN_CLICKED(IDC_BUTTON8, &CHandPositionsDlg::OnBnClickedButton_tap_right)
	ON_BN_CLICKED(IDC_BUTTON9, &CHandPositionsDlg::OnBnClickedButton_soap_left)
	ON_BN_CLICKED(IDC_BUTTON10, &CHandPositionsDlg::OnBnClickedButton_soap_right)
	ON_BN_CLICKED(IDC_BUTTON11, &CHandPositionsDlg::OnBnClickedButton_towel_left)
	ON_BN_CLICKED(IDC_BUTTON12, &CHandPositionsDlg::OnBnClickedButton_towel_right)
	ON_BN_CLICKED(IDC_BUTTON13, &CHandPositionsDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON14, &CHandPositionsDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON15, &CHandPositionsDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON16, &CHandPositionsDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON17, &CHandPositionsDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON18, &CHandPositionsDlg::OnBnClickedButton18)
	ON_BN_CLICKED(IDC_BUTTON19, &CHandPositionsDlg::OnBnClickedButton19)
	ON_BN_CLICKED(IDC_BUTTON20, &CHandPositionsDlg::OnBnClickedButton20)
	ON_BN_CLICKED(IDC_BUTTON21, &CHandPositionsDlg::OnBnClickedButton21)
	ON_BN_CLICKED(IDC_BUTTON22, &CHandPositionsDlg::OnBnClickedButton22)
	ON_BN_CLICKED(IDC_BUTTON23, &CHandPositionsDlg::StopPromptingMusic)
	ON_BN_CLICKED(IDC_BUTTON24, &CHandPositionsDlg::PausePromptingMusic)
	ON_BN_CLICKED(IDC_BUTTON25, &CHandPositionsDlg::ResumePromptingMusic)
	//ON_BN_CLICKED(IDC_BUTTON26, &CHandPositionsDlg::RestartPromptingMusic)
	ON_BN_CLICKED(IDC_BUTTON27, &CHandPositionsDlg::OnBnClickedButtonWaterOn)
	ON_BN_CLICKED(IDC_BUTTON28, &CHandPositionsDlg::OnBnClickedButtonWaterOff)
	ON_BN_CLICKED(IDC_BUTTON29, &CHandPositionsDlg::OnBnClickedButton29)
	ON_BN_CLICKED(IDC_BUTTON30, &CHandPositionsDlg::OnBnClickedButton30)
	ON_BN_CLICKED(IDC_BUTTON31, &CHandPositionsDlg::OnBnClickedButton31)
END_MESSAGE_MAP()


// CHandPositionsDlg message handlers

void CHandPositionsDlg::OnBnClicked_away_left()
{
	leftHandPos.SetWindowTextA("away");
}

void CHandPositionsDlg::OnBnClickedButton_away_right()
{
	rightHandPos.SetWindowTextA("away");
}

void CHandPositionsDlg::OnBnClickedButton_sink_left()
{
	leftHandPos.SetWindowTextA("sink");
}

void CHandPositionsDlg::OnBnClickedButton_sink_right()
{
	rightHandPos.SetWindowTextA("sink");
}
void CHandPositionsDlg::OnBnClickedButton_water_left()
{
	leftHandPos.SetWindowTextA("water");
}

void CHandPositionsDlg::OnBnClickedButton_water_right()
{
	rightHandPos.SetWindowTextA("water");
}

void CHandPositionsDlg::OnBnClickedButton_tap_left()
{
	leftHandPos.SetWindowTextA("tap");
}

void CHandPositionsDlg::OnBnClickedButton_tap_right()
{
	rightHandPos.SetWindowTextA("tap");
}

void CHandPositionsDlg::OnBnClickedButton_soap_left()
{
	leftHandPos.SetWindowTextA("soap");
	buttoninput = SOAPDISPENSER;
}

void CHandPositionsDlg::OnBnClickedButton_soap_right()
{
	rightHandPos.SetWindowTextA("soap");
	buttoninput = SOAPDISPENSER;
}

void CHandPositionsDlg::OnBnClickedButton_towel_left()
{
	leftHandPos.SetWindowTextA("towel");
	buttoninput = TOWELDISPENSER;
}

void CHandPositionsDlg::OnBnClickedButton_towel_right()
{
	rightHandPos.SetWindowTextA("towel");
	buttoninput = TOWELDISPENSER;
}

void CHandPositionsDlg::OnBnClickedButton1()
{
	buttoninput = WETHANDS_BUTTON;
}

void CHandPositionsDlg::OnBnClickedButton2()
{
	buttoninput = GOTSOAP_BUTTON;
}

void CHandPositionsDlg::OnBnClickedButton3()
{
	buttoninput = SCRUBBED_BUTTON;
}

void CHandPositionsDlg::OnBnClickedButton4()
{
	buttoninput = RINSED_BUTTON;
}

void CHandPositionsDlg::OnBnClickedButton5()
{
	buttoninput = DRIEDHANDS_BUTTON;
}

short CHandPositionsDlg::GetButtonInput()
{
	short temp = buttoninput;
	buttoninput = 0;
	return temp;
}
void CHandPositionsDlg::OnBnClickedButton18()
{
	caregiverStatus.SetWindowTextA("Intervention Complete");
	updatedState.SetWindowTextA("1");
}

void CHandPositionsDlg::OnBnClickedButton19()
{
	caregiverStatus.SetWindowTextA("Intervention Complete");
	updatedState.SetWindowTextA("2");
}

void CHandPositionsDlg::OnBnClickedButton20()
{
	caregiverStatus.SetWindowTextA("Intervention Complete");
	updatedState.SetWindowTextA("3");
}

void CHandPositionsDlg::OnBnClickedButton21()
{
	caregiverStatus.SetWindowTextA("Intervention Complete");
	updatedState.SetWindowTextA("4");
}

void CHandPositionsDlg::OnBnClickedButton22()
{
	caregiverStatus.SetWindowTextA("Intervention Complete");
	updatedState.SetWindowTextA("5");
}

void CHandPositionsDlg::StopPromptingMusic()//Stop Music
{
	mciSendString("stop mysound", NULL, 0, 0);
	//logFile_h.open(pDoc->logFileName, ios::app);
	//logFile_h << clock() << "\t" << "\t" << "\t" << "Music Stopped" << endl;
	//logFile_h.close();
}

void CHandPositionsDlg::PausePromptingMusic()//Pause Music
{
	mciSendString("pause mysound", NULL, 0, 0);
	//logFile_h.open(pDoc->logFileName, ios::app);
	//logFile_h << clock() << "\t" << "\t" << "\t" <<"Music Paused" << endl;
	//logFile_h.close();
}

void CHandPositionsDlg::ResumePromptingMusic()
{
	mciSendString("resume mysound", NULL, 0, 0);
	//logFile_h.open(pDoc->logFileName, ios::app);
	//logFile_h << clock() << "\t" << "\t" << "\t" << "Music Resumed" << endl;
	//logFile_h.close();
}
//void CHandPositionsDlg::RestartPromptingMusic()
//{
//	mciSendString("play mysound from 0", NULL, 0, 0);
//	//logFile_h.open(pDoc->logFileName, ios::app);
//	//logFile_h << clock() << "\t" << "\t" << "\t" << "Music Restarted" << endl;
//	//logFile_h.close();
//	
//}

void CHandPositionsDlg::OnBnClickedButtonWaterOn()
{
	waterStatus.SetWindowTextA("WATER ON");
}

void CHandPositionsDlg::OnBnClickedButtonWaterOff()
{
	waterStatus.SetWindowTextA("WATER OFF");
}
void CHandPositionsDlg::OnBnClickedButton29()
{
	leftHandPos.SetWindowTextA("off task");
}

void CHandPositionsDlg::OnBnClickedButton30()
{
	rightHandPos.SetWindowTextA("off task");
}

void CHandPositionsDlg::OnBnClickedButton31()
{
	waterAlert.SetWindowTextA("TURN OFF WATER!");
}
