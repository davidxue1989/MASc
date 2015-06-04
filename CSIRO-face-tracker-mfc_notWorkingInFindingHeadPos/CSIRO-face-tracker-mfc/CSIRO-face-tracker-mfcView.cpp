
// CSIRO-face-tracker-mfcView.cpp : implementation of the CCSIROfacetrackermfcView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "CSIRO-face-tracker-mfc.h"
#endif

#include "CSIRO-face-tracker-mfcDoc.h"
#include "CSIRO-face-tracker-mfcView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCSIROfacetrackermfcView

IMPLEMENT_DYNCREATE(CCSIROfacetrackermfcView, CView)

BEGIN_MESSAGE_MAP(CCSIROfacetrackermfcView, CView)
END_MESSAGE_MAP()

// CCSIROfacetrackermfcView construction/destruction

CCSIROfacetrackermfcView::CCSIROfacetrackermfcView()
{
	// TODO: add construction code here

}

CCSIROfacetrackermfcView::~CCSIROfacetrackermfcView()
{
}

BOOL CCSIROfacetrackermfcView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CCSIROfacetrackermfcView drawing

void CCSIROfacetrackermfcView::OnDraw(CDC* /*pDC*/)
{
	CCSIROfacetrackermfcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CCSIROfacetrackermfcView diagnostics

#ifdef _DEBUG
void CCSIROfacetrackermfcView::AssertValid() const
{
	CView::AssertValid();
}

void CCSIROfacetrackermfcView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCSIROfacetrackermfcDoc* CCSIROfacetrackermfcView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCSIROfacetrackermfcDoc)));
	return (CCSIROfacetrackermfcDoc*)m_pDocument;
}
#endif //_DEBUG


// CCSIROfacetrackermfcView message handlers
