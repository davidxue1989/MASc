// IDDUserSettings3.cpp : implementation file
//

#include "stdafx.h"
#include "AutismCOACH.h"
#include "IDDUserSettings3.h"
#include "afxdialogex.h"

//dx nov 09, 2012: for changing volumes
#include <mmdeviceapi.h>
#include <endpointvolume.h>

#include <MMSystem.h> //dx oct 31, 2012: for PlaySound


#define VOLUME_TIC_FREQUENCY 100

// IDDUserSettings3 dialog

IMPLEMENT_DYNAMIC(IDDUserSettings3, CDialog)

IDDUserSettings3::IDDUserSettings3(CWnd* pParent /*=NULL*/)
	: CDialog(IDDUserSettings3::IDD, pParent)
{
	useCountdown = 0;
	scrubTime = 0;
	rinseTime = 0;
	soapTime = 0;
	//dx nov 09, 2012
	wetTime = 0;
	dryTime = 0;
	volume = 0;
	pVolumeText = NULL;

	pSettingDlg1 = NULL;
	pSettingDlg2 = NULL;
}

IDDUserSettings3::~IDDUserSettings3()
{
}

void IDDUserSettings3::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO3, useCountdown);
	//DDX_Text(pDX, IDC_EDIT_SCRUBTIME, scrubTime);
	//DDX_Text(pDX, IDC_EDIT_RINSETIME, rinseTime);
	//DDX_Text(pDX, IDC_EDIT_SOAPTIME2, soapTime);
	//DDX_Text(pDX, IDC_EDIT_WETTIME, wetTime);
	//DDX_Text(pDX, IDC_EDIT_DRYTIME, dryTime);
	DDX_Slider(pDX, IDC_SLIDER1, volume);
	DDX_Control(pDX, IDC_SLIDER1, m_SliderCtrl);
}

BOOL IDDUserSettings3::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// disable IDPREV, enable IDNEXT
	this->GetDlgItem(IDC_BUTTON_PREV)->EnableWindow(TRUE);
	this->GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);

	font.DeleteObject();
	font.CreateFontA(30, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");//dxtodo sep 18 ,2012: debug fails here
	
	//GetDlgItem(IDC_STATIC21)->SetFont(&font);
	//GetDlgItem(IDC_STATIC20)->SetFont(&font);
	//GetDlgItem(IDC_STATIC19)->SetFont(&font);
	GetDlgItem(IDC_STATIC16)->SetFont(&font);
	GetDlgItem(IDC_STATIC18)->SetFont(&font);
	GetDlgItem(IDC_STATIC15)->SetFont(&font);
	GetDlgItem(IDC_STATIC17)->SetFont(&font);
	//GetDlgItem(IDC_STATIC)->SetFont(&font);

	m_SliderCtrl.SetRange(0, VOLUME_TIC_FREQUENCY);
	m_SliderCtrl.SetTicFreq(VOLUME_TIC_FREQUENCY);
	m_SliderCtrl.SetPos(volume);

	//dx nov 09, 2012
	pVolumeText = new CStatic;

	//get the position of the text beside of which this is to be displayed
	CRect rect;
	CWnd *pWnd = GetDlgItem(IDC_STATIC_VOLUME);
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

	pVolumeText->Create(convertVolume2StaticText(), style, rect, this);
	pVolumeText->SetFont(&m_font);	

	//dx nov 10, 2012
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", wetTime);
	GetDlgItem(IDC_STATIC_wetTime)->SetWindowTextA(ctrs_staticText);
	ctrs_staticText.Format("%dsec", soapTime);
	GetDlgItem(IDC_STATIC_soapTime)->SetWindowTextA(ctrs_staticText);
	ctrs_staticText.Format("%dsec", dryTime);
	GetDlgItem(IDC_STATIC_dryTime)->SetWindowTextA(ctrs_staticText);
	ctrs_staticText.Format("%dsec", scrubTime);
	GetDlgItem(IDC_STATIC_scrubTime)->SetWindowTextA(ctrs_staticText);
	ctrs_staticText.Format("%dsec", rinseTime);
	GetDlgItem(IDC_STATIC_rinseTime)->SetWindowTextA(ctrs_staticText);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BOOL IDDUserSettings3::OnClose()
{
	CDialog::OnClose();
	delete pVolumeText;
	pVolumeText = NULL;
	return true;
}


BEGIN_MESSAGE_MAP(IDDUserSettings3, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PREV, &IDDUserSettings3::OnBnClickedButtonPrev)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &IDDUserSettings3::OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_BUTTON33, &IDDUserSettings3::OnBnClickedButton33)
	ON_BN_CLICKED(IDC_BUTTON34, &IDDUserSettings3::OnBnClickedButton34)
	ON_BN_CLICKED(IDC_BUTTON35, &IDDUserSettings3::OnBnClickedButton35)
	ON_BN_CLICKED(IDC_BUTTON36, &IDDUserSettings3::OnBnClickedButton36)
	ON_BN_CLICKED(IDC_BUTTON37, &IDDUserSettings3::OnBnClickedButton37)
	ON_BN_CLICKED(IDC_BUTTON38, &IDDUserSettings3::OnBnClickedButton38)
