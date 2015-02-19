#pragma once


#include "VidPlayerDefines.h"
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;



class CWaterDetector
{
public:
	CWaterDetector(void);
	virtual ~CWaterDetector(void);

	void Init(int waterX, int waterY, float waterRadius);
	float EstimateWaterStatus(IplImage* currentFrame, unsigned char*  skinImageRaw, unsigned int imWidth, unsigned int imHeight);
	void SaveVideoFeatures(CString filename);

	friend class CRERCAnnotatorDoc;

	int GetHistogramCount(); // return the # bins in each histogram (e.g. 256)

protected:

	void CleanUp(void);

public:
	void ComputeVideoFeatures(IplImage* currentFrame, unsigned char* skinImageRaw, int testCase, bool write = false,  int gt_label = 0);//Feb. 23, 2011
	
protected:
	float EstimateWaterOnUsingVideoFeatures(); // returns the probability of water being on
	#ifdef PLAY_AND_PROCESS_AUDIO
		void ComputeAudioFeatures();
		float EstimateWaterOnUsingAudioFeatures(int currentFrameNo);
	#endif

	/*
	unsigned int centreX;
	unsigned int centreY;
	unsigned int radius;
	unsigned int upperLeftX;
	unsigned int upperLeftY;
	unsigned int lowerRightX;
	unsigned int lowerRightY;
	*/

	// (intermediate) for image processing
	IplImage* currentWaterImg;		// time: now
	IplImage* prevWaterImg;			// time: now - 1 frame
	IplImage* prev2WaterImg;		// time: now - 2 frame
	IplImage* waterDiffImg;			// 1st order derivative (colour)
	IplImage* waterDiffImgBW;		// 1st order derivative (B&W)
	IplImage* waterDiff2Img;		// 2nd order derivative (colour)
	IplImage* waterDiff2ImgBW;		// 2nd order derivative (B&W)
	IplImage* skinMaskImg;			// skin (=hand) region (colour) (time: now)
	IplImage* currentSkinMaskImgBW;	// skin region (B&W) (time: now)
	IplImage* prevSkinMaskImgBW;	// prev sking region (B&W) (time: now-1)
	IplImage* prev2SkinMaskImgBW;	// prev prev sking region (B&W) (time: now-2)
	CvRect waterRect;				// water region (from calib data. approximation: circle->rectangle) NOTE: this might not necessarily be the same as the water region in the calib data. e.g. it's size is currently the avg between the sink region and the water region
	CvMat* highPassFilterKernel;	// simple 3x3 HPF kernel
	CvHistogram* hist1;				// histogram of the first order gradiant
	CvHistogram* hist2;				// histogram of the second order gradiant

	// video features
public://Justin
	int maskedRegion1Size;			// size of the masked region (=hand = skin) (for 1st order temporal derivative) (if high: not much left to see: trust earlier decisions & audio decisions more)
	int maskedRegion2Size;			// size of the masked region (=hand = skin) (for 2nd order temporal derivative) (if high: not much left to see: trust earlier decisions & audio decisions more)

//Justin
public:

	#ifdef VIDEO_FEATURES_EXPORT_HISTOGRAM_MOMENTS
		double firstOrderDerivativeHistFirstMoment;
		double firstOrderDerivativeHistSecondMoment;
		double firstOrderDerivativeHistThirdMoment;
		double secondOrderDerivativeHistFirstMoment;
		double secondOrderDerivativeHistSecondMoment;
		double secondOrderDerivativeHistThirdMoment;
		double entropy1;
		double entropy2;
	#endif


	#ifdef SAVE_INTERMEDIATE_WATER_DETECTION_VIDEOS
		CvVideoWriter *vidWriter;
	#endif

	// (intermediate) for audio processing

	// audio features

	// decisions
	float waterOnProbabilityFromVideo;	// probability (in [0, 1])
	#ifdef PLAY_AND_PROCESS_AUDIO
		float waterOnProbabilityFromAudio;	// probability (in [0, 1])
	#endif
	float waterOnProbability;			// combined audio & video (in [0, 1])

public:
	int bins_left[8];
	int bins_right[8];
	double lefttapfeatureweight[8];
	double righttapfeatureweight[8];

	//bools
	bool water_images_set;

	//counters
	int num_images_set;


public:
	//September 27, Justin
	void UpdateImages(IplImage* currentFrame);
	void SetWaterImages(IplImage* currentFrame);
	void EndWaterDetection();
	void DebugWindow(CString windowname, IplImage*& im);
};



	
		