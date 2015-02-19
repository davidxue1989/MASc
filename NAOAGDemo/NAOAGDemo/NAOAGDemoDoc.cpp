
// NAOAGDemoDoc.cpp : implementation of the CNAOAGDemoDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "NAOAGDemo.h"
#endif

#include "NAOAGDemoDoc.h"


#include "NAOAGDemoView.h"

#include <iostream>
#include <fstream>
using namespace std;

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CNAOAGDemoDoc

IMPLEMENT_DYNCREATE(CNAOAGDemoDoc, CDocument)

BEGIN_MESSAGE_MAP(CNAOAGDemoDoc, CDocument)
END_MESSAGE_MAP()


// CNAOAGDemoDoc construction/destruction

CNAOAGDemoDoc::CNAOAGDemoDoc()
{
}

CNAOAGDemoDoc::~CNAOAGDemoDoc()
{
}

BOOL CNAOAGDemoDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	POSITION posV = GetFirstViewPosition();
	CNAOAGDemoView * view = (CNAOAGDemoView *) GetNextView(posV);
	ASSERT_VALID(view);
	if (!view) {
		AfxMessageBox(_T("Failed to get view!"));
		exit(1);
	}

	NUI_IMAGE_TYPE m_depthType(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX);
	NUI_IMAGE_TYPE m_colorType(NUI_IMAGE_TYPE_COLOR);
	NUI_IMAGE_RESOLUTION m_depthRes(NUI_IMAGE_RESOLUTION_320x240);
	NUI_IMAGE_RESOLUTION m_colorRes(NUI_IMAGE_RESOLUTION_640x480);
	BOOL m_bNearMode = TRUE;
	BOOL m_bSeatedSkeletonMode = TRUE;
	gaze = gNone;
	if (!SUCCEEDED(m_FTHelper.Init(view->GetParentFrame()->m_hWnd,
		FTHelperCallingBack,
		this,
		FTHelperCallingBackTrackLost,
		this,
		m_depthType,
		m_depthRes,
		m_bNearMode,
		TRUE, // if near mode doesn't work, fall back to default mode
		m_colorType,
		m_colorRes,
		m_bSeatedSkeletonMode))){
			AfxMessageBox(_T("Error: m_FTHelper failed to init!"));
			exit(1);
	}

	view->SetTimer(TIMER_ID_AGDEMO, 1000, 0);

	return TRUE;
}


