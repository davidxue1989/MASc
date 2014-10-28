//http://www.codeproject.com/Questions/516017/Howplustoplusdiaplayplustheplusvideopluswithinplus
#pragma comment(lib,"Strmiids.lib")
#pragma once

#include <dshow.h>
#include <string>


// PlaybackDlg dialog

class PlaybackDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PlaybackDlg)

public:
	PlaybackDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~PlaybackDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


	// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//afx_msg void OnPaint();
public:
	void Play(std::wstring filename);
	IGraphBuilder *pGraph;
	IMediaControl *pControl;
	IMediaEventEx   *pEvent;
	IVMRWindowlessControl *pWc;
	RECT rcSrc, rcDest;
	HWND hwnd;
	HRESULT InitWindowlessVMR(
		HWND hwndApp,                  // Window to hold the video. 
		IGraphBuilder* pGraph,         // Pointer to the Filter Graph Manager. 
		IVMRWindowlessControl** ppWc   // Receives a pointer to the VMR.
		);



	DECLARE_MESSAGE_MAP()
};