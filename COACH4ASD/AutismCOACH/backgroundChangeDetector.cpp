#include "stdafx.h"
#include "backgroundChangeDetector.h"

#include <iostream>
#include <fstream>
using namespace std;

#define RESOURCE_FILES_PATH "C:\\dxFiles\\COACH4ASDresourceFiles\\"

void backgroundChangeDetector::safe_release_image(IplImage*& im)
{
	if (im != NULL)
	{
		cvReleaseImage(&im);
		im = NULL;
	}
}
backgroundChangeDetector::backgroundChangeDetector(OBJECT_REGION regionIndex, CCalibrateDlg *pCalibDlg)
	: regionIndex(regionIndex), isTraining(false), pCalibDlg(pCalibDlg), testCase(0), gt_label(0), first_iteration(true)
{
	currentFrame = gray_backgroundImage = gray_currentFrame = gray_foregroundImage = NULL;
}
backgroundChangeDetector::~backgroundChangeDetector()
{
	//safe_release_image(currentFrame);
	safe_release_image(gray_backgroundImage);
	safe_release_image(gray_currentFrame);
	safe_release_image(gray_foregroundImage);

}
void backgroundChangeDetector::initializeImages(CvSize size, int depth, int channels)
{
	//initializeImages_helper(currentFrame, size, depth, channels); //dxnote oct 24, 2012: currentFrame is never a local copy, we just save a pointer to the one in pDoc
	initializeImages_helper(gray_backgroundImage, size, depth, channels);	
	initializeImages_helper(gray_currentFrame, size, depth, channels);
	initializeImages_helper(gray_foregroundImage, size, depth, channels);
}	
void backgroundChangeDetector::initializeImages_helper(IplImage *&image, CvSize size, int depth, int channels)
{
	if (image == NULL)//if not created already
	{
		image = cvCreateImage(size, depth, channels);
	}
}

void backgroundChangeDetector::setImageROI(IplImage *image)
{
	//Get region centre and radius
	//Go to currentframe, from x = centre - r, y = centre - r to x = centre + r, y = centre + r
	//and copy the values to an array/IplImage

	int xc = pCalibDlg->regions[regionIndex].centreX;
	int yc = pCalibDlg->regions[regionIndex].centreY;
	//dx oct 24, 2012:
	//int radius = (int) pCalibDlg->regions[regionIndex].radius;
	int width = pCalibDlg->regions[regionIndex].width;
	int height = pCalibDlg->regions[regionIndex].height;
	//int startx = xc - radius;
	//int starty = yc - radius;
	//int stopx = xc + radius;
	//int stopy = yc + radius;
	//int width = stopx - startx + 1;
	//int height = stopy - starty + 1;
	int startx = xc - width;
	int starty = yc - height;

	//cvShowImage("currentFrame", image);
	//cvSetImageROI(image, cvRect(startx, starty, width, height) );
	cvSetImageROI(image, cvRect(startx, starty, width*2, height*2) ); //dx oct 24, 2012
	//cvShowImage("currentFrameROI", image);
}


void backgroundChangeDetector::GetBackground()
{
	setImageROI(currentFrame);

	//Set background

	if (isTraining)
	{
		//gray_backgroundImage = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth, 1);
		//IplImage *temp = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth, currentFrame->nChannels);
		//cvCopyImage(currentFrame, temp);
		//gray_backgroundImage = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth, 1);
		cvCvtColor(currentFrame, gray_backgroundImage, CV_BGR2GRAY);
		cvSaveImage(RESOURCE_FILES_PATH + getRegionName() + "_BackgroundImage.jpg", gray_backgroundImage);
	}
	else
	{
		IplImage* tempImage = cvLoadImage(RESOURCE_FILES_PATH + getRegionName() + "_BackgroundImage.jpg");
		//gray_backgroundImage = cvCreateImage(cvGetSize(tempImage), tempImage->depth, 1);
		cvCvtColor(tempImage, gray_backgroundImage, CV_BGR2GRAY);
		safe_release_image(tempImage);
	}
	cvResetImageROI(currentFrame);	
}

void backgroundChangeDetector::logFeaturesTraining(int gt_label)
{
	ofstream logfeatures(RESOURCE_FILES_PATH + getRegionName() + "_features.csv", ios::app);

	//dx nov 24, 2012: changed to csv file extension, and don't do the header row
	if(testCase == 0) 
	{//dxnote nov 24, 2012: do nothing in first interation, since data is corrupted		
	//{//write the header row
	//	logfeatures << "\t";
	//	for (int i=0; i<8; i++)
	//	{
	//		CString label;
	//		label.Format("%s%d\t", getRegionName(), i);
	//		logfeatures << label;
	//	}
	//	logfeatures<< "Ground Truth Label" << endl;//Feb. 23, 2011
	}
	else
	{//write the data rows
		logfeatures << testCase << "\t";
		for (int i=0; i<8; i++)
		{
			logfeatures << bins[i] << "\t";
		}
		logfeatures << gt_label << endl;
	}
	logfeatures.close();
}

