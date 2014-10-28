#pragma once
#include <string>


// PROMPT_QUEUE_DLG dialog

class PROMPT_QUEUE_DLG : public CDialogEx
{
	DECLARE_DYNAMIC(PROMPT_QUEUE_DLG)

public:
	PROMPT_QUEUE_DLG(CWnd* pParent = NULL);   // standard constructor
	virtual ~PROMPT_QUEUE_DLG();

	void start();
	void updateDisplay(std::wstring	text);

// Dialog Data
	enum { IDD = IDD_DIALOG_PROMPT_QUEUE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
