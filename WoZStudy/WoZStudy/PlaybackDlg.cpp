//http://www.codeproject.com/Questions/516017/Howplustoplusdiaplayplustheplusvideopluswithinplus
//also good: http://www.flipcode.com/archives/DirectShow_For_Media_Playback_In_Windows-Part_II_DirectShow_In_C.shtml

// PlaybackDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WoZStudy.h"
#include "PlaybackDlg.h"
#include "afxdialogex.h"


// PlaybackDlg dialog

IMPLEMENT_DYNAMIC(PlaybackDlg, CDialogEx)

PlaybackDlg::PlaybackDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(PlaybackDlg::IDD, pParent)
{
}

PlaybackDlg::~PlaybackDlg()
{
}

void PlaybackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PlaybackDlg, CDialogEx)
	//ON_WM_PAINT()
END_MESSAGE_MAP()


// PlaybackDlg message handlers

void PlaybackDlg::Play(std::wstring filename)
{
	stopNow = false;
	isPlaying = true;
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		MessageBox(L"ERROR - Could not initialize COM library");
		return;
	}

	// Create the filter graph manager and query for interfaces.
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **) &pGraph);
	if (FAILED(hr))
	{
		MessageBox(L"ERROR - Could not create the Filter Graph Manager.");
		return;
	}

	hr = pGraph->QueryInterface(IID_IMediaControl, (void **) &pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **) &pEvent);

	hr = InitWindowlessVMR(AfxGetApp()->GetMainWnd()->m_hWnd, pGraph, &pWc);
	hr = pGraph->RenderFile(filename.c_str(), NULL);
	long lWidth, lHeight;
	hr = pWc->GetNativeVideoSize(&lWidth, &lHeight, NULL, NULL);
	if (SUCCEEDED(hr))
	{
		RECT rcSrc, rcDest;
		// Set the source rectangle.
		SetRect(&rcSrc, 0, 0, lWidth, lHeight);

		// Get the window client area.
		//GetClientRect(&rcDest);
		//c->GetClientRect(&rcDest);
		rcDest = rcSrc;
		// Set the destination rectangle.
		SetRect(&rcDest, 0, 1300, rcDest.right, 1300 + rcDest.bottom);

		// Set the video position.
		hr = pWc->SetVideoPosition(&rcSrc, &rcDest);
	}

	if (SUCCEEDED(hr))
	{

		// Run the graph.
		hr = pControl->Run();


		//use wait for multiple objects: 1. handle to media event 2. handle to stop video command

		//if (SUCCEEDED(hr))
		//{
		//	// Wait for completion.
		//	long evCode = -1;
		//	while (evCode != 1) {
		//		if (stopNow) {
		//			pControl->Stop();
		//			break;
		//		}
		//		pEvent->WaitForCompletion(200, &evCode);
		//	}

		//	// Note: Do not use INFINITE in a real application, because it
		//	// can block indefinitely.
		//}
	}

	pWc->Release();
	pControl->Release();
	pGraph->Release();
	pEvent->Release();
	CoUninitialize();

	isPlaying = false;
}

void PlaybackDlg::Stop() {
	stopNow = true;
}

HRESULT PlaybackDlg::InitWindowlessVMR(
	HWND hwndApp,                  // Window to hold the video. 
	IGraphBuilder* pGraph,         // Pointer to the Filter Graph Manager. 
	IVMRWindowlessControl** ppWc   // Receives a pointer to the VMR.
	)
{
	if (!pGraph || !ppWc)
	{
		return E_POINTER;
	}
	IBaseFilter* pVmr = NULL;
	IVMRWindowlessControl* pWc = NULL;
	// Create the VMR. 
	HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL,
		CLSCTX_INPROC, IID_IBaseFilter, (void**) &pVmr);
	if (FAILED(hr))
	{
		return hr;
	}

	// Add the VMR to the filter graph.
	hr = pGraph->AddFilter(pVmr, L"Video Mixing Renderer");
	if (FAILED(hr))
	{
		pVmr->Release();
		return hr;
	}
	// Set the rendering mode.  
	IVMRFilterConfig* pConfig;
	hr = pVmr->QueryInterface(IID_IVMRFilterConfig, (void**) &pConfig);
	if (SUCCEEDED(hr))
	{
		hr = pConfig->SetRenderingMode(VMRMode_Windowless);
		pConfig->Release();
	}
	if (SUCCEEDED(hr))
	{
		// Set the window. 
		hr = pVmr->QueryInterface(IID_IVMRWindowlessControl, (void**) &pWc);
		if (SUCCEEDED(hr))
		{
			hr = pWc->SetVideoClippingWindow(hwndApp);
			if (SUCCEEDED(hr))
			{
				*ppWc = pWc; // Return this as an AddRef'd pointer. 
			}
			else
			{
				// An error occurred, so release the interface.
				pWc->Release();
			}
		}
	}
	pVmr->Release();
	return hr;
}


//// If you add a minimize button to your dialog, you will need the code below
////  to draw the icon.  For MFC applications using the document/view model,
////  this is automatically done for you by the framework.
//
//void PlaybackDlg::OnPaint()
//{
//	if (IsIconic())
//	{
//		CPaintDC dc(this); // device context for painting
//		RECT        rcClient;
//		GetClientRect(&rcClient);
//		if (pWc != NULL)
//		{
//			// Find the region where the application can paint by subtracting 
//			// the video destination rectangle from the client area.
//			// (Assume that g_rcDest was calculated previously.)
//			HRGN rgnClient = CreateRectRgnIndirect(&rcClient);
//			HRGN rgnVideo = CreateRectRgnIndirect(&rcDest);
//			CombineRgn(rgnClient, rgnClient, rgnVideo, RGN_DIFF);
//
//			// Paint on window.
//			HBRUSH hbr = GetSysColorBrush(COLOR_BTNFACE);
//			FillRgn(dc.m_hDC, rgnClient, hbr);
//
//			// Clean up.
//			DeleteObject(hbr);
//			DeleteObject(rgnClient);
//			DeleteObject(rgnVideo);
//
//			// Request the VMR to paint the video.
//			HRESULT hr = pWc->RepaintVideo(hwnd, dc.m_hDC);
//		}
//		else  // There is no video, so paint the whole client area. 
//		{
//			FillRect(dc.m_hDC, &rcClient, (HBRUSH) (COLOR_BTNFACE + 1));
//		}
//		//-----------------------------------------------------------------------------------------------------------//
//		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<wparam>(dc.GetSafeHdc()), 0);
//
//		// Center icon in client rectangle
//		int cxIcon = GetSystemMetrics(SM_CXICON);
//		int cyIcon = GetSystemMetrics(SM_CYICON);
//		CRect rect;
//		GetClientRect(&rect);
//		int x = (rect.Width() - cxIcon + 1) / 2;
//		int y = (rect.Height() - cyIcon + 1) / 2;
//
//		// Draw the icon
//		dc.DrawIcon(x, y, m_hIcon);
//	}
//	else
//	{
//		CDialog::OnPaint();
//	}
//	//-------------------------------------------------------------------------------------------------------//
//}