void CNAOAGDemoDoc::FTHelperCallingBack(PVOID pVoid) {
	CNAOAGDemoDoc* pDoc = reinterpret_cast<CNAOAGDemoDoc*>(pVoid);
	if (pDoc)
	{
		IFTResult* pResult = pDoc->m_FTHelper.GetResult();
		if (pResult && SUCCEEDED(pResult->GetStatus()))
		{
			FLOAT scale;
			FLOAT rotationXYZ[3];
			FLOAT translationXYZ[3];
			pResult->Get3DPose(&scale, rotationXYZ, translationXYZ);

			//dx aug 25, 2013: saving the pose {translation and rotation} into file
			// format is: tran_x, trans_y, trans_z, direction_x, direction_y, direction_z all in units of m, and in camera's frame
			FLOAT tranx, trany, tranz, dirx, diry, dirz;
			tranx = translationXYZ[0];
			trany = translationXYZ[1];
			tranz = translationXYZ[2];
			/*
			direction unit vector:
			n = [0; 0; -1]
			head pose rotation matrix (doing Yaw (Ry), then Pitch (Rx), then Roll (Rz)):
			Rx =
			[ 1,          0,           0;
			0, cos(pitch), -sin(pitch);
			0, sin(pitch),  cos(pitch)]
			Ry =
			[  cos(yaw), 0, sin(yaw);
			0, 1,        0;
			-sin(yaw), 0, cos(yaw)]
			Rz =
			[ cos(roll), -sin(roll), 0;
			sin(roll),  cos(roll), 0;
			0,          0, 1]
			R = Rz*Rx*Ry =
			[ cos(roll)*cos(yaw) - sin(pitch)*sin(roll)*sin(yaw), -cos(pitch)*sin(roll), cos(roll)*sin(yaw) + cos(yaw)*sin(pitch)*sin(roll);
			cos(yaw)*sin(roll) + cos(roll)*sin(pitch)*sin(yaw),  cos(pitch)*cos(roll), sin(roll)*sin(yaw) - cos(roll)*cos(yaw)*sin(pitch);
			-cos(pitch)*sin(yaw),            sin(pitch),                                cos(pitch)*cos(yaw)]
			gaze direction unit vector:
			n' = R*n =
			[-cos(roll)*sin(yaw)-cos(yaw)*sin(pitch)*sin(roll);
			cos(roll)*cos(yaw)*sin(pitch)-sin(roll)*sin(yaw);
			-cos(pitch)*cos(yaw)]
			*/
			FLOAT pitch = rotationXYZ[0] / 180.0f * ((float) M_PI);
			FLOAT yaw = rotationXYZ[1] / 180.0f * ((float) M_PI);
			FLOAT roll = rotationXYZ[2] / 180.0f * ((float) M_PI);
			dirx = -cos(roll)*sin(yaw) - cos(yaw)*sin(pitch)*sin(roll);
			diry = cos(roll)*cos(yaw)*sin(pitch) - sin(roll)*sin(yaw);
			dirz = -cos(pitch)*cos(yaw);

			//save this into file
			ofstream f;
			f.open("pose.csv", ios::app);
			f << tranx << "," << trany << "," << tranz << "," << dirx << "," << diry << "," << dirz << endl;
			f.close();

			//determine if gazing at an object
			Vector M(VZERO); //mean of object location
			Matrix cov_inv(VZERO, VZERO, VZERO); //inverse of the covariance matrix of object location
			FLOAT zscore;
			Vector X(VZERO); //the position along the gaze direction that's closest to the mean of object location
			Vector P0(tranx, trany, tranz);
			Vector U(dirx, diry, dirz);

			//NAO's location
			M.set(-0.0172846365285260, -0.194743749426271, 1.41944377257685);
			cov_inv.set(Vector(513.858313685998, 273.104260265962, 612.460434105609), Vector(273.104260265962, 383.775818150305, 412.435213031405), Vector(612.460434105609, 412.435213031405, 809.058677957935));

			X = P0 + U*dot((M - P0), U);
			zscore = dot(X - M, cov_inv*(X - M));
			if (zscore < 50 ? TRUE : FALSE)
				pDoc->gaze = gNAO;

			//Obj's location
			M.set(0.493560450884594, -0.120304660629529, 0.884095166202588);
			cov_inv.set(Vector(2823.61382994240, -260.317616131125, -6281.25028006067)
				, Vector(-260.317616131125, 372.441990775953, 161.749157907919)
				, Vector(-6281.25028006067, 161.749157907919, 15701.1617207151));
			X = P0 + U*dot((M - P0), U);
			zscore = dot(X - M, cov_inv*(X - M));
			if (zscore < 50 ? TRUE : FALSE)
				pDoc->gaze = gObj;
		}
	}
}


void CNAOAGDemoDoc::FTHelperCallingBackTrackLost(PVOID pVoid) {
	CNAOAGDemoDoc* pDoc = reinterpret_cast<CNAOAGDemoDoc*>(pVoid);
	if (pDoc)
	{
		pDoc->gaze = gNone;
	}
}


// CNAOAGDemoDoc serialization

void CNAOAGDemoDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CNAOAGDemoDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CNAOAGDemoDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CNAOAGDemoDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CNAOAGDemoDoc diagnostics

#ifdef _DEBUG
void CNAOAGDemoDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CNAOAGDemoDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CNAOAGDemoDoc commands
