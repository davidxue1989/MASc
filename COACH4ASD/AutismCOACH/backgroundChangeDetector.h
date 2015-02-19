/*
oct 23, 2012: backgroundChangeDetector

implement backgroundChangeDetector class:
GetBackground
ComputeVideoFeatures (logFeatures)
GetRegionForeground
GetTapFeatures (GetFeatures)
ResetTapFeatures (ResetFeatures)
convert region (index) to region name (string);
and instantiate objects: waterDetectorL, waterDetectorR from it; make it work for the current code (compare with waterDetector)
with fields:
	float changeDetectedProbability;			// combined audio & video (in [0, 1])
	int bins[8];
	double featureweights[8];
	int regionIndex;
derive from it faucetOcclusionDetector, towelLiftDetector

dxtodo:
oct 23, 2012:
ensure when region position is changed, all region background images are updated (save new backgrnd imges to files)
reread weight from file before each prediction
*/

#pragma once

#include "CalibrateDlg.h"

class backgroundChangeDetector
{
public:
	bool first_iteration;

	//public functions
	backgroundChangeDetector(OBJECT_REGION regionIndex, CCalibrateDlg *pCalibDlg);
	~backgroundChangeDetector();
	float predictProbabilityOfBackgroundChange(IplImage *currentFrame);
	void predictProbabilityOfBackgroundChange_init(IplImage *currentFrame, bool isTraining);
	void loadFeatureWeights();

	//setters & getters
	void setIsTraining(bool isTraining) { this->isTraining = isTraining; }
	//bool getIsTraining() { return isTraining; }
	//void setCurrentFrame(IplImage *currentFrame) { this->currentFrame = currentFrame; }
	void setGt_label (int gt_label) { this->gt_label = gt_label; }


protected:

private:
	//fields
	OBJECT_REGION regionIndex;
	bool isTraining;
	int gt_label;
	int testCase;
	CCalibrateDlg *pCalibDlg; //dxnote oct 24, 2012: has to be pointer, we don't want to make a local copy of pCalibDlg

	float probabilityOfBackgroundChange_processedFrame;
	int bins[8];
	float featureWeights[8];

	IplImage *currentFrame;
	IplImage *gray_backgroundImage;
	IplImage *gray_currentFrame;
	IplImage *gray_foregroundImage;

	//helper functions
	void initializeImages(CvSize size, int depth, int channels);
	void initializeImages_helper(IplImage *&image, CvSize size, int depth, int channels);
	void setImageROI(IplImage *image);
	void GetBackground();
	void logFeaturesTraining(int gt_label);
	void GetRegionForeground();
	void GetFeatures();
	bool IsAlignByte(int count, int width, int widthstep);
	int Magnitude(int number);
	CString getRegionName();
	void safe_release_image(IplImage*& im);

};