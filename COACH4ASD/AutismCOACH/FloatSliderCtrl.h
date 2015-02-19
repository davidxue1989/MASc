// CSliderCtrl can only handle integer min, max, and step size. 
// This is a simple wrapper around it to fake having float range and step size
// Babak Taati, IATSL, Aug 2009

#pragma once
#include "afxcmn.h"

#define        ROUNDDOUBLETOINT(A)        ((A)<0.0?-((int)(-(A)+0.5)):((int)((A)+0.5)))

class CFloatSliderCtrl :
	protected CSliderCtrl	// make it protected inheritance so the user doesn't have access to CSliderCtrl functions since they contain int values
							// therefore: I need GetMyWnd for a sligh modification in calling DDX_Control
{
public:
	CFloatSliderCtrl(void) {}
	~CFloatSliderCtrl(void) {}

	CWnd* GetMyWnd()	{
						return this;
					}

	void SetMinMaxStep(double min, double max, double step)	{	
																fMin = min;
																fMax = max;
																fStepSize = step;

																nMin = 0;
																nMax = ROUNDDOUBLETOINT( (max-min)/step );

																SetRange(nMin, nMax);
															}

	void SetCurrentPos(double pos)	{
										nPos = ROUNDDOUBLETOINT((pos-fMin)/fStepSize);
										SetPos(nPos);
									}

	double GetCurrentPos()	{	
								nPos = GetPos();
								fPos = nPos * fStepSize + fMin;
								return fPos;
							}

protected:
	double fMin, fMax, fStepSize, fPos;
	int nMin, nMax, nPos;

};
