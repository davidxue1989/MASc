///* dxnote aug 30, 2012: //dxnote sep 27, 2012: these notes are outdated
// * This is for scrubbing detection using optical flow.  The parameters to play with are: 
// * number of frames in the stack => for smoothing effect, will create delay on scrubbing detection
// * =====> #define NFRAMES 5
// * distance range for feature 1
// * distance range for feature 2
// * (ThresholdVelocity(rise, run, 15, 50) || ThresholdVelocity(rise, run, 75, 160))  
// *			if (distance>15 && distance<50)
// *			else if (distance>75 && distance<160)
// * counts range for feature 1
// * counts range for feature 2
// *		if (numDistFeaturesActivated1>100 && numDistFeaturesActivated1<150 
// *		&& numDistFeaturesActivated2>7 && numDistFeaturesActivated2<20 )
// * These parameters will change depending on the computer's camera frame/image processing rate.
// * To calibrate: 
// * 1) make sure skin tracking is properly calibrated, 
// * 2) do two sets of scrubbing, one incorrect/doing random waving, the other correct scrubbing [may consider two runs for each set, one with prompts set to picture only and 30s apart, the other video plus verbal and 1s apart)
// * look at numfeaturesactivateddata.txt, identify the other parameters
// * the number of frames in the stack should be set so to create a delay around 1sec
// * distance ranges can be seen in HistDistances (or tempDistances)
// * counts range can then be seen in numDistFeaturesActivated1 and numDistFeaturesActivated2
// */

#include "StdAfx.h"
#include "OpticalFlow.h"
#include "AutismCOACHDoc.h"
#include "math.h"

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#include <time.h>

#include <MMSystem.h> // for playing a .wav file
#pragma comment(lib, "winmm.lib") // for playing a .wav file
#include <windows.h>

const int MAX_CORNERS = 500;
#define PI 3.141592
//#define NFRAMES 5
#define NFRAMES 1 //dx oct 19, 2012: changed to 1, do the aggregation of results of past number of frames in pDoc instead of opticalFlow


bool velocitythresholding = true;
bool bincountthresholding = false;

COpticalFlow::COpticalFlow(void)
{
	m_prevImg = NULL;
	gray_m_prevImg = NULL;
	m_currentImg = NULL;
	gray_m_currentImg = NULL;

	eig_image = NULL;
	tmp_image = NULL;
	pyrA = NULL;
	pyrB = NULL;

	//bools
	firstcall = true;

	//set all elements to zero
	for (int i = 0; i<360; i++)
	{
		angles[i] = 0;
		prev_angles[i] = 0;

		distances[i] = 0;//Feb. 1, 2011
		prev_distances[i] = 0;//Feb. 1, 2011
		distancesCount1[i] = 0;
		distancesCount2[i] = 0;

	}

	//for (int i = 0; i<7; i++)//Commented out January 10, 2012
	//{
	//	smoothedsignal[i] = 4;//Jan 6
	//}

	iterations = 0; //dxnote aug 22, 2012: not used...

	variablethreshold = 0;

	RUBBINGHANDSDETECTEDBYOPTICALFLOW = false;//Dec.21
	
	numfeaturesactivated = 0;
	numDistFeaturesActivated1 = 0;
	numDistFeaturesActivated2 = 0;

	//dataFrameReady = true; //dxnote sep 24, 2012: this is set to true to prevent opticalFlowDataFrameFile being written if we are not doing TRAINSCRUBBING
	saveDataFrame_scrubTraining = false; //dx oct 20, 2012
}

COpticalFlow::~COpticalFlow(void)
{
	CleanUp();
}

void COpticalFlow::CleanUp()
{
	safe_release_image(m_prevImg);
	safe_release_image(gray_m_prevImg);
	safe_release_image(m_currentImg);
	safe_release_image(gray_m_currentImg);
	safe_release_image(eig_image);
	safe_release_image(tmp_image);
	safe_release_image(pyrA);
	safe_release_image(pyrB);

	//while(!HistAngles.empty())
	//{
	//	delete[] HistAngles.front();
	//	HistAngles.pop_front();
	//}
	//while(!HistDistances.empty())
	//{
	//	delete[] HistDistances.front();
	//	HistDistances.pop_front();
	//}
	//while(!HistDistanceCounts1.empty())
	//{
	//	delete[] HistDistanceCounts1.front();
	//	HistDistanceCounts1.pop_front();
	//}	
	//while(!HistDistanceCounts2.empty())
	//{
	//	delete[] HistDistanceCounts2.front();
	//	HistDistanceCounts2.pop_front();
	//}
	while(!scrubbingDetectedFramesQueue.empty())
	{
		scrubbingDetectedFramesQueue.pop_front();
	}

	//dx nov 29, 2012	
	while (!cornersA_copy.empty())
		cornersA_copy.pop_front();
	while (!cornersB_copy.empty())
		cornersB_copy.pop_front();
}


