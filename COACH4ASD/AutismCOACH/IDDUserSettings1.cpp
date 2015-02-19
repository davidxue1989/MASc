// IDDUserSettings1.cpp : implementation file
//

#include "stdafx.h"
#include "AutismCOACH.h"
#include "IDDUserSettings1.h"
#include "afxdialogex.h"

// IDDUserSettings1 dialog

IMPLEMENT_DYNAMIC(IDDUserSettings1, CDialog)

IDDUserSettings1::IDDUserSettings1(CWnd* pParent /*=NULL*/)
	: CDialog(IDDUserSettings1::IDD, pParent)
{
	levelOnePrompt = 0;
	levelTwoPrompt = 0;
	levelThreePrompt = 0;
	levelFourPrompt = 0;
	levelFivePrompt = 0;
	nPromptsFromSettings2 = 0;
}

IDDUserSettings1::~IDDUserSettings1()
{
}

void IDDUserSettings1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_RADIO2, levelOnePrompt);
	DDX_Radio(pDX, IDC_RADIO7, levelTwoPrompt);
	DDX_Radio(pDX, IDC_RADIO16, levelThreePrompt);
	DDX_Radio(pDX, IDC_RADIO52, levelFourPrompt);
	DDX_Radio(pDX, IDC_RADIO57, levelFivePrompt);	
}


