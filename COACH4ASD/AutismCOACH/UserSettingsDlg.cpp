// UserSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AutismCOACH.h"
#include "UserSettingsDlg.h"

// CUserSettingsDlg dialog

IMPLEMENT_DYNAMIC(CUserSettingsDlg, CDialog)

CUserSettingsDlg::CUserSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUserSettingsDlg::IDD, pParent)
	, startOver(0)
	, resoap(0)
	, useCountdown(0)
	, levelOnePrompt(0)
	, levelTwoPrompt(0)
	, levelThreePrompt(0)
	, levelFourPrompt(0)
	, levelFivePrompt(0)
	, priming(0)
	, rewardallsteps(0)

	, nPrompts(0)
	, nextPromptTime(0)
	, scrubTime(0)
	, rinseTime(0)
	, firstPromptTime(0)
	, use_attention_grabbers(0)
	, soapTime(0) //dx oct 28, 2012
	/*dx nov 09, 2012*/
	, wetTime(0)
	, dryTime(0)
	, volume(0)
{
	alreadyentered_usersettings = false;
}

CUserSettingsDlg::~CUserSettingsDlg()
{
}

void CUserSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Radio(pDX, IDC_RADIO5, startOver);
	//DDX_Radio(pDX, IDC_RADIO7, resoap);
	//DDX_Radio(pDX, IDC_RADIO39, useCountdown);

	//DDX_Radio(pDX, IDC_RADIO14, levelOnePrompt);
	//DDX_Radio(pDX, IDC_RADIO19, levelTwoPrompt);
	//DDX_Radio(pDX, IDC_RADIO24, levelThreePrompt);
	//DDX_Radio(pDX, IDC_RADIO29, levelFourPrompt);
	//DDX_Radio(pDX, IDC_RADIO34, levelFivePrompt);

	//DDX_Radio(pDX, IDC_RADIO41, priming);
	//DDX_Radio(pDX, IDC_RADIO43, rewardallsteps);

	//DDX_Text(pDX, IDC_EDIT13, nPrompts);
	//DDX_Text(pDX, IDC_EDIT14, firstPromptTime);
	//DDX_Text(pDX, IDC_EDIT15, nextPromptTime);
	//DDX_Text(pDX, IDC_EDIT16, rinseTime);
	//DDX_Text(pDX, IDC_EDIT17, scrubTime);
	//DDX_Control(pDX, IDC_SPIN, m_SpinQ6);
	//DDX_Control(pDX, IDC_SPIN13, m_SpinQ9);
	//DDX_Control(pDX, IDC_SPIN14, m_SpinQ10);
	//DDX_Control(pDX, IDC_SPIN15, m_SpinQ11);
	//DDX_Control(pDX, IDC_SPIN16, m_SpinQ12);
	//DDX_Radio(pDX, IDC_RADIO9, use_attention_grabbers);
}


