
// NAOAGDemoView.cpp : implementation of the CNAOAGDemoView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "NAOAGDemo.h"
#endif

#include "NAOAGDemoDoc.h"
#include "NAOAGDemoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNAOAGDemoView

IMPLEMENT_DYNCREATE(CNAOAGDemoView, CView)

BEGIN_MESSAGE_MAP(CNAOAGDemoView, CView)
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CNAOAGDemoView construction/destruction

CNAOAGDemoView::CNAOAGDemoView()
{
	// TODO: add construction code here
}

CNAOAGDemoView::~CNAOAGDemoView()
{
}

BOOL CNAOAGDemoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CNAOAGDemoView drawing

void CNAOAGDemoView::OnDraw(CDC* /*pDC*/)
{
	CNAOAGDemoDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CNAOAGDemoView diagnostics

#ifdef _DEBUG
void CNAOAGDemoView::AssertValid() const
{
	CView::AssertValid();
}

void CNAOAGDemoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CNAOAGDemoDoc* CNAOAGDemoView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNAOAGDemoDoc)));

	return (CNAOAGDemoDoc*)m_pDocument;
}
#endif //_DEBUG


// CNAOAGDemoView message handlers


void CNAOAGDemoView::OnTimer(UINT_PTR nIDEvent)
{
	CNAOAGDemoDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	switch (nIDEvent) {
	case TIMER_ID_AGDEMO:
		//if (pDoc->agNAO.isBusy())
		//	return;
		//if (pDoc->gotGaze) {
		//	pDoc->agNAO.doGotGaze();
		//}
		//else {
		//	pDoc->agNAO.doAG();
		//}


		if (pDoc->gaze == pDoc->gNone)
			pDoc->agNAO.executeAG();
		else if (pDoc->gaze == pDoc->gNAO)
			pDoc->agNAO.executePrompt();
		else if (pDoc->gaze == pDoc->gObj)
		{
			pDoc->agNAO.executeReward();
			//remove timer
		}
	}

	CView::OnTimer(nIDEvent);
}