BOOL IDDUserSettings1::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// disable IDPREV, enable IDNEXT
	//this->GetDlgItem(IDC_BUTTON_PREV)->EnableWindow(FALSE);
	this->GetDlgItem(IDC_BUTTON_PREV)->EnableWindow(TRUE);
	this->GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(TRUE);

	//CFont font; //dxnote oct 28, 2012: the font variable has to be persisting, i.e. a member variable to the dialog, else it won't be displayed properly
	font.DeleteObject();
	font.CreateFontA(30, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");//dxtodo sep 18 ,2012: debug fails here

	GetDlgItem(IDC_STATIC)->SetFont(&font);
	GetDlgItem(IDC_STATIC6)->SetFont(&font);
	GetDlgItem(IDC_STATIC7)->SetFont(&font);
	GetDlgItem(IDC_STATIC8)->SetFont(&font);
	GetDlgItem(IDC_STATIC9)->SetFont(&font);

	//dx nov 09, 2012:
	//saves the normal and bold fonts for the radio buttons	
	pfont_notBold = GetDlgItem(IDC_RADIO2)->GetFont();
	//change it to bold
	LOGFONT lf; 
	pfont_notBold->GetLogFont(&lf);
	lf.lfWeight = lf.lfWeight*2;
	pfont_Bold = new CFont;
	//pfont_Bold->DeleteObject(); //dxnote oct 28, 2012: need to delete the font first before creating a new one
	pfont_Bold->CreateFontIndirect(&lf);

	//setting the selected prompt types bold
	bool Bolded[5];
	
	int IDs1[] = {IDC_RADIO2, IDC_RADIO3, IDC_RADIO4, IDC_RADIO5, IDC_RADIO6};
	for (int i=0; i<5; i++)
	{
		Bolded[i] = (i == levelOnePrompt);
	}	
	setItemsBold(IDs1, 5, Bolded, 5);
	
	int IDs2[] = {IDC_RADIO7, IDC_RADIO8, IDC_RADIO13, IDC_RADIO14, IDC_RADIO15};
	for (int i=0; i<5; i++)
	{
		Bolded[i] = (i == levelTwoPrompt);
	}
	setItemsBold(IDs2, 5, Bolded, 5);
	
	int IDs3[] = {IDC_RADIO16, IDC_RADIO17, IDC_RADIO18, IDC_RADIO50, IDC_RADIO51};
	for (int i=0; i<5; i++)
	{
		Bolded[i] = (i == levelThreePrompt);
	}
	setItemsBold(IDs3, 5, Bolded, 5);
	
	int IDs4[] = {IDC_RADIO52, IDC_RADIO53, IDC_RADIO54, IDC_RADIO55, IDC_RADIO56};
	for (int i=0; i<5; i++)
	{
		Bolded[i] = (i == levelFourPrompt);
	}
	setItemsBold(IDs4, 5, Bolded, 5);
	
	int IDs5[] = {IDC_RADIO57, IDC_RADIO58, IDC_RADIO59, IDC_RADIO60, IDC_RADIO61};
	for (int i=0; i<5; i++)
	{
		Bolded[i] = (i == levelFivePrompt);
	}
	setItemsBold(IDs5, 5, Bolded, 5);

	//dxnote oct 28, 2012: disabling the prompt settings for options exceeding the nPrompts chosen
	if (nPromptsFromSettings2 < 5)
	{
		GetDlgItem(IDC_RADIO57)->EnableWindow(0);
		GetDlgItem(IDC_RADIO58)->EnableWindow(0);
		GetDlgItem(IDC_RADIO59)->EnableWindow(0);
		GetDlgItem(IDC_RADIO60)->EnableWindow(0);
		GetDlgItem(IDC_RADIO61)->EnableWindow(0);
	}
	if (nPromptsFromSettings2 < 4)
	{
		GetDlgItem(IDC_RADIO52)->EnableWindow(0);
		GetDlgItem(IDC_RADIO53)->EnableWindow(0);
		GetDlgItem(IDC_RADIO54)->EnableWindow(0);
		GetDlgItem(IDC_RADIO55)->EnableWindow(0);
		GetDlgItem(IDC_RADIO56)->EnableWindow(0);
	}
	if (nPromptsFromSettings2 < 3)
	{
		GetDlgItem(IDC_RADIO16)->EnableWindow(0);
		GetDlgItem(IDC_RADIO17)->EnableWindow(0);
		GetDlgItem(IDC_RADIO18)->EnableWindow(0);
		GetDlgItem(IDC_RADIO50)->EnableWindow(0);
		GetDlgItem(IDC_RADIO51)->EnableWindow(0);
	}
	if (nPromptsFromSettings2 < 2)
	{
		GetDlgItem(IDC_RADIO7)->EnableWindow(0);
		GetDlgItem(IDC_RADIO8)->EnableWindow(0);
		GetDlgItem(IDC_RADIO13)->EnableWindow(0);
		GetDlgItem(IDC_RADIO14)->EnableWindow(0);
		GetDlgItem(IDC_RADIO15)->EnableWindow(0);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL IDDUserSettings1::OnClose()
{
	CDialog::OnClose();
	delete pfont_Bold;
	pfont_Bold = NULL;
	return true;
}


BEGIN_MESSAGE_MAP(IDDUserSettings1, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PREV, &IDDUserSettings1::OnBnClickedButtonPrev)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &IDDUserSettings1::OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_RADIO2, &IDDUserSettings1::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &IDDUserSettings1::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, &IDDUserSettings1::OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_RADIO5, &IDDUserSettings1::OnBnClickedRadio5)
	ON_BN_CLICKED(IDC_RADIO6, &IDDUserSettings1::OnBnClickedRadio6)
	ON_BN_CLICKED(IDC_RADIO7, &IDDUserSettings1::OnBnClickedRadio7)
	ON_BN_CLICKED(IDC_RADIO8, &IDDUserSettings1::OnBnClickedRadio8)
	ON_BN_CLICKED(IDC_RADIO13, &IDDUserSettings1::OnBnClickedRadio13)
	ON_BN_CLICKED(IDC_RADIO14, &IDDUserSettings1::OnBnClickedRadio14)
	ON_BN_CLICKED(IDC_RADIO15, &IDDUserSettings1::OnBnClickedRadio15)
	ON_BN_CLICKED(IDC_RADIO16, &IDDUserSettings1::OnBnClickedRadio16)
	ON_BN_CLICKED(IDC_RADIO17, &IDDUserSettings1::OnBnClickedRadio17)
	ON_BN_CLICKED(IDC_RADIO18, &IDDUserSettings1::OnBnClickedRadio18)
	ON_BN_CLICKED(IDC_RADIO50, &IDDUserSettings1::OnBnClickedRadio50)
	ON_BN_CLICKED(IDC_RADIO51, &IDDUserSettings1::OnBnClickedRadio51)
	ON_BN_CLICKED(IDC_RADIO52, &IDDUserSettings1::OnBnClickedRadio52)
	ON_BN_CLICKED(IDC_RADIO53, &IDDUserSettings1::OnBnClickedRadio53)
	ON_BN_CLICKED(IDC_RADIO54, &IDDUserSettings1::OnBnClickedRadio54)
	ON_BN_CLICKED(IDC_RADIO55, &IDDUserSettings1::OnBnClickedRadio55)
	ON_BN_CLICKED(IDC_RADIO56, &IDDUserSettings1::OnBnClickedRadio56)
	ON_BN_CLICKED(IDC_RADIO57, &IDDUserSettings1::OnBnClickedRadio57)
	ON_BN_CLICKED(IDC_RADIO58, &IDDUserSettings1::OnBnClickedRadio58)
	ON_BN_CLICKED(IDC_RADIO59, &IDDUserSettings1::OnBnClickedRadio59)
	ON_BN_CLICKED(IDC_RADIO60, &IDDUserSettings1::OnBnClickedRadio60)
	ON_BN_CLICKED(IDC_RADIO61, &IDDUserSettings1::OnBnClickedRadio61)
END_MESSAGE_MAP()


// IDDUserSettings1 message handlers


void IDDUserSettings1::OnBnClickedButtonPrev()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	EndDialog(IDC_BUTTON_PREV);
}


