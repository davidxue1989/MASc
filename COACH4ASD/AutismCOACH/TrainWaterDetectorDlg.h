#pragma once


// CTrainWaterDetectorDlg dialog

class CTrainWaterDetectorDlg : public CDialog
{
	DECLARE_DYNAMIC(CTrainWaterDetectorDlg)

public:
	CTrainWaterDetectorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTrainWaterDetectorDlg();

// Dialog Data
	enum { IDD = IDD_TRAINWATERDETECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int waterDetectionCategory;
	bool userReadyToContinue;
	afx_msg void OnBnClickedContinue();
};
