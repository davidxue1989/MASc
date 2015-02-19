// CalibrateDlg.cpp : implementation file
// Babak Taati, Jul 2009
// based on colorcalibrate.cpp, by Axel von Bertoldi, last modified Jul 2006

#include "stdafx.h"
#include "AutismCOACH.h"
#include "CalibrateDlg.h"
#include "PFHTDefines.h"
#include "VidPlayerDefines.h"

#define CALIB_IMAGE_OPENCV_NAME "CalibImage"

// misc.
double square(double x) { return (x*x); }


// this global variables is needed due to non-Obj-Oriented nature of cvSetMouseCallback function (OpenCV) 
CCalibrateDlg* globalCalibDlg = NULL;	//global pointer to CCalibrateDlg class (is initialised in OnInitDialog)

// this global function is needed due to non-Obj-Oriented nature of cvSetMouseCallback function (OpenCV) 
void OnMouseClickOnCalibImg(int event, int x, int y, int flags, void* param) //global function to handle mouse action callback from image window
{
	globalCalibDlg->ProcessMouseCallback(event, x, y, flags, param);
}


// CCalibrateDlg dialog

IMPLEMENT_DYNAMIC(CCalibrateDlg, CDialog)

CCalibrateDlg::CCalibrateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCalibrateDlg::IDD, pParent)
	, meanH(0)
	, meanS(0)
	, meanV(0)
	, varianceH(0)
	, varianceS(0)
	, varianceV(0)
	, highH(0)
	, highS(0)
	, highV(0)
	, lowH(0)
	, lowS(0)
	, lowV(0)
	, sizeMin(0)
	, sizeMed(0)
	, sizeMax(0)
	, tracker(NULL)
	, originalImage(NULL)
	, showImage(NULL)
	, x0(0)
	, x1(0)
	, y0(0)
	, y1(0)
	, radius(0)
	, width(0)
	, height(0)
	, waterDetectorInitialized(NULL)
	, actionDetectorInitialized(NULL)
{
	/*
	sliderMeanH.SetCurrentPos(meanH);
	sliderMeanS.SetCurrentPos(meanS);
	sliderMeanV.SetCurrentPos(meanV);

	sliderVarianceH.SetCurrentPos(varianceH);
	sliderVarianceS.SetCurrentPos(varianceS);
	sliderVarianceV.SetCurrentPos(varianceV);

	sliderHighH.SetCurrentPos(highH);
	sliderHighS.SetCurrentPos(highS);
	sliderHighV.SetCurrentPos(highV);

	sliderLowH.SetCurrentPos(lowH);
	sliderLowS.SetCurrentPos(lowS);
	sliderLowV.SetCurrentPos(lowV);

	sliderSizeMin.SetPos(sizeMin);
	sliderSizeMed.SetPos(sizeMed);
	sliderSizeMax.SetPos(sizeMax);
	*/
	//textput = false;
	m_hAccelTable = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_CALIB_MAIN_MENU));

	//regionset = false; //dx oct 24, 2012
}

CCalibrateDlg::~CCalibrateDlg()
{
	CleanImages();
}

void CCalibrateDlg::CleanImages()
{
	if (originalImage!=NULL)
	{
		cvReleaseImage(&originalImage);
		originalImage = NULL;
	}
	if (showImage!=NULL)
	{
		cvReleaseImage(&showImage);
		showImage = NULL;
	}
}

void CCalibrateDlg::SetTracker(HandTracker* t)
{
	tracker = t;
}

void CCalibrateDlg::SetImage(IplImage* inImg)
{
	CleanImages();

	if (inImg != NULL)
	{
		originalImage = cvCloneImage(inImg);
		width = inImg->width;
		height = inImg->height;
	}

	// the window is designed for 640*480. So have to move around controls/statics if it's 320*240
	//GetDlgItem(IDC_CALIBIMAGE_STATIC)->MoveWindow(1, 1, width, height, 1);
	//CWnd* hhhh = GetDlgItem(IDC_CALIBIMAGE_STATIC);
}

void CCalibrateDlg::SetVideoFilename(CString fn)
{
	//videoFilename = fn;
	defaultCalibFilename = fn;
	defaultCalibFilename.Replace(".avi", ".calib");
}

/*
void CCalibrateDlg::GetPoints(int* x_0, int* y_0, int* x_1, int* y_1)
{
	*x_0 = x0;
	*x_1 = x1;
	*y_0 = y0;
	*y_1 = y1;
}
*/

void CCalibrateDlg::GetBox(int* x, int* y, int* w, int* h)
{
	*x = min(x0, x1);
	*y = min(y0, y1);
	*w = abs(x0 - x1);
	*h = abs(y0 - y1);
	assert(abs(x0 - x1) == Abs(x0 - x1));	
	assert(abs(y0 - y1) == Abs(y0 - y1));
}

/*
void CCalibrateDlg::GetCircle(int* x, int* y, float* r)
{
	*x = x0;
	*y = y0;
	*r = radius;
}
*/

void CCalibrateDlg::GetTrackerModel()
{
	GetTrackerColorModel();
}

void CCalibrateDlg::SetTrackerModel()
{
	SetTrackerColorModel();
	SetTrackerRegions();
}

void CCalibrateDlg::SetTrackerRegions()//Edited - Justin Bimbrahw - December 6, 2010
{
	tracker->soapPos.setTo(regions[REGION_SOAP].centreX/2, regions[REGION_SOAP].centreY/2);
	tracker->min_soap_dist = regions[REGION_SOAP].radius/2;

	tracker->soap_spoutPos.setTo(regions[REGION_SOAP_SPOUT].centreX/2, regions[REGION_SOAP_SPOUT].centreY/2);
	tracker->min_soap_spout_dist = regions[REGION_SOAP_SPOUT].radius/2;

	tracker->LTapPos.setTo(regions[REGION_TAP_LEFT].centreX/2, regions[REGION_TAP_LEFT].centreY/2);
	tracker->min_ltap_dist = regions[REGION_TAP_LEFT].radius/2;

	tracker->RTapPos.setTo(regions[REGION_TAP_RIGHT].centreX/2, regions[REGION_TAP_RIGHT].centreY/2);
	tracker->min_rtap_dist = regions[REGION_TAP_RIGHT].radius/2;

	tracker->waterPos.setTo(regions[REGION_WATER].centreX/2, regions[REGION_WATER].centreY/2);
	tracker->min_water_dist = regions[REGION_WATER].radius/2;

	tracker->sinkPos.setTo(regions[REGION_SINK].centreX/2, regions[REGION_SINK].centreY/2);
	tracker->min_sink_dist = regions[REGION_SINK].radius/2;
									
	tracker->min_towel_dist = regions[REGION_TOWEL].radius/2;

	return;
}


void CCalibrateDlg::LoadDefaultCalibFileIfItExists()
{
	LoadCalib(defaultCalibFilename);
	//bool success = LoadCalib(defaultCalibFilename);
	//return success;
}

void CCalibrateDlg::LoadCalib()
{
	CString filename; // default filename = [video file].avi -> [video file].calib

	char strFilter[] = { "Calibration Files (*.calib)|*.calib|All Files (*.*)|*.*||" };
	CFileDialog FileDlg(TRUE, ".calib", defaultCalibFilename, 0, strFilter);
	if( FileDlg.DoModal() == IDOK )
	{
		filename = FileDlg.GetPathName();		
	}
	else
		return; // failure 

	LoadCalib(filename);
	return;
	
}