void backgroundChangeDetector::GetRegionForeground()
{
	setImageROI(currentFrame);

	//gray_currentFrame = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth, 1);
	//safe_release_image(gray_foregroundImage);//This line MUST preceed the next!
	//gray_foregroundImage = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth, 1);
	//dxnote oct 23, 2012: why try to clear gray_foregroundImage? isn't rewritten automatically by cvAbsDiff(gray_currentFrame, gray_backgroundImage, gray_foregroundImage);?

	cvCvtColor(currentFrame, gray_currentFrame, CV_BGR2GRAY);
	cvResetImageROI(currentFrame);
	//Get the foreground
	cvAbsDiff(gray_currentFrame, gray_backgroundImage, gray_foregroundImage);

	//dxdebug oct 24, 2012:
	cvSaveImage(RESOURCE_FILES_PATH + getRegionName() + "_gray_currentFrame.jpg", gray_currentFrame);
	cvSaveImage(RESOURCE_FILES_PATH + getRegionName() + "_gray_foregroundImage.jpg", gray_foregroundImage);
}

void backgroundChangeDetector::GetFeatures()
{
	//clears the bins (i.e. resets the featuers)
	for (int i=0; i<8; i++)
	{
		bins[i] = 0;
	}

	for (int count = 0; count < gray_foregroundImage->imageSize; count++)
	{
		if( !IsAlignByte(count, gray_foregroundImage->width, gray_foregroundImage->widthStep) )
		{
			//count number of elements in each bin of histogram
			//8 bins for 129 possible values - each bin holds 16 values (17 in last bin)

			int imgdatum = Magnitude( (int)gray_foregroundImage->imageData[count] );

			if (imgdatum > 128-1*16)
				bins[7] ++;
			else if (imgdatum > 128-2*16)
				bins[6] ++;
			else if (imgdatum > 128-3*16)
				bins[5] ++;
			else if (imgdatum > 128-4*16)
				bins[4] ++;
			else if (imgdatum > 128-5*16)
				bins[3] ++;
			else if (imgdatum > 128-6*16)
				bins[2] ++;
			else if (imgdatum > 128-7*16)
				bins[1] ++;
			else if (imgdatum >= 128-8*16)
				bins[0] ++;
		}
	}
}

bool backgroundChangeDetector::IsAlignByte(int count, int width, int widthstep)
{
	int column = (count) % widthstep; 
	if ( column > (width - 1) )
		return true;
	return false;
}
int backgroundChangeDetector::Magnitude(int number)
{
	if (number < 0)
		number = -1*number;
	return number;
}

CString backgroundChangeDetector::getRegionName()
{
	CString name;
	switch (regionIndex)
	{
	case REGION_SOAP:
		name = "REGION_SOAP";
		break;
	case REGION_TAP_LEFT:
		name = "REGION_TAP_LEFT";
		break;
	case REGION_TAP_RIGHT:
		name = "REGION_TAP_RIGHT";
		break;
	case REGION_WATER:
		name = "REGION_WATER";
		break;
	case REGION_SINK:
		name = "REGION_SINK";
		break;
	case REGION_TOWEL:
		name = "REGION_TOWEL";
		break;
	case REGION_SOAP_SPOUT:
		name = "REGION_SOAP_SPOUT";
		break;
	case REGION_FAUCET:
		name = "REGION_FAUCET";

	}
	return name;
}

float backgroundChangeDetector::predictProbabilityOfBackgroundChange(IplImage *currentFrame)
{
	float probability = 0;
	this->currentFrame = currentFrame;

	GetRegionForeground();
	GetFeatures();
	if (isTraining)
		logFeaturesTraining(gt_label);

	loadFeatureWeights(); //oct 25, 2012: reload feature weights everytime to make sure it's using the most updated files

	for (int i = 0; i<8; i++)
	{
		probability += bins[i]*featureWeights[i];
	}
	float e = 2.1873;
	probability = -1*probability;
	probability = 1 / (1 + pow(e, probability));

	testCase++;

	probabilityOfBackgroundChange_processedFrame = probability;
	return probability;
}
void backgroundChangeDetector::predictProbabilityOfBackgroundChange_init(IplImage *currentFrame, bool isTraining)
{
	this->isTraining = isTraining;
	testCase = 0;
	setImageROI(currentFrame);
	initializeImages(cvGetSize(currentFrame), currentFrame->depth, 1); //dxnote oct 24, 2012: channels = 1 for grayscale images

	cvResetImageROI(currentFrame);
	
	this->currentFrame = currentFrame;

	////dx debug oct 24, 2012:
	//cvSaveImage(RESOURCE_FILES_PATH + getRegionName() + "_currentFrame.jpg", currentFrame);		
	////dx debug oct 24, 2012:
	//cvSaveImage(RESOURCE_FILES_PATH + getRegionName() + "_this-currentFrame.jpg", this->currentFrame);

	GetBackground();
	if (isTraining)
		logFeaturesTraining(gt_label);
	testCase++;
}

//dx oct 25, 2012
void backgroundChangeDetector::loadFeatureWeights()
{	
	////dxdebug oct 24, 2012:
	//ofstream createTheFile(RESOURCE_FILES_PATH + getRegionName() + "_featureWeights.txt", ios::app);
	//createTheFile.close();

	ifstream featureWeightFile(RESOURCE_FILES_PATH + getRegionName() + "_featureWeights.txt");
	for (int i=0; i<8; i++)
	{
		featureWeightFile >> featureWeights[i];
	}
	featureWeightFile.close();
}