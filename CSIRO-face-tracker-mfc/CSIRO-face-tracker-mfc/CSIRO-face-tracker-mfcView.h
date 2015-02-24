
// CSIRO-face-tracker-mfcView.h : interface of the CCSIROfacetrackermfcView class
//

#pragma once


class CCSIROfacetrackermfcView : public CView
{
protected: // create from serialization only
	CCSIROfacetrackermfcView();
	DECLARE_DYNCREATE(CCSIROfacetrackermfcView)

// Attributes
public:
	CCSIROfacetrackermfcDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CCSIROfacetrackermfcView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in CSIRO-face-tracker-mfcView.cpp
inline CCSIROfacetrackermfcDoc* CCSIROfacetrackermfcView::GetDocument() const
   { return reinterpret_cast<CCSIROfacetrackermfcDoc*>(m_pDocument); }
#endif