void CCalibrateDlg::LoadCalib(CString filename)
{
	// To Do: serialize ...

	if (waterDetectorInitialized != NULL) // is only null the very first time
	{
		*waterDetectorInitialized = false;
	}

	if (actionDetectorInitialized != NULL) // is only null the very first time
	{
		*actionDetectorInitialized = false;
	}

	FILE* fp;
	char buf[1024]; // temp value read from file
	float val=0; // temp value read from file
	int N=0; // temp value read from file
	errno_t err;
	
	//fopen_s(&fp, filename, "r");
	//fopen_s(&fp, "E:\\home\\david\\projects\\RERC\\sampleVideo\\bc28_trial_26_d_comp_calib.txt", "r");
	//if( (err  = fopen_s( &fp, "E:\\home\\david\\projects\\RERC\\sampleVideo\\bc28_trial_26_d_comp_calib.txt", "r" )) !=0 )
	if( (err  = fopen_s( &fp, filename, "r" )) !=0 )
	{
		MessageBox("Cannot open calibration file. You should calibrate this video or load a valid calibration file.", "Error");
		return;
	}	

	// to be improved: 
	//	- assumes a very rigid file format ... 
	//	- no exception handling, ...

	// load color models 
	fscanf_s(fp, "%s", buf, 1024); // must be 'MODELS'
		assert(strcmp(buf, "MODELS")==0); //dxdebug oct 25, 2012

	for (int ii=0; ii<OBECT_TYPES; ii++)
	{
		fscanf_s(fp, "%s", buf, 1024); // must be 'HAND' the first time and TOWEL the second time and SOAP the third time
		assert(strcmp(buf, "HAND")==0||strcmp(buf, "TOWEL")==0||strcmp(buf, "SOAP")==0); //dxdebug oct 25, 2012

		fscanf_s(fp, "%s", buf, 1024); // must be 'Mean'
		assert(strcmp(buf, "Mean")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%s", buf, 1024); // must be 'h'
		assert(strcmp(buf, "h")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%f", &val); // hand - mean - h
		objects[ii].mean.h = val;
		fscanf_s(fp, "%s", buf, 1024); // must be 's'
		assert(strcmp(buf, "s")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%f", &val); // hand - mean - s
		objects[ii].mean.s = val;
		fscanf_s(fp, "%s", buf, 1024); // must be 'v'
		assert(strcmp(buf, "v")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%f", &val); // hand - mean - v
		objects[ii].mean.v = val;

		fscanf_s(fp, "%s", buf, 1024); // must be 'Variance'
		assert(strcmp(buf, "Variance")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%s", buf, 1024); // must be 'h'
		assert(strcmp(buf, "h")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%f", &val); // hand - sig - h
		objects[ii].sig.h = val;
		fscanf_s(fp, "%s", buf, 1024); // must be 's'
		assert(strcmp(buf, "s")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%f", &val); // hand - sig - s
		objects[ii].sig.s = val;
		fscanf_s(fp, "%s", buf, 1024); // must be 'v'
		assert(strcmp(buf, "v")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%f", &val); // hand - sig - v
		objects[ii].sig.v = val;

		fscanf_s(fp, "%s", buf, 1024); // must be 'High'
		assert(strcmp(buf, "High")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%s", buf, 1024); // must be 'h'
		assert(strcmp(buf, "h")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%f", &val); // hand - high - h
		objects[ii].high.h = val;
		fscanf_s(fp, "%s", buf, 1024); // must be 's'
		assert(strcmp(buf, "s")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%f", &val); // hand - high - s
		objects[ii].high.s = val;
		fscanf_s(fp, "%s", buf, 1024); // must be 'v'
		assert(strcmp(buf, "v")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%f", &val); // hand - high - v
		objects[ii].high.v = val;

		fscanf_s(fp, "%s", buf, 1024); // must be 'Low'
		assert(strcmp(buf, "Low")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%s", buf, 1024); // must be 'h'
		assert(strcmp(buf, "h")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%f", &val); // hand - low - h
		objects[ii].low.h = val;
		fscanf_s(fp, "%s", buf, 1024); // must be 's'
		assert(strcmp(buf, "s")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%f", &val); // hand - low - s
		objects[ii].low.s = val;
		fscanf_s(fp, "%s", buf, 1024); // must be 'v'
		assert(strcmp(buf, "v")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%f", &val); // hand - low - v
		objects[ii].low.v = val;

		fscanf_s(fp, "%s", buf, 1024); // must be 'Size'
		assert(strcmp(buf, "Size")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%s", buf, 1024); // must be 'min'
		assert(strcmp(buf, "min")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%d", &N, 1); // hand - size - min
		size[ii].min = N;
		fscanf_s(fp, "%s", buf, 1024); // must be 'med'
		assert(strcmp(buf, "med")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%d", &N, 1); // hand - size - med
		size[ii].med = N;
		fscanf_s(fp, "%s", buf, 1024); // must be 'max'
		assert(strcmp(buf, "max")==0); //dxdebug oct 25, 2012
		fscanf_s(fp, "%d", &N, 1); // hand - size - max
		size[ii].max = N;		
	}

	// load regions

	fscanf_s(fp, "%s", buf, 1024); // must be 'REGIONS'	
	assert(strcmp(buf, "REGIONS")==0); //dxdebug oct 24, 2012
	
	for (int ii=0; ii<NUM_REGIONS; ii++)
	{
		fscanf_s(fp, "%s", buf, 1024); // must be SOAP, LEFT_TAP, RIGHT_TAP, WATER, SINK, TOWEL, SOAP_SPOUT, FAUCET in exaclty this order //dx oct 24, 2012
		assert(strcmp(buf, "SOAP")==0||strcmp(buf, "LEFT_TAP")==0||strcmp(buf, "RIGHT_TAP")==0||strcmp(buf, "WATER")==0||strcmp(buf, "SINK")==0||strcmp(buf, "TOWEL")==0||strcmp(buf, "SOAP_SPOUT")==0||strcmp(buf, "FAUCET")==0); //dxdebug oct 24, 2012

		fscanf_s(fp, "%s", buf, 1024); // must be 'centreX'
		assert(strcmp(buf, "centreX")==0); //dxdebug oct 24, 2012
		fscanf_s(fp, "%d", &N, 1); // centreX
		#ifdef IMAGE_SIZE_ORIGINAL
			regions[ii].centreX = N;
		#else
			regions[ii].centreX = N;///2;//Edited - Justin Bimbrahw - December 6, 2010
		#endif

		fscanf_s(fp, "%s", buf, 1024); // must be 'centreY'
		fscanf_s(fp, "%d", &N, 1); // centreY
		assert(strcmp(buf, "centreY")==0); //dxdebug oct 24, 2012
		#ifdef IMAGE_SIZE_ORIGINAL
			regions[ii].centreY = N;
		#else
			regions[ii].centreY = N;///2;//Edited - Justin Bimbrahw - December 6, 2010
		#endif

		fscanf_s(fp, "%s", buf, 1024); // must be 'radius'
		assert(strcmp(buf, "radius")==0); //dxdebug oct 24, 2012
		fscanf_s(fp, "%f", &val, 1); // radius
		#ifdef IMAGE_SIZE_ORIGINAL
			regions[ii].radius = val;
		#else
			regions[ii].radius = val;///2;//Edited - Justin Bimbrahw - December 6, 2010
		#endif

		fscanf_s(fp, "%s", buf, 1024); // must be 'width'
		assert(strcmp(buf, "width")==0); //dxdebug oct 24, 2012
		fscanf_s(fp, "%d", &N, 1);
		#ifdef IMAGE_SIZE_ORIGINAL
			regions[ii].width = N;
		#else
			regions[ii].width = N;
		#endif

		fscanf_s(fp, "%s", buf, 1024); // must be 'height'
		assert(strcmp(buf, "height")==0); //dxdebug oct 24, 2012
		fscanf_s(fp, "%d", &N, 1);
		#ifdef IMAGE_SIZE_ORIGINAL
			regions[ii].height = N;
		#else
				regions[ii].height = N;
		#endif
	}
									
	fclose(fp);

	// set color model + set regions
	SetTrackerColorModel();	
	SetTrackerRegions();

	return;
}

/*
int	CCalibrateDlg::LoadRegions()
{
	FILE *fp = NULL;
	char fn[1024];
	char buf[1024];
	float val;
	OBJECT_REGION objReg; // soap, tap left, towel, etc

	// assemble the filename using the root log path and patient name, and just "region" for the file
	sprintf_s(fn, "%s/%s/region.txt",	rootLogPath.c_str(), subjectName.c_str());

	// open the file
	if ((fopen_s(&fp, fn, "r") ) == NULL)
	{	//dang!	
		MessageBox("CCalibrateDlg::LoadRegions failed!", "Error", MB_OK);
		return 1; // failure (following Jesse's style, 1=failure, 0=success)
	}

	// Parse the file. Sort of a state machine right now. Maybe it's not the best way to do this?
	while (!feof(fp))
	{
		//get a token
		fscanf_s(fp, "%s", buf);
		//what's the state?
		if (strcmp("Soap:", buf) == 0)
		{
			objReg = REGION_SOAP;
		}
		else if (strcmp("TapLeft:", buf) == 0)
		{
			objReg = REGION_TAP_LEFT;
		}
		else if (strcmp("TapRight:", buf) == 0)
		{
			objReg = REGION_TAP_RIGHT;
		}
		else if (strcmp("Water:", buf) == 0)
		{
			objReg = REGION_WATER;
		}
		else if (strcmp("Sink:", buf) == 0)
		{
			objReg = REGION_SINK;
		}
		else if (strcmp("Towel:", buf) == 0)
		{
			objReg = REGION_TOWEL;
		}
		else
		{
			fscanf_s(fp, "%f", &val);
			if(strcmp("centreX", buf) == 0)
			{
				this->regions[objReg].centreX = (int)val;
			}
			else if(strcmp("centreY", buf) == 0)
			{
				this->regions[objReg].centreY = (int)val;
			}
			else if(strcmp("radius", buf) == 0)
			{
				this->regions[objReg].radius = val;
			}
		}
	}
	fclose(fp);
		
	this->SetRegions();

	return 0; 
}
*/
void CCalibrateDlg::SaveCalib()
{
	CString filename; // default filename = [video file].avi -> [video file].calib

	char strFilter[] = { "Calibration Files (*.calib)|*.calib|All Files (*.*)|*.*||" };
	CFileDialog FileDlg(FALSE, ".calib", defaultCalibFilename, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
	if( FileDlg.DoModal() == IDOK )
	{
		filename = FileDlg.GetPathName();
	}
	else
		return; // failure 

	SaveCalib(filename);
}

void CCalibrateDlg::SaveCalib(CString filename)
{
	// To Do: serialize ...

	FILE* pf;
	fopen_s(&pf, filename, "wt");

	// save color models

	fprintf(pf, "MODELS\n");

	for (int ii=0; ii<OBECT_TYPES; ii++)
	{
		if (ii == OBJECT_HAND)
		{
			fprintf(pf, "\tHAND\n");
		}
		else if (ii == OBJECT_TOWEL)
		{
			fprintf(pf, "\tTOWEL\n");
		}
		else if (ii == OBJECT_SOAP)//Modification for soap
		{
			fprintf(pf, "\tSOAP\n");
		}

		fprintf(pf, "\t\tMean\n");
		fprintf(pf, "\t\t\t\th\t%.5f\n", objects[ii].mean.h);
		fprintf(pf, "\t\t\t\ts\t%.5f\n", objects[ii].mean.s);
		fprintf(pf, "\t\t\t\tv\t%.5f\n", objects[ii].mean.v);

		fprintf(pf, "\t\tVariance\n");
		fprintf(pf, "\t\t\t\th\t%.5f\n", objects[ii].sig.h);
		fprintf(pf, "\t\t\t\ts\t%.5f\n", objects[ii].sig.s);
		fprintf(pf, "\t\t\t\tv\t%.5f\n", objects[ii].sig.v);

		fprintf(pf, "\t\tHigh\n");
		fprintf(pf, "\t\t\t\th\t%.5f\n", objects[ii].high.h);
		fprintf(pf, "\t\t\t\ts\t%.5f\n", objects[ii].high.s);
		fprintf(pf, "\t\t\t\tv\t%.5f\n", objects[ii].high.v);

		fprintf(pf, "\t\tLow\n");
		fprintf(pf, "\t\t\t\th\t%.5f\n", objects[ii].low.h);
		fprintf(pf, "\t\t\t\ts\t%.5f\n", objects[ii].low.s);
		fprintf(pf, "\t\t\t\tv\t%.5f\n", objects[ii].low.v);

		fprintf(pf, "\t\tSize\n");
		fprintf(pf, "\t\t\t\tmin\t%d\n", size[ii].min);
		fprintf(pf, "\t\t\t\tmed\t%d\n", size[ii].med);
		fprintf(pf, "\t\t\t\tmax\t%d\n", size[ii].max);		
	}


	// save regions

	fprintf(pf, "REGIONS\n");

	for (int ii=0; ii<NUM_REGIONS; ii++)
	{
		if (ii == REGION_SOAP)
		{
			fprintf(pf, "\tSOAP\n");
		}
		else if (ii == REGION_TAP_LEFT)
		{
			fprintf(pf, "\tLEFT_TAP\n");
		}
		else if (ii == REGION_TAP_RIGHT)
		{
			fprintf(pf, "\tRIGHT_TAP\n");
		}
		else if (ii == REGION_WATER)
		{
			fprintf(pf, "\tWATER\n");
		}
		else if (ii == REGION_SINK)
		{
			fprintf(pf, "\tSINK\n");
		}
		else if (ii == REGION_TOWEL)
		{
			fprintf(pf, "\tTOWEL\n");
		}
		else if (ii == REGION_SOAP_SPOUT)//Modification for soap
		{
			fprintf(pf, "\tSOAP_SPOUT\n");
		}
		else if (ii == REGION_FAUCET) //dx oct 24, 2012
		{
			fprintf(pf, "\tFAUCET\n");
		}
		

		// if 640x480, save normally
		// if 320x240, times two (so we always save the regions for a 640x480 video)
		#ifdef IMAGE_SIZE_ORIGINAL
			fprintf(pf, "\t\t\tcentreX\t%d\n", regions[ii].centreX);
			fprintf(pf, "\t\t\tcentreY\t%d\n", regions[ii].centreY);
			fprintf(pf, "\t\t\tradius\t%.5f\n", regions[ii].radius);
		#else
			fprintf(pf, "\t\t\tcentreX\t%d\n", regions[ii].centreX);//*2);//Edited December 6, 2010 - Justin Bimbrahw
			fprintf(pf, "\t\t\tcentreY\t%d\n", regions[ii].centreY);//*2);
			fprintf(pf, "\t\t\tradius\t%.5f\n", regions[ii].radius);//*2);
			fprintf(pf, "\t\t\twidth\t%d\n", regions[ii].width);//*2);
			fprintf(pf, "\t\t\theight\t%d\n", regions[ii].height);//*2);
		#endif
	}

	fclose(pf);
}

/*
void CCalibrateDlg::LogModel(int trial)
{
	MessageBox("Not coded yet!", "oops", MB_OK);
}
*/

void CCalibrateDlg::GetTrackerColorModel()
{
	//soap mean
	objects[OBJECT_SOAP].mean.h = tracker->sco.h;//Modification for soap
	objects[OBJECT_SOAP].mean.s = tracker->sco.s;//Modification for soap
	objects[OBJECT_SOAP].mean.v = tracker->sco.v;//Modification for soap
	//soap sig
	objects[OBJECT_SOAP].sig.h = tracker->ssigco.h;//Modification for soap
	objects[OBJECT_SOAP].sig.s = tracker->ssigco.s;//Modification for soap
	objects[OBJECT_SOAP].sig.v = tracker->ssigco.v;//Modification for soap
	//towel mean
	objects[OBJECT_TOWEL].mean.h = tracker->tco.h;//This line duplicated for soap
	objects[OBJECT_TOWEL].mean.s = tracker->tco.s;//This line duplicated for soap
	objects[OBJECT_TOWEL].mean.v = tracker->tco.v;//This line duplicated for soap
	//towel sig
	objects[OBJECT_TOWEL].sig.h = tracker->tsigco.h;//This line duplicated for soap
	objects[OBJECT_TOWEL].sig.s = tracker->tsigco.s;//This line duplicated for soap
	objects[OBJECT_TOWEL].sig.v = tracker->tsigco.v;//This line duplicated for soap
	//hand mean, use right hand only
	objects[OBJECT_HAND].mean.h = tracker->rhco.h;
	objects[OBJECT_HAND].mean.s = tracker->rhco.s;
	objects[OBJECT_HAND].mean.v = tracker->rhco.v;
	//hand sig
	objects[OBJECT_HAND].sig.h = tracker->rhsigco.h;
	objects[OBJECT_HAND].sig.s = tracker->rhsigco.s;
	objects[OBJECT_HAND].sig.v = tracker->rhsigco.v;

	//soap high
	objects[OBJECT_SOAP].high.h = tracker->scthi.h;//Modification for soap
	objects[OBJECT_SOAP].high.s = tracker->scthi.s;//Modification for soap
	objects[OBJECT_SOAP].high.v = tracker->scthi.v;//Modification for soap
	//soap low
	objects[OBJECT_SOAP].low.h = tracker->sctlo.h;//Modification for soap
	objects[OBJECT_SOAP].low.s = tracker->sctlo.s;//Modification for soap
	objects[OBJECT_SOAP].low.v = tracker->sctlo.v;//Modification for soap
	//towel high
	objects[OBJECT_TOWEL].high.h = tracker->tcthi.h;//This line duplicated for soap
	objects[OBJECT_TOWEL].high.s = tracker->tcthi.s;//This line duplicated for soap
	objects[OBJECT_TOWEL].high.v = tracker->tcthi.v;//This line duplicated for soap
	//towel low
	objects[OBJECT_TOWEL].low.h = tracker->tctlo.h;//This line duplicated for soap
	objects[OBJECT_TOWEL].low.s = tracker->tctlo.s;//This line duplicated for soap
	objects[OBJECT_TOWEL].low.v = tracker->tctlo.v;//This line duplicated for soap
	//hand high
	objects[OBJECT_HAND].high.h = tracker->cthi.h;
	objects[OBJECT_HAND].high.s = tracker->cthi.s;
	objects[OBJECT_HAND].high.v = tracker->cthi.v;
	//hand low
	objects[OBJECT_HAND].low.h = tracker->ctlo.h;
	objects[OBJECT_HAND].low.s = tracker->ctlo.s;
	objects[OBJECT_HAND].low.v = tracker->ctlo.v;

	// added by Babak:

	// min/med/max hand sizes
	size[OBJECT_HAND].min = tracker->hand_size.min;
	size[OBJECT_HAND].med = tracker->hand_size.med;
	size[OBJECT_HAND].max = tracker->hand_size.max;

	// min/med/max towel sizes
	size[OBJECT_TOWEL].min = tracker->towel_size.min;
	size[OBJECT_TOWEL].med = tracker->towel_size.med;
	size[OBJECT_TOWEL].max = tracker->towel_size.max;

	return;
}

void CCalibrateDlg::SetTrackerColorModel()
{
	//soap mean
	tracker->sco.setColor(objects[OBJECT_SOAP].mean.h, objects[OBJECT_SOAP].mean.s, objects[OBJECT_SOAP].mean.v);//Modification for soap
	
	//soap sig
	tracker->ssigco.setColor(objects[OBJECT_SOAP].sig.h, objects[OBJECT_SOAP].sig.s, objects[OBJECT_SOAP].sig.v);//Modification for soap
	
	//towel mean
	tracker->tco.setColor(objects[OBJECT_TOWEL].mean.h, objects[OBJECT_TOWEL].mean.s, objects[OBJECT_TOWEL].mean.v);//This line duplicated for soap
	
	//towel sig
	tracker->tsigco.setColor(objects[OBJECT_TOWEL].sig.h, objects[OBJECT_TOWEL].sig.s, objects[OBJECT_TOWEL].sig.v);//This line duplicated for soap
	
	//left hand mean
	tracker->lhco.setColor(	objects[OBJECT_HAND].mean.h, objects[OBJECT_HAND].mean.s, objects[OBJECT_HAND].mean.v);
	
	//left hand sig
	tracker->lhsigco.setColor(objects[OBJECT_HAND].sig.h, objects[OBJECT_HAND].sig.s, objects[OBJECT_HAND].sig.v);

	//right hand mean (same as left)
	tracker->rhco.setColor(	objects[OBJECT_HAND].mean.h, objects[OBJECT_HAND].mean.s, objects[OBJECT_HAND].mean.v);

	//right hand sig (same as left)
	tracker->rhsigco.setColor(objects[OBJECT_HAND].sig.h, objects[OBJECT_HAND].sig.s, objects[OBJECT_HAND].sig.v);

	//segmentation

	//hand high
	tracker->cthi.setColor(objects[OBJECT_HAND].high.h, objects[OBJECT_HAND].high.s, objects[OBJECT_HAND].high.v);

	//hand low
	tracker->ctlo.setColor(objects[OBJECT_HAND].low.h, objects[OBJECT_HAND].low.s, objects[OBJECT_HAND].low.v);

	//towel high
	tracker->tcthi.setColor(objects[OBJECT_TOWEL].high.h, objects[OBJECT_TOWEL].high.s, objects[OBJECT_TOWEL].high.v);//This line duplicated for soap

	//towel low
	tracker->tctlo.setColor(objects[OBJECT_TOWEL].low.h, objects[OBJECT_TOWEL].low.s, objects[OBJECT_TOWEL].low.v);//This line duplicated for soap

	//soap high
	tracker->scthi.setColor(objects[OBJECT_SOAP].high.h, objects[OBJECT_SOAP].high.s, objects[OBJECT_SOAP].high.v);//Modification for soap

	//soap low
	tracker->sctlo.setColor(objects[OBJECT_SOAP].low.h, objects[OBJECT_SOAP].low.s, objects[OBJECT_SOAP].low.v);//Modification for soap


	// min and max hand/towel/soap sizes
	tracker->hand_size.min	= size[OBJECT_HAND].min;
	tracker->hand_size.med	= size[OBJECT_HAND].med;
	tracker->hand_size.max	= size[OBJECT_HAND].max;
	tracker->towel_size.min	= size[OBJECT_TOWEL].min;
	tracker->towel_size.med	= size[OBJECT_TOWEL].med;
	tracker->towel_size.max	= size[OBJECT_TOWEL].max;
	tracker->soap_size.min	= size[OBJECT_SOAP].min;//Modification for soap
	tracker->soap_size.med	= size[OBJECT_SOAP].med;//Modification for soap
	tracker->soap_size.max	= size[OBJECT_SOAP].max;//Modification for soap

	for (int i = 0; i < tracker->num_particles ; i++)
	{
		tracker->spf->newptc[i]->co.setColor(tracker->sco);//Modification for soap
		tracker->spf->newptc[i]->sigco.setColor(tracker->ssigco);//Modification for soap
		tracker->tpf->newptc[i]->co.setColor(tracker->tco);//This line duplicated for soap
		tracker->tpf->newptc[i]->sigco.setColor(tracker->tsigco);//This line duplicated for soap
		tracker->lhpf->newptc[i]->co.setColor(tracker->lhco);
		tracker->lhpf->newptc[i]->sigco.setColor(tracker->lhsigco);
		tracker->rhpf->newptc[i]->co.setColor(tracker->rhco);
		tracker->rhpf->newptc[i]->sigco.setColor(tracker->rhsigco);
	}

	//make the tracker aware of the new values
	tracker->reset();

	return;
}

/*
void CCalibrateDlg::SetColorModel(ColorModel obj, cc_size obj_size)
{
	meanH = obj.mean.h;
	meanS = obj.mean.s;
	meanV = obj.mean.v;

	varianceH = obj.sig.h;
	varianceS = obj.sig.s;
	varianceV = obj.sig.v;

	highH = obj.high.h;
	highS = obj.high.s;
	highV = obj.high.v;

	lowH = obj.low.h;
	lowS = obj.low.s;
	lowV = obj.low.v;

	sizeMin = obj_size.min;
	sizeMed = obj_size.med;
	sizeMax = obj_size.max;
}
*/

void CCalibrateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MEAN_H_EDIT, meanH);
	DDX_Text(pDX, IDC_MEAN_S_EDIT, meanS);
	DDX_Text(pDX, IDC_MEAN_V_EDIT, meanV);

	DDX_Text(pDX, IDC_VARIANCE_H_EDIT, varianceH);
	DDX_Text(pDX, IDC_VARIANCE_S_EDIT, varianceS);
	DDX_Text(pDX, IDC_VARIANCE_V_EDIT, varianceV);

	DDX_Text(pDX, IDC_HIGH_H_EDIT, highH);
	DDX_Text(pDX, IDC_HIGH_S_EDIT, highS);
	DDX_Text(pDX, IDC_HIGH_V_EDIT, highV);

	DDX_Text(pDX, IDC_LOW_H_EDIT, lowH);
	DDX_Text(pDX, IDC_LOW_S_EDIT, lowS);
	DDX_Text(pDX, IDC_LOW_V_EDIT, lowV);

	DDX_Text(pDX, IDC_SIZE_MIN_EDIT, sizeMin);
	DDX_Text(pDX, IDC_SIZE_MED_EDIT, sizeMed);
	DDX_Text(pDX, IDC_SIZE_MAX_EDIT, sizeMax);

	// slight modification here since I'm using floatSliders, privately derived from CSliderCtrl
	DDX_Control(pDX, IDC_MEAN_H_SLIDER, *(sliderMeanH.GetMyWnd()));
	DDX_Control(pDX, IDC_MEAN_S_SLIDER, *(sliderMeanS.GetMyWnd()));
	DDX_Control(pDX, IDC_MEAN_V_SLIDER, *(sliderMeanV.GetMyWnd()));
	DDX_Control(pDX, IDC_VARIANCE_H_SLIDER, *(sliderVarianceH.GetMyWnd()));
	DDX_Control(pDX, IDC_VARIANCE_S_SLIDER, *(sliderVarianceS.GetMyWnd()));
	DDX_Control(pDX, IDC_VARIANCE_V_SLIDER, *(sliderVarianceV.GetMyWnd()));
	DDX_Control(pDX, IDC_HIGH_H_SLIDER, *(sliderHighH.GetMyWnd()));
	DDX_Control(pDX, IDC_HIGH_S_SLIDER, *(sliderHighS.GetMyWnd()));
	DDX_Control(pDX, IDC_HIGH_V_SLIDER, *(sliderHighV.GetMyWnd()));
	DDX_Control(pDX, IDC_LOW_H_SLIDER, *(sliderLowH.GetMyWnd()));
	DDX_Control(pDX, IDC_LOW_S_SLIDER, *(sliderLowS.GetMyWnd()));
	DDX_Control(pDX, IDC_LOW_V_SLIDER, *(sliderLowV.GetMyWnd()));
	DDX_Control(pDX, IDC_SIZE_MIN_SLIDER, sliderSizeMin);
	DDX_Control(pDX, IDC_SIZE_MED_SLIDER, sliderSizeMed);
	DDX_Control(pDX, IDC_SIZE_MAX_SLIDER, sliderSizeMax);
}



BEGIN_MESSAGE_MAP(CCalibrateDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CCalibrateDlg::OnBnClickedOk)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDCANCEL, &CCalibrateDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CALIBRATE_BUTTON, &CCalibrateDlg::OnBnClickedCalibrateButton)
	ON_BN_CLICKED(IDC_CALIB_UPDATE_BUTTON, &CCalibrateDlg::OnBnClickedCalibUpdateButton)
	ON_BN_CLICKED(IDC_REGION_SOAP_BUTTON, &CCalibrateDlg::OnBnClickedRegionSoapButton)
	ON_BN_CLICKED(IDC_REGION_RIGHT_TAP_BUTTON, &CCalibrateDlg::OnBnClickedRegionRightTapButton)
	ON_BN_CLICKED(IDC_REGION_LEFT_TAP_BUTTON, &CCalibrateDlg::OnBnClickedRegionLeftTapButton)
	ON_BN_CLICKED(IDC_REGION_TOWEL_BUTTON, &CCalibrateDlg::OnBnClickedRegionTowelButton)
	ON_BN_CLICKED(IDC_REGION_SINK_BUTTON, &CCalibrateDlg::OnBnClickedRegionSinkButton)
	ON_BN_CLICKED(IDC_REGION_WATER_BUTTON, &CCalibrateDlg::OnBnClickedRegionWaterButton)
	ON_BN_CLICKED(IDC_OBJECT_TOWEL_RADIO, &CCalibrateDlg::OnBnClickedObjectTowelRadio)
	ON_BN_CLICKED(IDC_OBJECT_HAND_RADIO, &CCalibrateDlg::OnBnClickedObjectHandRadio)
	ON_BN_CLICKED(IDC_LOAD_CALIB_BUTTON, &CCalibrateDlg::OnBnClickedLoadCalibButton)
	ON_BN_CLICKED(IDC_SAVE_CALIB_BUTTON, &CCalibrateDlg::OnBnClickedSaveCalibButton)
	ON_WM_SHOWWINDOW()
	ON_COMMAND(ID_CALIB_FILE_OPEN, &CCalibrateDlg::OnCalibFileOpen)
	ON_COMMAND(ID_CALIB_FILE_SAVE, &CCalibrateDlg::OnCalibFileSave)
	ON_COMMAND(ID_CALIB_VIEW_REGIONS, &CCalibrateDlg::OnCalibViewRegions)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON2, &CCalibrateDlg::OnBnClickedRegionSoapSpoutButton)//Modification for soap
	ON_BN_CLICKED(IDC_RADIO9, &CCalibrateDlg::OnBnClickedObjectSoapRadio)//Modification for soap
	ON_BN_CLICKED(IDC_BUTTON1, &CCalibrateDlg::OnBnClickedRegionFaucetButton)
END_MESSAGE_MAP()


// CCalibrateDlg message handlers

void CCalibrateDlg::OnBnClickedOk()
{
	if (modified)
	{
		if ( IDYES == MessageBox("Save modifications?", "Please confirm", MB_YESNO  ) )
		{
			SaveCalib();
		}
	}

	SetTrackerModel();
	*calibWindowOn = false;
	mainCalibMenu.DestroyMenu();
	//Justin - April 13, 2010
	//::MessageBox(NULL, "The camera has been stopped and will need to be restarted.\nIf you were in a prompting session you will need to restart it.", "FYI!", MB_OK);
	//textput = true;//Justin - Aug 20, 2010
	OnOK();
}


void CCalibrateDlg::OnBnClickedCancel()
{
	//dx oct 24, 2012: commented out since cancel means no saving
	//if (modified)
	//{
	//	if ( IDYES == MessageBox("Save modifications?", "Please confirm", MB_YESNO  ) )
	//	{
	//		SaveCalib();
	//	}
	//}

	//SetTrackerModel();

	*calibWindowOn = false;
	mainCalibMenu.DestroyMenu();
	//Justin - April 13, 2010
	//::MessageBox(NULL, "The camera has been stopped and will need to be restarted.\nIf you were in a prompting session you will need to restart it.", "FYI!", MB_OK);
	OnCancel();
}


BOOL CCalibrateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	mainCalibMenu.LoadMenu(IDR_CALIB_MAIN_MENU);
	SetMenu(&mainCalibMenu);

	if (originalImage != NULL)
	{
		showImage = cvCloneImage(originalImage);

		// code to display an OvenCV image window as a Picture Box widget inside the current dialog GUI  
		cvNamedWindow(CALIB_IMAGE_OPENCV_NAME, CV_WINDOW_AUTOSIZE); //CV_WINDOW_AUTOSIZE);
		HWND hWnd = (HWND)cvGetWindowHandle(CALIB_IMAGE_OPENCV_NAME);
		HWND hParent = ::GetParent(hWnd);
		::SetParent(hWnd,GetDlgItem(IDC_CALIBIMAGE_STATIC)->m_hWnd); 
		::ShowWindow(hParent,SW_HIDE);

		cvShowImage(CALIB_IMAGE_OPENCV_NAME, showImage);
	}

	ShowWindow(SW_SHOW);	// to force showing this dialog
							// http://www.experts-exchange.com/Programming/System/Windows__Programming/MFC/Q_24285957.html


	// place the calib dlg at the upper right corner of the screen (so it's the right of the main window)
	WINDOWPLACEMENT pwp;
	GetWindowPlacement(&pwp);
	pwp.rcNormalPosition.bottom = pwp.rcNormalPosition.bottom - pwp.rcNormalPosition.top;
	pwp.rcNormalPosition.top = (LONG)(0);
	pwp.rcNormalPosition.left += (LONG)::GetSystemMetrics( SM_CXFULLSCREEN ) - pwp.rcNormalPosition.right;
	pwp.rcNormalPosition.right = (LONG)::GetSystemMetrics( SM_CXFULLSCREEN );
	SetWindowPlacement(&pwp);

	// set the global pointer to this calib dlg (needed for handling mouse event on the image)
	globalCalibDlg = this;

	// set the mouse callback function for the calib img
	cvSetMouseCallback(CALIB_IMAGE_OPENCV_NAME, OnMouseClickOnCalibImg);

	// initialize calib rectangle values
	x0 = 0;
	x1 = 0;
	y0 = 0;
	y1 = 0;
				
	sliderMeanH.SetMinMaxStep(-180, 180, 0.5);
	sliderMeanS.SetMinMaxStep(0, 1, 0.005);
	sliderMeanV.SetMinMaxStep(0, 1, 0.005);
	sliderVarianceH.SetMinMaxStep(0, 180, 0.05);
	sliderVarianceS.SetMinMaxStep(0, 1, 0.005);
	sliderVarianceV.SetMinMaxStep(0, 1, 0.005);
	sliderHighH.SetMinMaxStep(-180, 180, 0.05);
	sliderHighS.SetMinMaxStep(0, 1, 0.005);
	sliderHighV.SetMinMaxStep(0, 1, 0.005);
	sliderLowH.SetMinMaxStep(-180, 180, 0.5);
	sliderLowS.SetMinMaxStep(0, 1, 0.005);
	sliderLowV.SetMinMaxStep(0, 1, 0.005);
	sliderSizeMin.SetRange(0, 10000);
	sliderSizeMed.SetRange(0, 10000);
	sliderSizeMax.SetRange(0, 10000);

	selectedObject = OBJECT_HAND;
	((CButton*)(GetDlgItem(IDC_OBJECT_HAND_RADIO)))->SetCheck(true);

	GetTrackerModel();
	UpdateSliders();

					//CWnd* wnd = GetDlgItem(IDC_CALIBIMAGE_STATIC);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

INT_PTR CCalibrateDlg::DoModal()
{
	if (originalImage==NULL)
		return -1;

	return CDialog::DoModal();
	
	/*
	CDialog::OnPaint();		// http://www.experts-exchange.com/Programming/System/Windows__Programming/MFC/Q_24285957.html
							// still doesn't work! (the dialog box doesn't show until I press the ALT key)
							// solution: pause the video before calling DoModal for this dialog box and play/resume after ok/cancel

	ShowWindow(SW_SHOW);
	*/
}

int CCalibrateDlg::CalculateObjectStatistics(bool update/*=false*/)
{
	CalculateColorStatistics(update);

	//update the sliders
	UpdateSliders();

	//write the color model to the hand tracker
	SetTrackerColorModel();

	return 0;
}

void CCalibrateDlg::ProcessMouseCallback(int event, int x, int y, int flags, void* param)
{
	static bool mouseLeftBtnDown = false;
	static bool newRecangleDrawn = false;

	/*
	#ifdef IMAGE_SIZE_ORIGINAL // 640*480
		y = originalImage->height - y; // reverse the direction of the vertical axis (needed)
	#else // 320*240 (image already flipped once)
		// do nothing
	#endif
	*/
	
    switch ( event )
    {
		case CV_EVENT_LBUTTONDOWN:
				mouseLeftBtnDown = true;
				newRecangleDrawn = false;
				x0 = x;
				y0 = y; 
				x1 = x0;
				y1 = y0;
				showImage = cvCloneImage(originalImage); // clear previous rectangle, if it exists
				cvShowImage(CALIB_IMAGE_OPENCV_NAME, showImage);
				cvReleaseImage(&showImage); // no longer needed
				showImage = NULL;
				break;

		case CV_EVENT_LBUTTONUP:
				mouseLeftBtnDown = false;
				if ((x0 != x1) && (y0 != y1))
				{
					newRecangleDrawn = true;
				}
				break;

		case CV_EVENT_MOUSEMOVE:
				if (mouseLeftBtnDown)
				{	
					if (y<-6500)
						y = originalImage->height;
					if (x>65000)
						x=0;

					if (x<0)
						x = 0;
					else if (x>originalImage->width)
						x = originalImage->width;
					if (y<0)
						y = 0;
					else if (y>originalImage->height)
						y = originalImage->height;

					showImage = cvCloneImage(originalImage); // clear previous rectangle, if it exists
					x1 = x;
					y1 = y; 
					//cvRectangle(showImage, cvPoint(x0, y0), cvPoint(x1, y1), CV_RGB(255,255,0), 2);
					//dxnote nov 10, 2012: show a rectangle centred at (x0, y0), and has dimension (x0-x1)*2 X (y0-y1)*2					
					cvRectangle(showImage, cvPoint((x0<x1)?(x0-(x1-x0)):x1, (y0<y1)?(y0-(y1-y0)):y1), cvPoint((x0<x1)?x1:(x0+(x0-x1)), (y0<y1)?y1:(y0+(y0-y1))), CV_RGB(255,255,0), 1); //dx nov 10, 2012
					radius = sqrt( float ((x0-x1)*(x0-x1) + (y0-y1)*(y0-y1)) );
					cvCircle(showImage, cvPoint(x0, y0), ROUND(radius), CV_RGB(255, 0, 255), 1);
					cvRectangle(showImage, cvPoint(x0-ROUND(sqrt((radius*radius)/2)), y0-ROUND(sqrt((radius*radius)/2))), cvPoint(x0+ROUND(sqrt((radius*radius)/2)), y0+ROUND(sqrt((radius*radius)/2))), CV_RGB(0, 255, 255), 1);
					
					////dxforfun oct 20, 2012
					//cvEllipse(showImage, cvPoint(x0, y0), cvSize(abs(x0-x1), abs(y0-y1)), 0, 0, 360, CV_RGB(0, 255, 255), 2);

					cvShowImage(CALIB_IMAGE_OPENCV_NAME, showImage);
					cvReleaseImage(&showImage);	
					showImage = NULL;
				}
				break;
	}

	UpdateData(FALSE);	
}


void CCalibrateDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == NULL)
	{
		return;
	}

	UINT id = pScrollBar->GetDlgCtrlID();
	//UpdateData(TRUE);

	switch (id)
	{
		case(IDC_MEAN_H_SLIDER):
				meanH = sliderMeanH.GetCurrentPos();
				objects[selectedObject].mean.h = meanH;
				break;

		case(IDC_MEAN_S_SLIDER):
				meanS = sliderMeanS.GetCurrentPos();
				objects[selectedObject].mean.s = meanS;
				break;

		case(IDC_MEAN_V_SLIDER):
				meanV = sliderMeanV.GetCurrentPos();
				objects[selectedObject].mean.v = meanV ;	
				break;

		case(IDC_VARIANCE_H_SLIDER):
				varianceH = sliderVarianceH.GetCurrentPos();
				objects[selectedObject].sig.h = varianceH;
				break;

		case(IDC_VARIANCE_S_SLIDER):
				varianceS= sliderVarianceS.GetCurrentPos();
				objects[selectedObject].sig.s = varianceS;
				break;

		case(IDC_VARIANCE_V_SLIDER):
				varianceV = sliderVarianceV.GetCurrentPos();
				objects[selectedObject].sig.v = varianceV;
				break;

		case(IDC_HIGH_H_SLIDER):
				highH = sliderHighH.GetCurrentPos();
				objects[selectedObject].high.h = highH;
				break;

		case(IDC_HIGH_S_SLIDER):
				highS = sliderHighS.GetCurrentPos();
				objects[selectedObject].high.s = highS;
				break;

		case(IDC_HIGH_V_SLIDER):
				highV = sliderHighV.GetCurrentPos();
				objects[selectedObject].high.v = highV;
				break;

		case(IDC_LOW_H_SLIDER):
				lowH = sliderLowH.GetCurrentPos();
				objects[selectedObject].low.h = lowH;
				break;

		case(IDC_LOW_S_SLIDER):
				lowS = sliderLowS.GetCurrentPos();
				objects[selectedObject].low.s = lowS;
				break;

		case(IDC_LOW_V_SLIDER):
				lowV = sliderLowV.GetCurrentPos();
				objects[selectedObject].low.v = lowV;
				break;

		case(IDC_SIZE_MIN_SLIDER):
				sizeMin = sliderSizeMin.GetPos();
				size[selectedObject].min = sizeMin;
				break;

		case(IDC_SIZE_MED_SLIDER):
				sizeMed = sliderSizeMed.GetPos();
				size[selectedObject].med = sizeMed;
				break;

		case(IDC_SIZE_MAX_SLIDER):
				sizeMax = sliderSizeMax.GetPos();
				size[selectedObject].max = sizeMax;
				break;
	}

	UpdateData(FALSE);

	/*
	objects[selectedObject].mean.h = meanH;
	objects[selectedObject].mean.s = meanS;
	objects[selectedObject].mean.v = meanV ;	

	objects[selectedObject].sig.h = varianceH;
	objects[selectedObject].sig.s = varianceS;
	objects[selectedObject].sig.v = varianceV;

	objects[selectedObject].high.h = highH;
	objects[selectedObject].high.s = highS;
	objects[selectedObject].high.v = highV;

	objects[selectedObject].low.h = lowH;
	objects[selectedObject].low.s = lowS;
	objects[selectedObject].low.v = lowV;

	size[selectedObject].min = sizeMi;n
	size[selectedObject].med = sizeMed;
	size[selectedObject].max = sizeMax;
	*/

	//SetModel();
	SetTrackerColorModel();
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CCalibrateDlg::OnBnClickedCalibrateButton()
{
	modified = true;
//	*waterDetectorInitialized = false;
//	*actionDetectorInitialized = false;
	CalculateObjectStatistics(false); // update = false
	//UpdateSliders();
	//SetModel();
}

void CCalibrateDlg::OnBnClickedCalibUpdateButton()
{
	modified = true;
	//*waterDetectorInitialized = false;
	//*actionDetectorInitialized = false;
	CalculateObjectStatistics(true); // update = true
	//UpdateSliders();
	//SetModel();
}

int CCalibrateDlg::CalculateColorStatistics(bool update/*=false*/)
{
	//NOTE: calibration stats for H channel are now calculated using
	//circular stats methods.  See "circStat" Matlab toolbox by Berens or 
	//"Statistical analysis of circular data" by Fisher for more info.

	int x, y, w, h, count=0;
	int i, j;
	double dTmp;
	double s_max = 0, v_max = 0;
	double s_min = 1, v_min = 1;
	double h_total_s = 0.0, h_total_c = 0.0, s_total = 0.0, v_total = 0.0;
	double s_total_2 = 0, v_total_2 = 0;
	static double dDegToRadRatio = 3.14159265 / 180.0;

	//if we're updating a previously computed model,
	// start from previous values
	if (update) 
	{		
		count = size[selectedObject].med;

		s_max = objects[selectedObject].high.s;
		s_min = objects[selectedObject].low.s;
		v_max = objects[selectedObject].high.v;
		v_min = objects[selectedObject].low.v;
	  
		//(note: use "circular stats" methods for H channel!)
		h_total_s = sin(objects[selectedObject].mean.h * dDegToRadRatio) * count;
		h_total_c = cos(objects[selectedObject].mean.h * dDegToRadRatio) * count;
		s_total = objects[selectedObject].mean.s*count;
		v_total = objects[selectedObject].mean.v*count;
	  
		s_total_2 = (objects[selectedObject].sig.s+square(s_total/count))*count;
		v_total_2 = (objects[selectedObject].sig.v+square(v_total/count))*count;
	}


	//get the region to operate over
	GetBox(&x, &y, &w, &h);

	//if the selected region is nothing, do nothing.
	if(w <= 0 || h <= 0)
	{
		return 1;
	}
	
	//update the hsv image with the latest image from the camera
	MakeHsvImage();

	//calculate the mean
	for (j=y; j < y+h; j++)
	{		//rows
		for (i=x; i < x+w; i++)
		{	//columns
			if( (tracker->cim[i + j*width].h) != UNDEFINED_HUE){	//if the hue is undefined, ignore the pixel

				//add up the pixel values
				//(note: use "circular stats" methods for H channel!)
				dTmp = tracker->cim[i + j*width].h;
				h_total_s += sin(dTmp * dDegToRadRatio);
				h_total_c += cos(dTmp * dDegToRadRatio);
				s_total += tracker->cim[i + j*width].s;
				v_total += tracker->cim[i + j*width].v;

				//add up the square of the pixel values
				s_total_2 += square(tracker->cim[i + j*width].s);
				v_total_2 += square(tracker->cim[i + j*width].v);

				//get the running min and max
				s_max = max(s_max, tracker->cim[i + j*width].s);
				v_max = max(v_max, tracker->cim[i + j*width].v);
				s_min = min(s_min, tracker->cim[i + j*width].s);
				v_min = min(v_min, tracker->cim[i + j*width].v);
				count+=1;
				
			}
		}
	}
	
	//calc means, and save them
	objects[selectedObject].mean.h = (atan2(h_total_s, h_total_c)) / dDegToRadRatio;  
	objects[selectedObject].mean.s = (s_total/count);
	objects[selectedObject].mean.v = (v_total/count);

	//calc sigmas, and save them
	dTmp = sqrt( square(h_total_s) + square(h_total_c) ) / count;
	objects[selectedObject].sig.h = ( 2.0 * (1.0 - dTmp) ) / square(dDegToRadRatio);
	objects[selectedObject].sig.s = ( s_total_2/count - square(s_total/count) );
	objects[selectedObject].sig.v = ( v_total_2/count - square(v_total/count) );

	//threshold values, save them
	//(Note: max and min of H channel are estimated as (Mean +/- 2*StdDeviation) because H channel is cyclic) 
	objects[selectedObject].high.h = objects[selectedObject].mean.h + (2.0*sqrt(objects[selectedObject].sig.h));   
	objects[selectedObject].high.s = s_max;
	objects[selectedObject].high.v = v_max;
	objects[selectedObject].low.h  = objects[selectedObject].mean.h - (2.0*sqrt(objects[selectedObject].sig.h));  
	objects[selectedObject].low.s  = s_min;
	objects[selectedObject].low.v  = v_min;

	if (!update) {
	  size[selectedObject].min = count/SCALE;
	  size[selectedObject].med = count;
	  size[selectedObject].max = count*SCALE*2;
	}

	/*
	//show the result
	cout << "\nCount : " << count << endl;
	cout << "Calculated values\nMean: ";
	cout << "H " << this->objects[this->selectedObject].mean.h;
	cout << ", S " << this->objects[this->selectedObject].mean.s;
	cout << ", V " << this->objects[this->selectedObject].mean.v << endl;
	cout << "Variance: ";
	cout << "H " << this->objects[this->selectedObject].sig.h;
	cout << ", S " << this->objects[this->selectedObject].sig.s;
	cout << ", V " << this->objects[this->selectedObject].sig.v << endl;
	*/

	return 0;
}

void CCalibrateDlg::UpdateSliders()
{
	meanH = objects[selectedObject].mean.h;
	meanS = objects[selectedObject].mean.s;
	meanV = objects[selectedObject].mean.v;	

	varianceH = objects[selectedObject].sig.h;
	varianceS = objects[selectedObject].sig.s;
	varianceV = objects[selectedObject].sig.v;

	highH = objects[selectedObject].high.h;
	highS = objects[selectedObject].high.s;
	highV = objects[selectedObject].high.v;

	lowH = objects[selectedObject].low.h;
	lowS = objects[selectedObject].low.s;
	lowV = objects[selectedObject].low.v;

	sizeMin = size[selectedObject].min;
	sizeMed = size[selectedObject].med;
	sizeMax = size[selectedObject].max;

	// --

	sliderMeanH.SetCurrentPos(meanH);
	sliderMeanS.SetCurrentPos(meanS);
	sliderMeanV.SetCurrentPos(meanV);

	sliderVarianceH.SetCurrentPos(varianceH);
	sliderVarianceS.SetCurrentPos(varianceS);
	sliderVarianceV.SetCurrentPos(varianceV);

	sliderHighH.SetCurrentPos(highH);
	sliderHighS.SetCurrentPos(highS);
	sliderHighV.SetCurrentPos(highV);

	sliderLowH.SetCurrentPos(lowH);
	sliderLowS.SetCurrentPos(lowS);
	sliderLowV.SetCurrentPos(lowV);

	sliderSizeMin.SetPos(sizeMin);
	sliderSizeMed.SetPos(sizeMed);
	sliderSizeMax.SetPos(sizeMax);
	// --

	UpdateData(FALSE);
}

/*
int CCalibrateDlg::LoadColorModel(CString filename)
{
	MessageBox("Not coded yet!", "oops", MB_OK);
	return 1;
}
*/

void CCalibrateDlg::MakeHsvImage()
{
	// no need for assert(IsMultipleOfFour(originalImage->width));
	// because originalImage is the same size as currentFrame (i.e. imageWidth) which is checked before
	getHSV(tracker->RGBHSVLookup, (unsigned char*)(originalImage->imageData), tracker->cim, width, height);
}

void CCalibrateDlg::SetTrackerObjectRegion(OBJECT_REGION obj_reg)//Justin - added *2
{//dxnote oct 24, 2012: radius is by tracker, width and height is used by backgroundChangeDetector, centreX,Y are used by both
	regions[obj_reg].centreX = x0*2;//x0, y0, radius are taken from the calibration window - they are in 320*240 coordinates
	regions[obj_reg].centreY = y0*2;
	regions[obj_reg].radius = radius*2;
	//dx oct 24, 2012:
	if (obj_reg == REGION_FAUCET) 
	//dx nov 10, 2012: make regions being squares subscribbing the circle, but a rectangle for faucet; centreX,Y is the centre for the circle, square and rectangle
	{
		regions[obj_reg].width = abs(x0-x1)*4;
		regions[obj_reg].height = abs(y0-y1)*4;
	}
	else
	{
		regions[obj_reg].width = regions[obj_reg].height = sqrt((regions[obj_reg].radius * regions[obj_reg].radius)/2);
		assert(regions[obj_reg].width == regions[obj_reg].height);
	}
	assert(regions[obj_reg].width >= 0);
	SetTrackerRegions();
	return;
}

void CCalibrateDlg::OnBnClickedRegionSoapButton()
{
	modified = true;
	//*waterDetectorInitialized = false;
	//*actionDetectorInitialized = false;
	SetTrackerObjectRegion(REGION_SOAP);
	SetTrackerModel();
}

void CCalibrateDlg::OnBnClickedRegionSoapSpoutButton()//Modification for soap
{
	modified = true;
	//*waterDetectorInitialized = false;
	//*actionDetectorInitialized = false;
	SetTrackerObjectRegion(REGION_SOAP_SPOUT);
	SetTrackerModel();
}

void CCalibrateDlg::OnBnClickedRegionRightTapButton()
{
	modified = true;
	//*waterDetectorInitialized = false;
	//*actionDetectorInitialized = false;
	SetTrackerObjectRegion(REGION_TAP_RIGHT);
	SetTrackerModel();
}

void CCalibrateDlg::OnBnClickedRegionLeftTapButton()
{
	modified = true;
	//*waterDetectorInitialized = false;
	//*actionDetectorInitialized = false;
	SetTrackerObjectRegion(REGION_TAP_LEFT);
	SetTrackerModel();
}

void CCalibrateDlg::OnBnClickedRegionTowelButton()
{
	modified = true;
	//*waterDetectorInitialized = false;
	//*actionDetectorInitialized = false;
	SetTrackerObjectRegion(REGION_TOWEL);
	SetTrackerModel();
}

void CCalibrateDlg::OnBnClickedRegionSinkButton()
{
	modified = true;
	//*waterDetectorInitialized = false;
	//*actionDetectorInitialized = false;
	SetTrackerObjectRegion(REGION_SINK);
	SetTrackerModel();
}

void CCalibrateDlg::OnBnClickedRegionWaterButton()
{
	modified = true;
	//*waterDetectorInitialized = false;
	//*actionDetectorInitialized = false;
	SetTrackerObjectRegion(REGION_WATER);
	SetTrackerModel();
}

void CCalibrateDlg::OnBnClickedObjectTowelRadio()
{
	selectedObject = OBJECT_TOWEL;
	UpdateSliders();
}

void CCalibrateDlg::OnBnClickedObjectHandRadio()
{
	selectedObject = OBJECT_HAND;
	UpdateSliders();
}

void CCalibrateDlg::OnBnClickedObjectSoapRadio()//Modification for soap
{
	selectedObject = OBJECT_SOAP;
	UpdateSliders();
}

void CCalibrateDlg::OnBnClickedLoadCalibButton()
{
	LoadCalib();
}

void CCalibrateDlg::OnBnClickedSaveCalibButton()
{
	SaveCalib();
}

ObjectRegion CCalibrateDlg::GetRegion(OBJECT_REGION k)
{
	if (k>=NUM_REGIONS)
	{
		ObjectRegion empty;
		return empty;
	}

	return regions[k];
}
void CCalibrateDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	modified = false;
}

BOOL CCalibrateDlg::PreTranslateMessage(MSG* pMsg)
{
	if (m_hAccelTable) {
		if (::TranslateAccelerator(m_hWnd, m_hAccelTable, pMsg)) {
			return(TRUE);
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CCalibrateDlg::OnCalibFileOpen()
{
	LoadCalib();
}

void CCalibrateDlg::OnCalibFileSave()
{
	SaveCalib();
}

void CCalibrateDlg::OnCalibViewRegions()
{
	CvFont waitfont;
	cvInitFont(&waitfont,CV_FONT_HERSHEY_SIMPLEX ,0.5f, 0.5f, 0, 1);  //Initialise font

	showImage = cvCloneImage(originalImage); // clear previous rectangle, if it exists
	

	for (int ii=0; ii<NUM_REGIONS; ii++)
	{

		char regionLabel[100];
		switch (ii)
		{
		case REGION_SOAP:
			sprintf_s(regionLabel, "soap");
			break;

		case REGION_SOAP_SPOUT://Modification for soap
			sprintf_s(regionLabel, "soap spout");
			break;

		case REGION_TAP_LEFT:
			sprintf_s(regionLabel, "left tap");
			break;

		case REGION_TAP_RIGHT:
			sprintf_s(regionLabel, "right tap");
			break;

		case REGION_WATER:
			sprintf_s(regionLabel, "water");
			break;

		case REGION_SINK:
			sprintf_s(regionLabel, "sink");
			break;

		case REGION_TOWEL:
			sprintf_s(regionLabel, "towel");
			break;

		case REGION_FAUCET: //dx oct 24, 2012
			sprintf_s(regionLabel, "faucet");
			break;

		default:
			sprintf_s(regionLabel, "unknown");
			break;
		}
	


		if (ROUND(regions[ii].radius)==0)
		{
		}
		else
		{
			//Modified August 20, 2010 - Justin Bimbrahw - added (/2) to centres and radii
			cvCircle(showImage, cvPoint(regions[ii].centreX/2, regions[ii].centreY/2), ROUND(regions[ii].radius/2), CV_RGB(200, 50, 50), 2);
			cvPutText(showImage, regionLabel, cvPoint(regions[ii].centreX/2-ROUND(regions[ii].radius/2*0.3), regions[ii].centreY/2+ROUND(regions[ii].radius/2*1.05)), &waitfont, CV_RGB(255,255,0));
		}

	}

	cvShowImage(CALIB_IMAGE_OPENCV_NAME, showImage);
	cvReleaseImage(&showImage);	
	showImage = NULL;

}

void CCalibrateDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	mainCalibMenu.DestroyMenu();
}


void CCalibrateDlg::OnBnClickedRegionFaucetButton()
{
	//dx oct 24, 2012
	//GetBox(&faucetx, &faucety, &faucetw, &fauceth);
	//CString sss;
	//sss.Format("%d %d %d %d", faucetx, faucety, faucetw, fauceth);
	//AfxMessageBox("Faucet box has been set: "+sss);//dx oct 24, 2012
	////::MessageBox(NULL, sss, "Faucet box has been set",MB_OK);
	//regionset = true;

	modified = true;
	//*waterDetectorInitialized = false;
	//*actionDetectorInitialized = false;
	SetTrackerObjectRegion(REGION_FAUCET);
	SetTrackerModel();
}
