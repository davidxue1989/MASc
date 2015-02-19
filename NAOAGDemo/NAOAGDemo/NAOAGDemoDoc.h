
// NAOAGDemoDoc.h : interface of the CNAOAGDemoDoc class
//


#pragma once

#include "FTHelper.h"
#include "Algebra3D.h"
using namespace Algebra3D;
#define VZERO	Vector(0, 0, 0)
#define _USE_MATH_DEFINES
#include <math.h>

#include "AG_NAO.h"

/* dxnote: this is the order of things being called during start up:
CNAOAGDemoApp::InitInstance
	CNAOAGDemoDoc::CNAOAGDemoDoc
	CMainFrame::CMainFrame
	CMainFrame::PreCreateWindow
	CMainFrame::OnCreate
		CNAOAGDemoView::CNAOAGDemoView
	CNAOAGDemoDoc::OnNewDocument
*/

class CNAOAGDemoDoc : public CDocument
{
public:
	static void FTHelperCallingBack(LPVOID lpParam);
	static void FTHelperCallingBackTrackLost(LPVOID lpParam);

	FTHelper m_FTHelper;
	enum GAZE { gNone, gNAO, gObj } gaze;

	//CNAOAGDemoView *view;
	AG_NAO agNAO;



protected: // create from serialization only
	CNAOAGDemoDoc();
	DECLARE_DYNCREATE(CNAOAGDemoDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CNAOAGDemoDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
