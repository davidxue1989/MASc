// CalibrateDlg.h : header file
// Babak Taati, Jul 2009
// based on colorcalibrate.h, by Axel von Bertoldi, last modified Jul 2006

#pragma once


// CCalibrateDlg dialog
#include "HandTracker.h"
#include "FloatSliderCtrl.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "Resource.h"

//#include <cxtypes.h>


//#define NUM_REGIONS 7//Modification for soap
#define NUM_REGIONS 8 //dx oct 24, 2012

#define OBECT_TYPES 3	//number of different object types to track. i.e. hand and towel. by type I mean objects that have a different colour model.//Modification for soap
#define SCALE 4	// used to scale the size of the selected region (when calibrating) to the min/med
				// object size (in pixels). why this value? probably because it works. the final min/max can be adjusted anyway, so it doesn't
				// really matter. this is just a starting point...

//enums

enum OBJECT_TYPE
{	//the types of objects we track
	OBJECT_HAND = 0,
	OBJECT_TOWEL = 1,
	OBJECT_SOAP = 2,//Modification for soap
};

enum OBJECT_REGION
{	// object regions
	REGION_SOAP = 0,
	REGION_TAP_LEFT,
	REGION_TAP_RIGHT,
	REGION_WATER,
	REGION_SINK,
	REGION_TOWEL,
	REGION_SOAP_SPOUT,//Modification for soap
	REGION_FAUCET
};

//helpful structs

// hsv tuple
struct ColorItem
{
	double h;
	double s;
	double v;
};
// colour model used by tracker
struct ColorModel
{
	ColorItem mean;
	ColorItem sig; // variance
	ColorItem high;
	ColorItem low;
};

struct ObjectRegion
{
	
	ObjectRegion()		// default constructor
					{	// set everything to 0
						centreX = 0;
						centreY = 0;
						radius = 0;
						width = 0;
						height = 0;
					}

	int centreX;
	int centreY;
	float radius;
	int width;
	int height;
};


class CCalibrateDlg : public CDialog
{
	DECLARE_DYNAMIC(CCalibrateDlg)

public:
	CCalibrateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCalibrateDlg();

	// { Babak
	void SetImage(IplImage* inImg);
	void SetTracker(HandTracker* t);
	ObjectRegion GetRegion(OBJECT_REGION k);
	void SetFlags(bool* calibrating)//, bool* water_detector_is_initialized, bool* action_detector_is_initialized)	
																			{	
																				calibWindowOn = calibrating; 
																				//waterDetectorInitialized = water_detector_is_initialized; 
																				//actionDetectorInitialized = action_detector_is_initialized;
																			}
	void SetVideoFilename(CString fn);
	int CalculateObjectStatistics(bool update=false);
	void LoadDefaultCalibFileIfItExists();
	// }

// Dialog Data
	enum { IDD = IDD_CALIBRATE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// { Babak
	
	//void GetPoints(int* x_0, int* y_0, int* x_1, int* y_1);
	void GetBox(int* x, int* y, int* w, int* h);

	void CCalibrateDlg::CleanImages(); // release memory
	void ProcessMouseCallback(int event, int x, int y, int flags, void* param);

	void CCalibrateDlg::UpdateSliders();
	

	void SetTrackerObjectRegion(OBJECT_REGION reg);

	int CalculateColorStatistics(bool update=false);
	void MakeHsvImage();
	
	void GetTrackerModel(); // from the tracker
	void SetTrackerModel(); // to the tracker

	void LoadCalib(); // from a file (ask user for filename)
public:	
	void LoadCalib(CString filename); // from a file
protected:
	void SaveCalib(); // to a file (default filename)
	void SaveCalib(CString filename); // to a file

	//void LogModel(int trial); // save to the trial path
	
	void GetTrackerColorModel(); // from the tracker
	//int	 LoadColorModel(CString filename); // from a file
	void SetTrackerColorModel(); // to the tracker 
	//void SetColorModel(ColorModel obj, cc_size obj_size);

	void SetTrackerRegions(); // to the tracker
	//int	LoadRegions(); // form a file