void COpticalFlow::DoOpticalFlow(IplImage*& currentImg)
	//This function was copied from page 332 of "Learning OpenCV" -refer to Lucas Kanade optical flow method
{
	//if this is the first call to the function - create images (4)
	if (firstcall)
	{
		//clean the queues (and probably the images too
		CleanUp();

		m_prevImg = cvCreateImage(cvSize( currentImg->width,  currentImg->height),  currentImg->depth,  currentImg->nChannels);
		gray_m_prevImg = cvCreateImage(cvSize( currentImg->width,  currentImg->height),  currentImg->depth, 1);

		m_currentImg = cvCreateImage(cvSize(currentImg->width, currentImg->height), currentImg->depth, currentImg->nChannels);
		gray_m_currentImg = cvCreateImage(cvSize(currentImg->width, currentImg->height), currentImg->depth, 1);

		cvCopy(currentImg, m_currentImg);	

		//loopNowTime = GetTickCount();
		//loopRateFile << "\t\t" << loopNowTime - loopStartTime << ", done firstcall" << endl;
		//loopStartTime = GetTickCount();

		firstcall = false;
		return;
	}

	ofstream loopRateFile ("C:\\dxFiles\\COACH4ASDresourceFiles\\loopRate.txt", ios::app);
	DWORD loopStartTime, loopNowTime;
	loopRateFile << "\t\tIn DoOpticalFlow: " << endl;
	loopStartTime = GetTickCount();

	//Otherwise update previous image

	cvZero(m_prevImg);
	cvCopy(m_currentImg, m_prevImg);

	//Update current Image
	cvCopy(currentImg, m_currentImg);

	//Convert images to grayscale for LK optical flow to work
	cvConvertImage(m_prevImg, gray_m_prevImg);
	cvConvertImage(m_currentImg, gray_m_currentImg);

	CvSize img_sz = cvGetSize( gray_m_prevImg );
	int win_size = 10;

	// The first thing we need to do is get the features
	// we want to track.
	if (eig_image == NULL)
		eig_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
	if (tmp_image == NULL)
		tmp_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
	corner_count = MAX_CORNERS;
	CvPoint2D32f* cornersA = new CvPoint2D32f[ MAX_CORNERS ];	

	loopNowTime = GetTickCount();
	loopRateFile << "\t\t" << loopNowTime - loopStartTime << ", pre cvGoodFeaturesToTrack" << endl;
	loopStartTime = GetTickCount();

	//dxnote nov 29, 2012: need to find the
	// maximum quality_level (specific to hand, invariant to similar coloured objects? tune by wearing something colour similar, or put something colour similar in scene), 
	// maximum min_distance (get rid of noises, depends on camera's distance to hand), 
	// X	and minimum corner_count (should be faster if asking for less corners to track, right?==> not really)
	////cvGoodFeaturesToTrack(gray_m_prevImg, eig_image, tmp_image, cornersA, &corner_count, 0.01, 1.0 /*changed by Justin*/);
	//cvGoodFeaturesToTrack(gray_m_prevImg, eig_image, tmp_image, cornersA, &corner_count, 0.01, 10); //dx nov 23, 2012: min_distance to 5 cuz 1 is too slow
	//corner_count = 10;
	cvGoodFeaturesToTrack(gray_m_prevImg, eig_image, tmp_image, cornersA, &corner_count, 0.01, 10);//dxscrub

	loopNowTime = GetTickCount();
	loopRateFile << "\t\t" << loopNowTime - loopStartTime << ", done cvGoodFeaturesToTrack" << endl;
	loopStartTime = GetTickCount();


	//Refine Corner Locations
	cvFindCornerSubPix(
		gray_m_prevImg,
		cornersA,
		corner_count,
		cvSize(win_size,win_size),
		cvSize(-1,-1),
		cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03)
		);

	loopNowTime = GetTickCount();
	loopRateFile << "\t\t" << loopNowTime - loopStartTime << ", done cvFindCornerSubPix" << endl;
	loopStartTime = GetTickCount();



	// Call the Lucas Kanade algorithm
	char features_found[ MAX_CORNERS ];
	float feature_errors[ MAX_CORNERS ];

	CvSize pyr_sz = cvSize( gray_m_prevImg->width+8, gray_m_currentImg->height/3 );
	if (pyrA == NULL)
		pyrA = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
	if (pyrB == NULL)
		pyrB = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
	CvPoint2D32f* cornersB = new CvPoint2D32f[ MAX_CORNERS ];	

	loopNowTime = GetTickCount();
	loopRateFile << "\t\t" << loopNowTime - loopStartTime << ", pre cvCalcOpticalFlowPyrLK" << endl;
	loopStartTime = GetTickCount();


	cvCalcOpticalFlowPyrLK(
		gray_m_prevImg,
		gray_m_currentImg,
		pyrA,
		pyrB,
		cornersA,//previous features
		cornersB,//current features
		corner_count,
		cvSize( win_size, win_size ),
		5,
		features_found,
		feature_errors,
		cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 ),
		0
		);

	loopNowTime = GetTickCount();
	loopRateFile << "\t\t" << loopNowTime - loopStartTime << ", done cvCalcOpticalFlowPyrLK" << endl;
	loopStartTime = GetTickCount();


	// Now make some image of what we are looking at:
	DoAngularHistogram(corner_count, features_found, feature_errors, cornersA, cornersB, true);
	//DoVelocityHistogram(corner_count, features_found, feature_errors, cornersA, cornersB);

	loopNowTime = GetTickCount();
	loopRateFile << "\t\t" << loopNowTime - loopStartTime << ", done rest of DoOpticalFlow" << endl;
	loopStartTime = GetTickCount();

	loopRateFile.close();

	//dx nov 29, 2012: save the corners
	while (!cornersA_copy.empty())
		cornersA_copy.pop_front();
	while (!cornersB_copy.empty())
		cornersB_copy.pop_front();
	for (int i=0; i<corner_count; i++)
	{
		cornersA_copy.push_back(cornersA[i]);
		cornersB_copy.push_back(cornersB[i]);
	}

	delete [] cornersA;
	delete [] cornersB;
}

