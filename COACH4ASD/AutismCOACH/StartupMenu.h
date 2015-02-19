#pragma once


// CStartupMenu dialog

class CStartupMenu : public CDialog
{
	DECLARE_DYNAMIC(CStartupMenu)

public:
	CStartupMenu(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStartupMenu();

// Dialog Data
	enum { IDD = IDD_STARTUPMENU };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CDocument* docPointer;//Dec. 12, 11

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedEnterUserSettings();
	void SetPointerToDoc(CDocument* pDoc);
	afx_msg void OnBnClickedStartPrompting();
};