	IplImage* originalImage; // the one received from CDoc (is needed to reset showImage when drawing a new rectangle)
	IplImage* showImage; // show this one & draw rectacles/circles on this one
	HandTracker* tracker; // pointer to the real tracker (no memory is allocated/released here)

	int x0, x1, y0, y1; // rectangle corner coordinates
	float radius; // distance between (x0,y0) and (x1,y1);

	
	//std::string rootLogPath;
	//std::string subjectName;

	OBJECT_TYPE selectedObject;
	cc_size size[OBECT_TYPES];
	ColorModel objects[OBECT_TYPES];

public:
	//bool textput;
	ObjectRegion regions[NUM_REGIONS];
	//dx oct 24, 2012
	//int faucetx, faucety, faucetw, fauceth;//Justin 
	//bool regionset;

protected:
	int width, height;
	bool modified; // is true if the calibration has been modified (so a 'do you wanna save' prompt comes up before closing the dlg
	bool* calibWindowOn; // set this to 0 when exiting (so CDoc knows when the window is closed)
	bool* waterDetectorInitialized; // set this to 0 if calib is modified (so CDoc knows to re initialize water detection) (important for the size of the water region or the colour of the hands)
	bool* actionDetectorInitialized; //  set this to 0 if calib is modified (so CDoc knows to re initialize water detection) (important for the the colour of the hands)

	//CString videoFilename;
	CString defaultCalibFilename;
	//}

	friend void OnMouseClickOnCalibImg(int event, int x, int y, int flags, void* param);

	DECLARE_MESSAGE_MAP()

	double meanH;
	double meanS;
	double meanV;
	double varianceH;
	double varianceS;
	double varianceV;
	double highH;
	double highS;
	double highV;
	double lowH;
	double lowS;
	double lowV;
	int sizeMin;
	int sizeMed;
	int sizeMax;
	CFloatSliderCtrl sliderMeanH;
	CFloatSliderCtrl sliderMeanS;
	CFloatSliderCtrl sliderMeanV;
	CFloatSliderCtrl sliderVarianceH;
	CFloatSliderCtrl sliderVarianceS;
	CFloatSliderCtrl sliderVarianceV;
	CFloatSliderCtrl sliderHighH;
	CFloatSliderCtrl sliderHighS;
	CFloatSliderCtrl sliderHighV;
	CFloatSliderCtrl sliderLowH;
	CFloatSliderCtrl sliderLowS;
	CFloatSliderCtrl sliderLowV;
	CSliderCtrl sliderSizeMin;
	CSliderCtrl sliderSizeMed;
	CSliderCtrl sliderSizeMax;

	CMenu mainCalibMenu;
	HACCEL  m_hAccelTable;	// so the dialog box can have hot keys (= short cuts = accelerators)
							// reference: http://support.microsoft.com/kb/222829

public:

	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	virtual INT_PTR DoModal();
	
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedCancel();


	afx_msg void OnBnClickedCalibrateButton();
	afx_msg void OnBnClickedCalibUpdateButton();
	afx_msg void OnBnClickedRegionSoapButton();
	afx_msg void OnBnClickedRegionRightTapButton();
	afx_msg void OnBnClickedRegionLeftTapButton();
	afx_msg void OnBnClickedRegionTowelButton();
	afx_msg void OnBnClickedRegionSinkButton();
	afx_msg void OnBnClickedRegionWaterButton();
	afx_msg void OnBnClickedObjectTowelRadio();
	afx_msg void OnBnClickedObjectHandRadio();

	// +++++++++++++++++++++++++++ TEMPORARY!! (start) ++++++++++++++++++++++++
	//friend class CRERCAnnotatorDoc;
	// +++++++++++++++++++++++++++ TEMPORARY!! (end) ++++++++++++++++++++++++

	afx_msg void OnBnClickedLoadCalibButton();
	afx_msg void OnBnClickedSaveCalibButton();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);


	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCalibFileOpen();
	afx_msg void OnCalibFileSave();
	afx_msg void OnCalibViewRegions();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedRegionSoapSpoutButton();//Modification for soap
	afx_msg void OnBnClickedObjectSoapRadio();//Modification for soap
	afx_msg void OnBnClickedRegionFaucetButton();
};