//Create histogram of orientation of lines
void COpticalFlow::DoAngularHistogram(int corner_count, char features_found[], float feature_errors[ ], CvPoint2D32f* cornersA, CvPoint2D32f* cornersB, bool writetofile)
{
	//ofstream numfeatsactivateddata;//log numfeaturesactivated to file to determine a scrubbing threshold - do this once while scrubing, once while not scrubbing
	//numfeatsactivateddata.open("C:\\dxFiles\\COACH4ASDresourceFiles\\numfeatsactivateddata.txt", ios::app);

	//set all elements to zero
	for (int i = 0; i<360; i++)
	{
		angles[i] = 0;
		distances[i] = 0;//Feb. 1, 2011
		distancesCount1[i] = 0;
		distancesCount2[i] = 0;

	}

	double rise; 
	double run;
	double tan_theta;
	double radians;
	double degrees;
	int quadrant;	// 2 | 1
					//---|---
					// 3 | 4
	float distancesS[360], distancesL[360];
	int anglesS[360], anglesL[360];
	for (int i=0; i<360; i++)
	{
		distancesS[i] = 0;
		distancesL[i] = 0;
		anglesS[i] = 0;
		anglesL[i] = 0;
	}

	//dx sep 27, 2012: just in case distanceQueue and angleQueue aren't empty
	distanceQueue.clear();
	angleQueue.clear();

	for( int i=0; i<corner_count; i++ )
	{
		if ( features_found[i]==0|| feature_errors[i]>550 ) {//from page 334 of Learning OpenCV
			printf("Error is %f/n",feature_errors[i]);
			continue;
		}
		printf ("Got it/n");
		CvPoint p0 = cvPoint(
			cvRound( cornersA[i].x ),
			cvRound( cornersA[i].y )
		);
		CvPoint p1 = cvPoint(
			cvRound( cornersB[i].x ),
			cvRound( cornersB[i].y )
		);

		cvLine( m_prevImg, p0, p1, CV_RGB(255,0,0),2);//, CV_AA, 0  );

		//populate the array:
		//get angle of each vector in radians
		//determine which quadrant it lies in
		//convert to degrees
		//round off and increment value in angles[]
		
		rise = p1.y - p0.y; 
		run  = p1.x - p0.x;
		
		
		//rise = 0; //theta = 0
		//run = 1;
		
		//rise = 1;//theta = 90
		//run = 0;

		//rise = 0;//theta = 180
		//run = -1;

		//rise = -1;//theta = 270
		//run = 0;

		tan_theta = rise/run;
		radians  = atan(tan_theta);
		degrees = radians*180/PI;

		//Determine which quadrant the angle is in

		//quadrant 1 includes 0-90
		//quadrant 2 includes 91-180
		//quadrant 3 includes 181-269
		//quadrant 4 includes 270-359
		
		if ( (rise >=0)&&(run>=0) )
			quadrant = 1;
		else if ( (rise >=0)&&(run<0) )
		{
			quadrant = 2;
			degrees += 180;
			if (degrees == 180)
				int a = 0;  //dx aug 22, 2012: don't know what this line is for... 
		}
		else if ( (rise<0)&&(run<0) )
		{
			quadrant = 3;
			degrees += 180;
		}
		else if ( (rise<0)&&(run>=0) )
		{
			quadrant = 4;
			degrees += 360;
		}
		
		if (degrees == 360) //dxnote oct 29, 2012: the legitimate angles should be (0, 360]
			degrees = 0;
		if (run == 0)
			degrees = -1; //dx oct 29, 2012: -1 degrees signifies error so we know to discard this point
		assert( (degrees >= 0) && (degrees < 360) || degrees == -1);
		
		distance = sqrt( pow(rise, 2) + pow(run, 2) );
		// look at all the distances and figure out what threshold of velocity to use
		angleQueue.push_back(degrees);
		distanceQueue.push_back(distance);
	}

	if (saveDataFrame_scrubTraining)
	{
		CSingleLock lock(&csDataFrameFile);
		lock.Lock();

		//dx sep 16, 2012: output queues to dataFrameFile /*and set dataFrameReady to true*/
		dataFrameFile.open("C:\\dxFiles\\COACH4ASDresourceFiles\\scrub_training_opticalFlowDataFrameFile", ios::app); //dxnote oct 19, 2012: the opticalFlowDataFrameFile is solely for scrub training's use, don't need it in normal doOpticalFlow
		if (dataFrameFile.is_open())
		{
			helperWriteDataFrameFile(dataFrameFile, distanceQueue, angleQueue);
			dataFrameFile.close();
			//dataFrameReady = true;//dx oct 20, 2012
		}

		lock.Unlock();
		//dxnote sep 21, 2012:  I've tried (shown below) to write a whole object raw into binary file, doesn't work when retrieving it
		// hard to specify object size and have to avoid saving pointers instead of objects themselves
	}

	//put the data into FRAMEs queue
	FRAME tempFrame;
	tempFrame.queueAngles = angleQueue;
	tempFrame.queueDistances = distanceQueue;
	queueFrames.clear(); //dxnote oct 20, 2012: need to clear queueFrames so there's only on frame on the framesqueue. need a queue because of the way function countNAngleBinsActivated is implemented
	queueFrames.push_back(tempFrame);

	int nAngleBins=36, nDistanceBins=3;
	deque<int> binDistanceBoundaries(nDistanceBins-1);
	binDistanceBoundaries [0] = 5;
	binDistanceBoundaries [1] = 80;

	//count the nAngleBinsActivated
	countNAngleBinsActivated(queueFrames, nDistanceBins, nAngleBins, binDistanceBoundaries);
	
	//dxnote oct 20, 2012: for now, nDistanceBins = 3; 0, 1, 2 are hard coded here, need to take care of cases where nDistanceBins is variable
	anglesActivatedS = queueFrames.front().nAngleBinsActivated[0];
	anglesActivated = queueFrames.front().nAngleBinsActivated[1];
	anglesActivatedL = queueFrames.front().nAngleBinsActivated[2];
      
	//dx sep 25, 2012:
	double a = scrubweights[0] + scrubweights[1]*anglesActivatedS + scrubweights[2]*anglesActivated + scrubweights[3]*anglesActivatedL; //dx oct 29, 2012
	//dxnote oct 20, 2012: note during the scrub training, the weight vector has anglesActivatedS before anglesActivated
	//probabilityOfScrubbing = 0;
	probabilityOfScrubbing = 1/(1+exp(-a));
	//if (anglesActivated>=15 && anglesActivatedS<=8 && anglesActivatedL<=4)
	if (probabilityOfScrubbing >= 0.5)
		scrubbingDetectedFramesQueue.push_back(true);
	else
		scrubbingDetectedFramesQueue.push_back(false);
	while(scrubbingDetectedFramesQueue.size() > NFRAMES)
		scrubbingDetectedFramesQueue.pop_front();
	deque<bool> temp = scrubbingDetectedFramesQueue;
	
	int nFramesScrub=0;
	while(!temp.empty())
	{
		if (temp.front())
			nFramesScrub++;
		temp.pop_front();
	}

	if (nFramesScrub >= NFRAMES*0.8) //dxnote oct 19, 2012: 0.8 or 1 don't matter, it works when NFRAMES = 1 either way
		RUBBINGHANDSDETECTEDBYOPTICALFLOW = true;
	else
		RUBBINGHANDSDETECTEDBYOPTICALFLOW = false;

	//numfeatsactivateddata << "scrubbingDetectedFramesQueue: " << endl;
	//deque<bool> temp = scrubbingDetectedFramesQueue;
	//while(!temp.empty())
	//{
	//	numfeatsactivateddata << temp.front() << " ";
	//	if (temp.front())
	//		nFramesScrub++;
	//	temp.pop_front();
	//}
	//numfeatsactivateddata << endl;
	//numfeatsactivateddata.close();
}