BEGIN_MESSAGE_MAP(CUserSettingsDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CUserSettingsDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CUserSettingsDlg message handlers

void CUserSettingsDlg::OnBnClickedOk()
{

	//logFile_u.open(logFileName, ios::app);
	//logFile_u << clock() << "\t" << "\t" << "\t" << "\t" << "\t" << "\t" << "\t" << "\t" << "Entered User Settings" << endl;
	//logFile_u.close();

	UpdateData(true);

	//dx may 25, 2012: debugged: the ofstream code is commented out and use CStdioFile instead
	//userFile.open("C:\\dxFiles\\COACH4ASDresourceFiles\\UserSettings.txt");//Dec. 30
	//if(userFile.is_open())
	//{
	//	//Prompts to be given at each Prompt Level
	//	userFile << levelOnePrompt <<endl;
	//	userFile << levelTwoPrompt <<endl;
	//	userFile << levelThreePrompt <<endl;
	//	userFile << levelFourPrompt <<endl;
	//	userFile << levelFivePrompt <<endl;

	//	//Timing of Prompts
	//	userFile << firstPromptTime <<endl;
	//	userFile << nextPromptTime <<endl;

	//	//Number of Prompts to give
	//	userFile << nPrompts <<endl;

	//	//Visual Features (Priming and Attention Grabbers)
	//	userFile << priming <<endl;
	//	userFile << use_attention_grabbers <<endl;

	//	//Rewarding
	//	userFile << rewardallsteps <<endl;

	//	//Scrubbing, rinsing
	//	userFile << useCountdown <<endl;
	//	userFile << scrubTime <<endl;
	//	userFile << rinseTime <<endl;

	//	//Strictness
	//	userFile << startOver << endl;
	//	userFile << resoap << endl;

	//	userFile.close();

	//}
	
	SaveUserSettings();
	OnOK();
}

void CUserSettingsDlg::SaveUserSettings()
{
	CStdioFile userSettings;
	userSettings.Open("C:\\dxFiles\\COACH4ASDresourceFiles\\UserSettings.txt",CFile::modeReadWrite);
	CString s;
	s.Format("%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n"
		,levelOnePrompt,levelTwoPrompt,levelThreePrompt,levelFourPrompt,levelFivePrompt
		,firstPromptTime,nextPromptTime,nPrompts,priming,use_attention_grabbers
		,rewardallsteps,useCountdown,scrubTime,rinseTime,startOver,resoap,soapTime,wetTime,dryTime,volume);
	userSettings.WriteString(s);
	userSettings.Close();
	

	alreadyentered_usersettings = true;

	prompttypes[0] = levelOnePrompt;
	prompttypes[1] = levelTwoPrompt;
	prompttypes[2] = levelThreePrompt;
	prompttypes[3] = levelFourPrompt;
	prompttypes[4] = levelFivePrompt;

}

void CUserSettingsDlg::SaveUserSettingsForTrial(CString filename)
{
	userFile.open(filename);
	userFile << startOver << endl;
	userFile << resoap << endl;
	userFile << useCountdown <<endl;
	userFile << nPrompts <<endl;
	userFile << levelOnePrompt <<endl;
	userFile << levelTwoPrompt <<endl;
	userFile << levelThreePrompt <<endl;
	userFile << levelFourPrompt <<endl;
	userFile << levelFivePrompt <<endl;
	userFile << firstPromptTime <<endl;
	userFile << nextPromptTime <<endl;
	userFile << scrubTime <<endl;
	userFile << rinseTime <<endl;
	userFile << priming <<endl;
	userFile << rewardallsteps <<endl;
	userFile << use_attention_grabbers <<endl;
	userFile.close();
}

void CUserSettingsDlg::LoadDefaultUserSettings()
{
	ifstream userFile("C:\\dxFiles\\COACH4ASDresourceFiles\\UserSettings.txt");

	string line;
  
	if (userFile.is_open())
	{
		
		//Prompts to be given at each Prompt Level
		getline(userFile, line);
		levelOnePrompt = atoi(line.c_str());
		getline(userFile, line);
		levelTwoPrompt = atoi(line.c_str());
		getline(userFile, line);
		levelThreePrompt = atoi(line.c_str());
		getline(userFile, line);
		levelFourPrompt = atoi(line.c_str());
		getline(userFile, line);
		levelFivePrompt = atoi(line.c_str());

		//Timing of Prompts
		getline(userFile, line);
		firstPromptTime = atoi(line.c_str());
		getline(userFile, line);
		nextPromptTime  = atoi(line.c_str());

		//Number of Prompts to give
		getline(userFile, line);
		nPrompts = atoi(line.c_str());
		
		//Visual Features (Priming and Attention Grabbers)
		getline(userFile, line);
		priming = atoi(line.c_str());
		getline(userFile, line);
		use_attention_grabbers = atoi(line.c_str());
		
		//Rewarding
		getline(userFile, line);
		rewardallsteps = atoi(line.c_str());
		
		//Scrubbing, rinsing
		getline(userFile, line);
		useCountdown = atoi(line.c_str());
		getline(userFile, line);
		scrubTime = atoi(line.c_str());
		getline(userFile, line);
		rinseTime = atoi(line.c_str());
		
		//Strictness
		getline(userFile, line);
		startOver = atoi(line.c_str());
		getline(userFile, line);
		resoap = atoi(line.c_str());

		//dx oct 28, 2012
		getline(userFile, line);
		soapTime = atoi(line.c_str());
		//dx nov 09, 2012
		getline(userFile, line);
		wetTime = atoi(line.c_str());
		getline(userFile, line);
		dryTime = atoi(line.c_str());
		getline(userFile, line);
		volume = atoi(line.c_str());


		prompttypes[0] = levelOnePrompt;
		prompttypes[1] = levelTwoPrompt;
		prompttypes[2] = levelThreePrompt;
		prompttypes[3] = levelFourPrompt;
		prompttypes[4] = levelFivePrompt;

		userFile.close();
	}
  else cout << "Unable to open file";//for debug purposes only
}

BOOL CUserSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_SpinQ6.SetRange(0, 10);
	m_SpinQ9.SetRange(0, 20);
	m_SpinQ10.SetRange(0, 30);
	m_SpinQ11.SetRange(0, 20);
	m_SpinQ12.SetRange(0, 20);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

