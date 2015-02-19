// UserDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AutismCOACH.h"
#include "UserDataDlg.h"

// CUserDataDlg dialog

IMPLEMENT_DYNAMIC(CUserDataDlg, CDialog)

CUserDataDlg::CUserDataDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUserDataDlg::IDD, pParent)
	, participantName(_T(""))
	, trialDate(_T(""))
	, trialNumber(0)
	, trialStartTime(_T(""))
	, calibrationFileName(_T(""))
{

}

CUserDataDlg::~CUserDataDlg()
{
}

void CUserDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, participantName);
	DDX_Text(pDX, IDC_EDIT3, trialDate);
	DDX_Text(pDX, IDC_EDIT2, trialNumber);
	DDX_Text(pDX, IDC_EDIT4, trialStartTime);
	DDX_Text(pDX, IDC_EDIT5, calibrationFileName);
}


BEGIN_MESSAGE_MAP(CUserDataDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CUserDataDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CUserDataDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CUserDataDlg::OnBnClickedButtonBrowse)
END_MESSAGE_MAP()


// CUserDataDlg message handlers

void CUserDataDlg::OnBnClickedOk()
{
	UpdateData(true);

	//Create a string with participant name and trial number
	CString userDataFileName = "";

	userDataFileName.Append(participantName);
	userDataFileName.Append("_TrialNumber_");
	userDataFileName.Format("%s%d", userDataFileName, trialNumber); //dx nov 05, 2012
	//char * trialNumberchar = new char[5];
	//_itoa_s(trialNumber, trialNumberchar, 25, 10);
	//userDataFileName.AppendChar(trialNumberchar[0]);
	//delete [] trialNumberchar;
	


	//String is now - D:\Visual Studio 2008\Projects\AutismCOACHMar9\participantName_TrialNumber_trialNumber


	//Now save the user settings to a file with appropriate name


	//create usersettings file and logfile
	CString string2 = "_TrialUserSettings.txt";
	CString string3 = "_TrialLogFile.txt";

	
	//Save the info for the trial
	CString trialInfo = userDataFileName;
	trialInfo.Append(".txt");
	mostrecentuserdata.open("C:\\dxFiles\\COACH4ASDresourceFiles\\MostRecentUserData.txt");
	mostrecentuserdata << participantName << endl << trialDate << endl << trialNumber << endl << trialStartTime << endl << calibrationFileName << endl;
	mostrecentuserdata.close();

	//Name the User Settings file specific to this trial
	trialUserSettings = userDataFileName;
	trialUserSettings.Append(string2);

	trialLogFile = userDataFileName;
	trialLogFile.Append(string3);
	
	OnOK();
}

void CUserDataDlg::OnBnClickedCancel()
{
	UpdateData(true);

	OnCancel();
}

void CUserDataDlg::OnBnClickedButtonBrowse()
{
	CFileDialog dlg(TRUE);//, _T("*.bmp"), NULL,

	if (dlg.DoModal() == IDOK){
		calibrationFileName = dlg.GetPathName();  // contain the selected filename
		GetDlgItem(IDC_EDIT5)->SetWindowTextA(calibrationFileName);
	}
}