void COpticalFlow::DoVelocityHistogram(int corner_count, char features_found[], float feature_errors[ ], CvPoint2D32f* cornersA, CvPoint2D32f* cornersB)
{
	//Create histogram of velocities
	//find velocities (length of vectors in pixels)

		float distances[640];
			for (int i = 0; i<640; i++)	{ distances[i] = 0; }
		float distance;
	
		for( int i=0; i<corner_count; i++ ) {
			if ( features_found[i]==0|| feature_errors[i]>550 ) {
				printf("Error is %f/n",feature_errors[i]);
				continue;
			}
		//Compute distances
		distance = sqrt( pow((cornersB[i].x - cornersA[i].x), 2) + pow((cornersB[i].y - cornersA[i].y), 2) );

		//Construct histogram
		distances[(int)distance]++;
		}


		//Set Threshold

		int threshold = 60;

		for (int g = 0; g<threshold; g++){distances[g] = 0;}

		//Create an image for showing a histogram
		//width is 640, height is 600
		IplImage* hist_img = cvCreateImage(cvSize( 360, 600 ),8,3);
		cvZero( hist_img );

		// populate our visualization with red lines.
		for (int j = 0; j<640; j++)
		{
			CvPoint pa = cvPoint(j, 600);
			CvPoint pb = cvPoint(j, max(0, 600-60*distances[j]) );//imarray->imageData[j]);
			cvLine(hist_img, pa, pb, CV_RGB(255,0,0), 2);
		}

		cvShowImage( "Ex", hist_img );
		safe_release_image(hist_img);
}

