#pragma once
#include "resource.h"

// GazeDetector dialog

class Detector : public CDialogEx
{
	DECLARE_DYNAMIC(Detector)

public:
	Detector(CWnd* pParent, CString tag, CString msg);   // standard constructor
	virtual ~Detector();
	void start();
	void stop();
	bool isStarted() { return started; };
	bool isDetected() { return detected; }

	// Dialog Data
	enum { IDD = IDD_DETECTOR };

private:
	CString tag;
	CString msg;
	bool detected;
	bool started;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadiodetected();
	afx_msg void OnBnClickedRadionotdetected();
};