//	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &IDDUserSettings3::OnNMCustomdrawSlider1)
	ON_BN_CLICKED(IDC_BUTTON39, &IDDUserSettings3::OnBnClickedButton39)
	ON_BN_CLICKED(IDC_BUTTON40, &IDDUserSettings3::OnBnClickedButton40)
	ON_BN_CLICKED(IDC_BUTTON41, &IDDUserSettings3::OnBnClickedButton41)
	ON_BN_CLICKED(IDC_BUTTON42, &IDDUserSettings3::OnBnClickedButton42)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, &IDDUserSettings3::OnSliderChangeMouseRelease)
	ON_BN_CLICKED(IDC_BUTTON1, &IDDUserSettings3::OnBnClickedButton1)
END_MESSAGE_MAP()


// IDDUserSettings3 message handlers


void IDDUserSettings3::OnBnClickedButtonPrev()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	EndDialog(IDC_BUTTON_PREV);
}


void IDDUserSettings3::OnBnClickedButtonNext()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	EndDialog(IDC_BUTTON_NEXT);
}


void IDDUserSettings3::OnBnClickedButton33()
{
	// TODO: Add your control notification handler code here	
	UpdateData(true);
	scrubTime++;
	if (scrubTime > 20)
		scrubTime = 20;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", scrubTime);
	GetDlgItem(IDC_STATIC_scrubTime)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings3::OnBnClickedButton34()
{
	// TODO: Add your control notification handler code here	
	UpdateData(true);
	scrubTime--;
	if (scrubTime < 0)
		scrubTime = 0;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", scrubTime);
	GetDlgItem(IDC_STATIC_scrubTime)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings3::OnBnClickedButton35()
{
	// TODO: Add your control notification handler code here	
	UpdateData(true);
	rinseTime++;
	if (rinseTime > 20)
		rinseTime = 20;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", rinseTime);
	GetDlgItem(IDC_STATIC_rinseTime)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings3::OnBnClickedButton36()
{
	// TODO: Add your control notification handler code here	
	UpdateData(true);
	rinseTime--;
	if (rinseTime < 0)
		rinseTime = 0;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", rinseTime);
	GetDlgItem(IDC_STATIC_rinseTime)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings3::OnBnClickedButton37()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	wetTime++;
	if (wetTime > 5)
		wetTime = 5;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", wetTime);
	GetDlgItem(IDC_STATIC_wetTime)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings3::OnBnClickedButton38()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	wetTime--;
	if (wetTime < 0)
		wetTime = 0;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", wetTime);
	GetDlgItem(IDC_STATIC_wetTime)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings3::OnBnClickedButton39()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	soapTime++;
	if (soapTime > 5)
		soapTime = 5;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", soapTime);
	GetDlgItem(IDC_STATIC_soapTime)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings3::OnBnClickedButton40()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	soapTime--;
	if (soapTime < 0)
		soapTime = 0;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", soapTime);
	GetDlgItem(IDC_STATIC_soapTime)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings3::OnBnClickedButton41()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	dryTime++;
	if (dryTime > 5)
		dryTime = 5;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", dryTime);
	GetDlgItem(IDC_STATIC_dryTime)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings3::OnBnClickedButton42()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	dryTime--;
	if (dryTime < 0)
		dryTime = 0;
	UpdateData(false);
	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", dryTime);
	GetDlgItem(IDC_STATIC_dryTime)->SetWindowTextA(ctrs_staticText);
}


void IDDUserSettings3::OnSliderChangeMouseRelease(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	int oldVolume = volume;
	UpdateData(true);
	if (oldVolume != volume) //play a ding when volume is changed as audio feedback of loudness
	{
		bool result = ChangeVolume( (double)volume/VOLUME_TIC_FREQUENCY, true);
		assert(!result); //the function ChangeVolume returns false if nothing is interrupted
		pVolumeText->SetWindowTextA(convertVolume2StaticText());
		PlaySound("C:\\WINDOWS\\Media\\ding.wav", NULL, SND_FILENAME | SND_ASYNC);
	}
}

//bScalar false if nVolume is in decibels, bScalar true if nVolume in percentage
bool IDDUserSettings3::ChangeVolume(double nVolume,bool bScalar)
{	
	//http://www.codeproject.com/Tips/233484/Change-Master-Volume-in-Visual-Cplusplus
	
	if (bScalar)
		assert(nVolume<=1 && nVolume>=0);

    HRESULT hr=NULL;
    bool decibels = false;
    bool scalar = false;
    double newVolume=nVolume;
 
    CoInitialize(NULL);
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, 
                          __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    IMMDevice *defaultDevice = NULL;
 
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    deviceEnumerator->Release();
    deviceEnumerator = NULL;
 
    IAudioEndpointVolume *endpointVolume = NULL;
    hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), 
         CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
    defaultDevice->Release();
    defaultDevice = NULL;
 
    // -------------------------
    float currentVolume = 0;
    endpointVolume->GetMasterVolumeLevel(&currentVolume);
    //printf("Current volume in dB is: %f\n", currentVolume);

    hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
    //CString strCur=L"";
    //strCur.Format(L"%f",currentVolume);
    //AfxMessageBox(strCur);

    // printf("Current volume as a scalar is: %f\n", currentVolume);
    if (bScalar==false)
    { //setting by percentage
        hr = endpointVolume->SetMasterVolumeLevel((float)newVolume, NULL);
    }
    else if (bScalar==true)
    { //setting by decibels
        hr = endpointVolume->SetMasterVolumeLevelScalar((float)newVolume, NULL);
    }
    endpointVolume->Release();
 
    CoUninitialize();
 
    return FALSE;
}

CString IDDUserSettings3::convertVolume2StaticText()
{
	//dx debug nov 09, 2012
	//double a = ((double)volume/VOLUME_TIC_FREQUENCY)*100;
	//int b = (int) (((double)volume/VOLUME_TIC_FREQUENCY)*100);
	CString volumeStr;
	volumeStr.Format(" %d%%", (int) ((double)volume/VOLUME_TIC_FREQUENCY*100));	
	return volumeStr;
}

void IDDUserSettings3::OnBnClickedButton1()
{
	if(AfxMessageBox("Are you sure to reset to default settings?", MB_YESNO) == IDYES)
	{
		//reset settings to default

		pSettingDlg1->levelFivePrompt = 4;
		pSettingDlg1->levelFourPrompt = 3;
		pSettingDlg1->levelThreePrompt = 2;
		pSettingDlg1->levelTwoPrompt = 1;
		pSettingDlg1->levelOnePrompt = 0;
		pSettingDlg1->nPromptsFromSettings2 = 5;

		pSettingDlg2->nPrompts = 5;	
		pSettingDlg2->firstPromptTime = 5;
		pSettingDlg2->nextPromptTime = 3;
		pSettingDlg2->priming = 0;
		pSettingDlg2->use_attention_grabbers = 2;
		pSettingDlg2->rewardallsteps = 0;

		useCountdown = 0;
		soapTime = 1;
		wetTime = 2;
		scrubTime = 10;
		rinseTime = 8;
		dryTime = 1;
		//note we don't restore the volume here

		//close dialog
		EndDialog(IDOK);
	}

	//update the UI screen:
	
	UpdateData(false);

	CString ctrs_staticText;
	ctrs_staticText.Format("%dsec", wetTime);
	GetDlgItem(IDC_STATIC_wetTime)->SetWindowTextA(ctrs_staticText);
	ctrs_staticText.Format("%dsec", soapTime);
	GetDlgItem(IDC_STATIC_soapTime)->SetWindowTextA(ctrs_staticText);
	ctrs_staticText.Format("%dsec", dryTime);
	GetDlgItem(IDC_STATIC_dryTime)->SetWindowTextA(ctrs_staticText);
	ctrs_staticText.Format("%dsec", scrubTime);
	GetDlgItem(IDC_STATIC_scrubTime)->SetWindowTextA(ctrs_staticText);
	ctrs_staticText.Format("%dsec", rinseTime);
	GetDlgItem(IDC_STATIC_rinseTime)->SetWindowTextA(ctrs_staticText);
}