bool COpticalFlow::ThresholdVelocity(double rise, double run, int lowthreshold, int highthreshold)
{
	//Compute distance
	distance = sqrt( pow(rise, 2) + pow(run, 2) );

	if ( (distance >= lowthreshold)&&(distance < highthreshold) )
		return true;
	return false;
}

void COpticalFlow::ThresholdArray(int* array, int arraylength, int threshold)
{
	for (int index = 0; index < arraylength; index++)
	{
		if (array[index] < threshold)
			array[index] = 0;
	}
}

void COpticalFlow::PlotAngularHistogram()
{
	double rise;
	double run;
	double tan_theta;
	double radians;
	double degrees;
	int quadrant;	// 2 | 1
					//---|---
					// 3 | 4
			
	int angle = 0;
	
	while (1)
	{
		rise = 100*sin(angle*PI/180);
		run = 100*cos(angle*PI/180);
		angle++;

		if (angle == 360)
			angle = 0;
	


	tan_theta = rise/run;
	radians  = atan(tan_theta);
	degrees = radians*180/PI;

	//Determine which quadrant the angle is in

	//quadrant 1 includes 0-90
	//quadrant 2 includes 91-180
	//quadrant 3 includes 181-269
	//quadrant 4 includes 270-359
	
	if ( (rise >=0)&&(run>=0) )
		quadrant = 1;
	else if ( (rise >=0)&&(run<0) )
	{
		quadrant = 2;
		degrees += 180;
	}
	else if ( (rise<0)&&(run<0) )
	{
		quadrant = 3;
		degrees += 180;
	}
	else if ( (rise<0)&&(run>=0) )
	{
		quadrant = 4;
		degrees += 360;
	}
	
	if (degrees == 360)
		degrees = 0;
		
		
	
	//Create an image for showing a histogram
	//width is 360, height is 600
	IplImage* hist_img = cvCreateImage(cvSize( 360, 600 ),8,3);
	cvZero( hist_img );

	// populate our visualization with red lines.
	CvPoint pa = cvPoint((int)degrees, 600);
	CvPoint pb = cvPoint((int)degrees, 0);//imarray->imageData[j]);
	cvLine(hist_img, pa, pb, CV_RGB(255,0,0), 2);

	//show histogram to the screen
	//cvNamedWindow( "Ex");
	cvShowImage( "Ex", hist_img );

	cvWaitKey(1);
	safe_release_image(hist_img);

	}//end of while
}

