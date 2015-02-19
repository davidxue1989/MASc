
// NAOAGDemoView.h : interface of the CNAOAGDemoView class
//

#pragma once

enum
{
	TIMER_ID_AGDEMO,
};

class CNAOAGDemoView : public CView
{
protected: // create from serialization only
	CNAOAGDemoView();
	DECLARE_DYNCREATE(CNAOAGDemoView)

// Attributes
public:
	CNAOAGDemoDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CNAOAGDemoView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

#ifndef _DEBUG  // debug version in NAOAGDemoView.cpp
inline CNAOAGDemoDoc* CNAOAGDemoView::GetDocument() const
   { return reinterpret_cast<CNAOAGDemoDoc*>(m_pDocument); }
#endif