void IDDUserSettings1::OnBnClickedButtonNext()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	EndDialog(IDC_BUTTON_NEXT);
}

void IDDUserSettings1::setItemsBold(int IDs[], int nIDs, bool Bolded[], int nBolded)
{
	assert( nIDs == nBolded);

	for (int i=0; i<nIDs; i++)
	{
		if (Bolded[i])
			GetDlgItem(IDs[i])->SetFont(pfont_Bold);
		else
			GetDlgItem(IDs[i])->SetFont(pfont_notBold);
	}
}

void IDDUserSettings1::OnBnClickedRadio2()
{
	int IDs[] = {IDC_RADIO2, IDC_RADIO3, IDC_RADIO4, IDC_RADIO5, IDC_RADIO6};
	bool bolded[] = {true, false, false, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio3()
{	
	int IDs[] = {IDC_RADIO2, IDC_RADIO3, IDC_RADIO4, IDC_RADIO5, IDC_RADIO6};
	bool bolded[] = {false, true, false, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio4()
{
	int IDs[] = {IDC_RADIO2, IDC_RADIO3, IDC_RADIO4, IDC_RADIO5, IDC_RADIO6};
	bool bolded[] = {false, false, true, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio5()
{
	int IDs[] = {IDC_RADIO2, IDC_RADIO3, IDC_RADIO4, IDC_RADIO5, IDC_RADIO6};
	bool bolded[] = {false, false, false, true, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio6()
{
	int IDs[] = {IDC_RADIO2, IDC_RADIO3, IDC_RADIO4, IDC_RADIO5, IDC_RADIO6};
	bool bolded[] = {false, false, false, false, true};
	setItemsBold(IDs, 5, bolded, 5);
}


void IDDUserSettings1::OnBnClickedRadio7()
{
	int IDs[] = {IDC_RADIO7, IDC_RADIO8, IDC_RADIO13, IDC_RADIO14, IDC_RADIO15};
	bool bolded[] = {true, false, false, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio8()
{
	int IDs[] = {IDC_RADIO7, IDC_RADIO8, IDC_RADIO13, IDC_RADIO14, IDC_RADIO15};
	bool bolded[] = {false, true, false, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio13()
{
	int IDs[] = {IDC_RADIO7, IDC_RADIO8, IDC_RADIO13, IDC_RADIO14, IDC_RADIO15};
	bool bolded[] = {false, false, true, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio14()
{
	int IDs[] = {IDC_RADIO7, IDC_RADIO8, IDC_RADIO13, IDC_RADIO14, IDC_RADIO15};
	bool bolded[] = {false, false, false, true, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio15()
{
	int IDs[] = {IDC_RADIO7, IDC_RADIO8, IDC_RADIO13, IDC_RADIO14, IDC_RADIO15};
	bool bolded[] = {false, false, false, false, true};
	setItemsBold(IDs, 5, bolded, 5);
}


void IDDUserSettings1::OnBnClickedRadio16()
{
	int IDs[] = {IDC_RADIO16, IDC_RADIO17, IDC_RADIO18, IDC_RADIO50, IDC_RADIO51};
	bool bolded[] = {true, false, false, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio17()
{
	int IDs[] = {IDC_RADIO16, IDC_RADIO17, IDC_RADIO18, IDC_RADIO50, IDC_RADIO51};
	bool bolded[] = {false, true, false, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio18()
{
	int IDs[] = {IDC_RADIO16, IDC_RADIO17, IDC_RADIO18, IDC_RADIO50, IDC_RADIO51};
	bool bolded[] = {false, false, true, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio50()
{
	int IDs[] = {IDC_RADIO16, IDC_RADIO17, IDC_RADIO18, IDC_RADIO50, IDC_RADIO51};
	bool bolded[] = {false, false, false, true, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio51()
{
	int IDs[] = {IDC_RADIO16, IDC_RADIO17, IDC_RADIO18, IDC_RADIO50, IDC_RADIO51};
	bool bolded[] = {false, false, false, false, true};
	setItemsBold(IDs, 5, bolded, 5);
}


void IDDUserSettings1::OnBnClickedRadio52()
{
	int IDs[] = {IDC_RADIO52, IDC_RADIO53, IDC_RADIO54, IDC_RADIO55, IDC_RADIO56};
	bool bolded[] = {true, false, false, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio53()
{
	int IDs[] = {IDC_RADIO52, IDC_RADIO53, IDC_RADIO54, IDC_RADIO55, IDC_RADIO56};
	bool bolded[] = {false, true, false, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio54()
{
	int IDs[] = {IDC_RADIO52, IDC_RADIO53, IDC_RADIO54, IDC_RADIO55, IDC_RADIO56};
	bool bolded[] = {false, false, true, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio55()
{
	int IDs[] = {IDC_RADIO52, IDC_RADIO53, IDC_RADIO54, IDC_RADIO55, IDC_RADIO56};
	bool bolded[] = {false, false, false, true, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio56()
{
	int IDs[] = {IDC_RADIO52, IDC_RADIO53, IDC_RADIO54, IDC_RADIO55, IDC_RADIO56};
	bool bolded[] = {false, false, false, false, true};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio57()
{
	int IDs[] = {IDC_RADIO57, IDC_RADIO58, IDC_RADIO59, IDC_RADIO60, IDC_RADIO61};
	bool bolded[] = {true, false, false, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio58()
{
	int IDs[] = {IDC_RADIO57, IDC_RADIO58, IDC_RADIO59, IDC_RADIO60, IDC_RADIO61};
	bool bolded[] = {false, true, false, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio59()
{
	int IDs[] = {IDC_RADIO57, IDC_RADIO58, IDC_RADIO59, IDC_RADIO60, IDC_RADIO61};
	bool bolded[] = {false, false, true, false, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio60()
{
	int IDs[] = {IDC_RADIO57, IDC_RADIO58, IDC_RADIO59, IDC_RADIO60, IDC_RADIO61};
	bool bolded[] = {false, false, false, true, false};
	setItemsBold(IDs, 5, bolded, 5);
}
void IDDUserSettings1::OnBnClickedRadio61()
{
	int IDs[] = {IDC_RADIO57, IDC_RADIO58, IDC_RADIO59, IDC_RADIO60, IDC_RADIO61};
	bool bolded[] = {false, false, false, false, true};
	setItemsBold(IDs, 5, bolded, 5);
}