//dx oct 20, 2012:
void COpticalFlow::helperWriteDataFrameFile(ofstream &dataFrameFile, deque<float> distanceQueue, deque<float> angleQueue)
{//dxnote oct 20, 2012: ofstream &dataFrameFile is passed by reference, or else it won't be accessible
	deque<float> temp;
	temp = distanceQueue;
	dataFrameFile << "distanceQueue,"<< temp.size() << ",";
	while (!temp.empty())
	{
		dataFrameFile << temp.front() << ",";
		temp.pop_front();
	}
	temp = angleQueue;
	dataFrameFile << "angleQueue,"<< temp.size() << ",";
	while (!temp.empty())
	{
		dataFrameFile << temp.front() << ",";
		temp.pop_front();
	}
	dataFrameFile << endl;	
}

void COpticalFlow::countNAngleBinsActivated(deque<COpticalFlow::FRAME> &queueFrames, int nDistanceBins, int nAngleBins, deque<int> binDistanceBoundaries)
{
	//a function that takes in the data and nAngleBins, nDistanceBins, and outputs a nDistanceBins-dimensional vector of nAngleBinsActivated for each frame
	for (int i=0; i<queueFrames.size(); i++)
	{
		//run through each frame, and count them into the correct Distance-Angle-Bins
		////resize the result bins
		deque<deque<int>> distanceAngleBins;
		distanceAngleBins.resize(nDistanceBins);
		for (int j=0; j<nDistanceBins; j++)
		{
			distanceAngleBins[j].resize(nAngleBins);
		}
		//going through each angle bins
		for (int j=0; j<(queueFrames.at(i).queueAngles.size()); j++)
		{
			int iAngle = 0;
			int iDist = 0;
			//for (int k=nDistanceBins-1; k>0; --k)
			for (int k=nDistanceBins-2; k>=0; k--)			
			{//finding which distance bin the point belongs to
				if (queueFrames.at(i).queueDistances[j] > binDistanceBoundaries[k])
				{
					iDist = k+1;
					break;
				}
			}

			//float test = -0.1;
			//test = (int) test; //dxtest oct 29, 2012: if test = -0.1; then (int)test == 0;
			if (queueFrames.at(i).queueAngles[j] >= 0)
			{//dx oct 29, 2012: for the case where queueFrames->at(i).queueAngles[j] == -1; this is when run == 0 in DoAngularHistogram
				iAngle = (queueFrames.at(i).queueAngles[j] / (360/nAngleBins));
				if (iAngle >= nAngleBins)
				{
					assert(iAngle < nAngleBins); //raise an exception during debuging
					iAngle = nAngleBins-1; //to make sure we don't go over the array's range, just a safe checking for release version
				}
				distanceAngleBins[iDist][iAngle]++;
			}
		}

		//saving outputs: nAngleBinsActivated
		//queueScrubbingFrames[i].nAngleBinsActivated.resize(nDistanceBins, 0);
		queueFrames.at(i).nAngleBinsActivated.resize(nDistanceBins); //dxnote sep 17, 2012: does it pad with zeros? =>yep!
		int count=0;
		for (int j=0; j<nDistanceBins; j++)
		{
			for (int k=0; k<nAngleBins; k++)
			{
				if (distanceAngleBins[j][k]>0)
					count++;
			}
			queueFrames.at(i).nAngleBinsActivated[j] = count;
			count = 0;
		}
	}
}