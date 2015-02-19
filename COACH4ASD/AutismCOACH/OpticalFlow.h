#pragma once

#include <iostream>
#include <fstream>

#include <deque>
#include <afxmt.h> //for multithreading, CCriticalSection

using namespace std;

class COpticalFlow
{
//Members
public:
	IplImage* m_prevImg;
	IplImage* gray_m_prevImg;
	IplImage* m_currentImg;
	IplImage* gray_m_currentImg;

	IplImage* eig_image;
	IplImage* tmp_image;
	IplImage* pyrA;
	IplImage* pyrB;

	bool firstcall;

	//create an array to store the values of angles [360]
	int angles[360];//stores the frequency of each angle from 0 to 360
	float distances[360];//stores the greatest distance travelled in the direction of a particular angle//Feb. 1, 2011
	int distancesCount1[360];
	int distancesCount2[360];

	double prev_angles[360];
	float prev_distances[360];

	int smoothedsignal[7];

	//int numfeaturesinlast7frames; //Commented out January 10, 2012

	static const int Tau = 10;

	int iterations;

	int variablethreshold;//Dec. 17, 2010

	bool RUBBINGHANDSDETECTEDBYOPTICALFLOW;//Dec. 21

	float distance;//Feb. 1, 2011

	float scrubweights[4];//Feb. 2, 2011


public:
	COpticalFlow(void);
	~COpticalFlow(void);
	void DoOpticalFlow(IplImage*& currentImg);
	void DoAngularHistogram(int corner_count, char features_found[], float feature_errors[], CvPoint2D32f* cornersA, CvPoint2D32f* cornersB, bool writetofile = false);
	void DoVelocityHistogram(int corner_count, char features_found[], float feature_errors[ ], CvPoint2D32f* cornersA, CvPoint2D32f* cornersB);
	bool ThresholdVelocity(double rise, double run,  int lowthreshold, int highthreshold);
	void ThresholdArray(int* array, int arraylength, int threshold);
	void PlotAngularHistogram();
	void CleanUp();

	ofstream scrublogFileO;//Jan 5

	//dx aug 23, 2012: implement a queue using array for remembering the past 10? frames' of binned histogram for angles and distance
	//queue<double[36]> HistAngles;
	//deque<int*> HistAngles;
	//deque<float*> HistDistances;
	//deque<int*> HistDistanceCounts1;
	//deque<int*> HistDistanceCounts2;

	deque<float> distanceQueue;
	deque<float> angleQueue;
	
	deque<bool> scrubbingDetectedFramesQueue;
	
	int numfeaturesactivated;
	int numDistFeaturesActivated1;
	int numDistFeaturesActivated2;

	int anglesActivated;
	int anglesActivatedS;
	int anglesActivatedL;

	//dx sep 16, 2012:
	ofstream dataFrameFile;
	bool saveDataFrame_scrubTraining;

	int corner_count;

	//dxdebug sep 17, 2012:
	//struct FRAME {
	//	queue<float> queueDistances;
	//	queue<float> queueAngles;
	//};
	//FRAME* frame;
	//queue<FRAME> queueFrames;
	//queue<FRAME> temp1;
	
	double probabilityOfScrubbing;

	//dx oct 20, 2012:
	CCriticalSection csDataFrameFile;

	//dx oct 20, 2012:
	void helperWriteDataFrameFile (ofstream &dataFrameFile, deque<float> distanceQueue, deque<float> angleQueue);

	struct FRAME {
		deque<float> queueDistances;
		deque<float> queueAngles;
		deque<int> nAngleBinsActivated;
	};
	deque<FRAME> queueFrames;

	void countNAngleBinsActivated(deque<FRAME> &queueFrames, int nDistanceBins, int nAngleBins, deque<int> binDistanceBoundaries);
	
	//dx nov 29, 2012
	deque<CvPoint2D32f> cornersA_copy, cornersB_copy; //saves the corners for later to be displayed
};
