#include "StdAfx.h"
#include "WaterDetector.h"


#include "AutismCOACHDoc.h"
#include "VidPlayerDefines.h"
#include "cv.h"	
#include <highgui.h>
#include <math.h>


#define VID_FEATURES_HISTOGRAM_BIN_COUNT	((int)(256))

CWaterDetector::CWaterDetector(void)
{
	//bools
	water_images_set = false;

	//counters
	num_images_set = 0;
	
	//lefttapfeatureweight[0] = 0.64092;
	//lefttapfeatureweight[1] = 0.88013;
	//lefttapfeatureweight[2] = 0.91418;
	//lefttapfeatureweight[3] = 0.5846;
	//lefttapfeatureweight[4] = 2.2075;
	//lefttapfeatureweight[5] = -2.2044;
	//lefttapfeatureweight[6] = 1.3068;
	//lefttapfeatureweight[7] = 0.81145;
	//righttapfeatureweight[0] = -0.67953;
	//righttapfeatureweight[1] = -1.0504;
	//righttapfeatureweight[2] = -1.4971;
	//righttapfeatureweight[3] = -0.26683;
	//righttapfeatureweight[4] = -0.45669;
	//righttapfeatureweight[5] = 2.6185;
	//righttapfeatureweight[6] = 3.504;
	//righttapfeatureweight[7] = 2.9693;

	/*
	//Open-up console for debug output (source: Michael Belshaw)
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	*/

	prevWaterImg = NULL;
	prev2WaterImg = NULL;
	currentWaterImg = NULL;
	waterDiffImg = NULL;
	waterDiffImgBW = NULL;
	waterDiff2Img = NULL;
	waterDiff2ImgBW = NULL;
	currentSkinMaskImgBW = NULL;
	prevSkinMaskImgBW = NULL;
	prev2SkinMaskImgBW = NULL;
	skinMaskImg = NULL;

	// a simple 3x3 high pass filter [0 -1 0; -1 4 -1; 0 -1 0]
	highPassFilterKernel = cvCreateMat(3,3,CV_32FC1);
	cvSet2D( highPassFilterKernel, 0, 0, cvRealScalar(0.0) );
	cvSet2D( highPassFilterKernel, 0, 1, cvRealScalar(-1.0) );
	cvSet2D( highPassFilterKernel, 0, 2, cvRealScalar(0.0) );
	cvSet2D( highPassFilterKernel, 1, 0, cvRealScalar(-1.0) );
	cvSet2D( highPassFilterKernel, 1, 1, cvRealScalar(4.0) );
	cvSet2D( highPassFilterKernel, 1, 2, cvRealScalar(-1.0) );
	cvSet2D( highPassFilterKernel, 2, 0, cvRealScalar(0.0) );
	cvSet2D( highPassFilterKernel, 2, 1, cvRealScalar(-1.0) );
	cvSet2D( highPassFilterKernel, 2, 2, cvRealScalar(0.0) );	

	
    //size of the histogram (1D histogram)
	int hsize[] = {VID_FEATURES_HISTOGRAM_BIN_COUNT};
	//ranges - grayscale 0 to 256
	float xranges[] = { 0, float(VID_FEATURES_HISTOGRAM_BIN_COUNT) };
	float* ranges[] = { xranges };
	// allocate memory for hist1 and hist2
	hist1 = cvCreateHist(1, hsize, CV_HIST_ARRAY, ranges,1);
	hist2 = cvCreateHist(1, hsize, CV_HIST_ARRAY, ranges,1);

	#ifdef SAVE_INTERMEDIATE_WATER_DETECTION_VIDEOS
		vidWriter = NULL;
	#endif
}

CWaterDetector::~CWaterDetector(void)
{
	CleanUp();
	cvReleaseMat(&highPassFilterKernel);
	cvReleaseHist(&hist1);
	cvReleaseHist(&hist2);
}

void CWaterDetector::CleanUp(void)
{
	safe_release_image(prevWaterImg);
	safe_release_image(prev2WaterImg);
	safe_release_image(currentWaterImg);
	safe_release_image(waterDiffImg);
	safe_release_image(waterDiffImgBW);
	safe_release_image(waterDiff2Img);
	safe_release_image(waterDiff2ImgBW);
	safe_release_image(currentSkinMaskImgBW);
	safe_release_image(prevSkinMaskImgBW);
	safe_release_image(prev2SkinMaskImgBW);
	safe_release_image(skinMaskImg);

	#ifdef SAVE_INTERMEDIATE_WATER_DETECTION_VIDEOS
		if (vidWriter != NULL)
		{
			cvReleaseVideoWriter(&vidWriter);
			vidWriter = NULL;
		}
	#endif
}


void CWaterDetector::Init(int waterX, int waterY, float waterRadius)
{//dxnote oct 22, 2012:  params passed are calibDlg.regions[REGION_SINK].centreX, y, and radius. why?
	CleanUp(); // clean up first (to free the memory if we were previously working on anohter video or ...)
	
	//dxnote oct 22, 2012: drawing a square enclosing the sink region?
	waterRect.x = max(0, waterX - int(waterRadius));
	waterRect.y = max(0, waterY - int(waterRadius));
	waterRect.width = min(640, 2 * int(waterRadius) + 1);
	waterRect.height = min(480, 2 * int(waterRadius) + 1);

	// allocate memory for intermediate images
	prevWaterImg = cvCreateImage(cvSize(waterRect.width, waterRect.height), IPL_DEPTH_8U, 3);
	prev2WaterImg = cvCreateImage(cvSize(waterRect.width, waterRect.height), IPL_DEPTH_8U, 3);
	currentWaterImg = cvCreateImage(cvSize(waterRect.width, waterRect.height), IPL_DEPTH_8U, 3); 
	waterDiffImg = cvCreateImage(cvSize(waterRect.width, waterRect.height), IPL_DEPTH_8U, 3); 
	waterDiffImgBW = cvCreateImage(cvSize(waterRect.width, waterRect.height), IPL_DEPTH_8U, 1); 
	waterDiff2Img = cvCreateImage(cvSize(waterRect.width, waterRect.height), IPL_DEPTH_8U, 3); 
	waterDiff2ImgBW = cvCreateImage(cvSize(waterRect.width, waterRect.height), IPL_DEPTH_8U, 1); 
	currentSkinMaskImgBW = cvCreateImage(cvSize(waterRect.width, waterRect.height), IPL_DEPTH_8U, 1); 
	prevSkinMaskImgBW = cvCreateImage(cvSize(waterRect.width, waterRect.height), IPL_DEPTH_8U, 1); 
	prev2SkinMaskImgBW = cvCreateImage(cvSize(waterRect.width, waterRect.height), IPL_DEPTH_8U, 1); 

	skinMaskImg = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3); 
}

int CWaterDetector::GetHistogramCount()
{ 
	return VID_FEATURES_HISTOGRAM_BIN_COUNT; 
}


float CWaterDetector::EstimateWaterStatus(IplImage* currentFrame, unsigned char* skinImageRaw, unsigned int imWidth, unsigned int imHeight) 
{
	//ComputeVideoFeatures(currentFrame, skinImageRaw);
	
	waterOnProbabilityFromVideo = EstimateWaterOnUsingVideoFeatures(); // returns something between 0 and 1, also shows confidence

	/*
	#ifdef PLAY_AND_PROCESS_AUDIO
		ComputeAudioFeatures(currentFrameNo, ...);
		waterOnFromAudio = EstimateWaterOnUsingAudioFeatures(currentFrameNo); // returns something between 0 and 1, also shows confidence
	#endif


	waterOnProbability = ....
	*/

	// ............................. TEMP ...............
	// ............................. TEMP ...............
	// ............................. TEMP ...............
	// ............................. TEMP ...............
	// ............................. TEMP ...............
	// ............................. TEMP ...............
	waterOnProbability = waterOnProbabilityFromVideo; // TEMPORARY .... later on: combine audio and video estimates in a smart way (using confidence and priors)

	return waterOnProbability;
}

float CWaterDetector::EstimateWaterOnUsingVideoFeatures()
{
	// ............................. TEMP ...............
	// ............................. TEMP ...............
	// ............................. TEMP ...............
	// ............................. TEMP ...............
	// ............................. TEMP ...............
	// ............................. TEMP ...............
	//return firstOrderDerivativeHistFirstMoment.at(currentFrameNo) > 2.0 ; // extremely naive .... TAMPORARY ....
	return 1; // temporary
}

void CWaterDetector::ComputeVideoFeatures(IplImage* currentFrame, unsigned char* skinImageRaw, int testCase, bool write, int gt_label)//Feb. 23, 2011
{
//dx oct 23, 2012: since we aren't using theses moments and entropies features, and just use intensity bin features, I commented the unused features out during processing
//	static double log_of_2 = log(2.0);
//
////Set handregiondilate count
//	static int handRegionDilateCount = 5; // in the end, the hand region is dilated a bit to be conservative and don't miss any of it
//
////Mark the region of interest in the current frame
//	cvSetImageROI(currentFrame, waterRect); // mark the region of interest 
//	//dxnote oct 22, 2012: the ROI here (i.e. waterRect) is set by CWaterDetector::init to the square enclosing the sink region. Why 
//
////Get the skin image image data
//	
//	//memcpy(skinMaskImg->imageData, skinImageRaw, 640 * 480 * 3); // copy skin (=hand) location (update (March 3, 2010): this image also contains towel location. So we mask out hand AND towel in the sink) 
//
//	IplImage* tempImg = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3);
//	memcpy(tempImg->imageData, skinImageRaw, 320 * 240 * 3);
//	cvResize(tempImg, skinMaskImg); //dxnote oct 22, 2012: skinMaskImg contains now a 640X480 version of the skinImageRaw
//	cvReleaseImage(&tempImg);
//	
////Mark the region of interest in the skin image and convert it to black and white, dilate
//	cvSetImageROI(skinMaskImg, waterRect); // we are interested only in hands (=skin) in the water area
//	cvConvertImage(skinMaskImg, currentSkinMaskImgBW); // convert to black & white
//	//cvThreshold(currentSkinMaskImgBW, currentSkinMaskImgBW, 1, 255, CV_THRESH_BINARY);
//	cvDilate(currentSkinMaskImgBW, currentSkinMaskImgBW, 0, handRegionDilateCount); // dilate a bit (to be conservative and don't miss skin regions)
//
////Copy the current frame to the currentWaterImg
//	cvSetZero(currentWaterImg);
//	cvCopy(currentFrame, currentWaterImg);
//
////Compute the 1st order temporal derivative and convert it to black and white
//	cvAbsDiff(currentWaterImg, prevWaterImg, waterDiffImg); // compute the 1st order temporal derivative
//	cvConvertImage(waterDiffImg, waterDiffImgBW); // make it black & white
//
////Compute the 2nd order temporal derivative and convert it to black and white
//	cvAbsDiff(currentWaterImg, prev2WaterImg, waterDiff2Img); // compute the 2nd order temporal derivative
//	cvConvertImage(waterDiff2Img, waterDiff2ImgBW); // make it black & white
//
//// high pass filter to reduce consistent zones (e.g. shadhows etc) but keep random-like water regions
//	cvFilter2D( waterDiffImgBW, waterDiffImgBW, highPassFilterKernel, cvPoint(-1,-1) );
//	cvFilter2D( waterDiff2ImgBW, waterDiff2ImgBW, highPassFilterKernel, cvPoint(-1,-1) );
//
//	
//		//Get the masked region size
//		maskedRegion1Size = 0;
//		maskedRegion2Size = 0;
//
//		for (int ii=0; ii<(waterRect.height); ii++) 
//		{
//			for (int jj=0; jj<(waterRect.width); jj++) 
//			{
//				int ix = jj + ii * currentSkinMaskImgBW->widthStep; // widthStep is the same in currentSkinMaskImgBW, prevSkinMaskImgBW, and prev2SkinMaskImgBW
//
//				// first order derivative
//				if(  ( (currentSkinMaskImgBW->imageData[ix])!=0 ) || ( (prevSkinMaskImgBW->imageData[ix])!=0 )  )
//				{
//					waterDiffImgBW->imageData[ix] =	0;
//					maskedRegion1Size ++;
//				}
//
//				// second order derivative
//				if(  ( (currentSkinMaskImgBW->imageData[ix])!=0 ) || ( (prev2SkinMaskImgBW->imageData[ix])!=0 )  )
//				{
//					waterDiff2ImgBW->imageData[ix] = 0;
//					maskedRegion2Size ++;
//				}
//			}
//		}	
//
//		// form a histogram of the 1st order temporal derivative
//		cvCalcHist(&waterDiffImgBW, hist1, 0, NULL);
//		cvNormalizeHist(hist1, 1.0);
//
//		// form a histogram of the 2nd order temporal derivative
//		cvCalcHist(&waterDiff2ImgBW, hist2, 0, NULL);
//		cvNormalizeHist(hist2, 1.0);
//
//
//		// compute statistics from the histograms and also draw the histogram 
//		//Only calculate statistics if writing to file
//
//		#ifdef VIDEO_FEATURES_EXPORT_HISTOGRAM_MOMENTS
//			entropy1 = 0;
//			entropy2 = 0;
//			firstOrderDerivativeHistFirstMoment = 0;
//			firstOrderDerivativeHistSecondMoment = 0;
//			firstOrderDerivativeHistThirdMoment = 0;
//			secondOrderDerivativeHistFirstMoment = 0;
//			secondOrderDerivativeHistSecondMoment = 0;
//			secondOrderDerivativeHistThirdMoment = 0;
//		#endif
//
//		int E1count = 0;
//		int M1count = 0;
//		int M2count = 0;
//		int M3count = 0;
//
//		float value1, value2;
//		for(int ii=0; ii < VID_FEATURES_HISTOGRAM_BIN_COUNT; ii++)
//		{
//			value1 = cvQueryHistValue_1D(hist1, ii);
//			value2 = cvQueryHistValue_1D(hist2, ii);
//
//
//			#ifdef VIDEO_FEATURES_EXPORT_HISTOGRAM_MOMENTS
//				// moments (give higher weights to lighter bins) and entropy
//
//				firstOrderDerivativeHistFirstMoment  += float(ii) * value1;
//				M1count ++;
//				firstOrderDerivativeHistSecondMoment  += float(ii*ii) * value1;
//				M2count ++;
//				firstOrderDerivativeHistThirdMoment  += float(ii*ii*ii) * value1;
//				M3count ++;
//				secondOrderDerivativeHistFirstMoment  += float(ii) * value2;
//				secondOrderDerivativeHistSecondMoment  += float(ii*ii) * value2;
//				secondOrderDerivativeHistThirdMoment  += float(ii*ii*ii) * value2;
//				
//				if (value1 > 0)
//				{	
//					entropy1  -= value1 * log(value1) / log_of_2;
//					E1count++;
//				}
//				if (value2 > 0)
//				{
//					entropy2  -= value2 * log(value2) / log_of_2;
//				}
//			#endif
//		}

		//Write features to file here
	if (write) //Feb. 23, 2011
	{
		ofstream logfeatures;
		if(testCase == 0)
		{
					
			logfeatures.open("C:\\dxFiles\\COACH4ASDresourceFiles\\TapFeatures.txt", ios::app);//Feb. 23, 2011
			
			logfeatures << "\t";
			logfeatures << "E1" << "\t";
			logfeatures << "E2" << "\t";
			logfeatures << "1st,1st" << "\t";
			logfeatures << "1st,2nd" << "\t";
			logfeatures << "1st,3rd" << "\t";
			logfeatures << "2nd,1st" << "\t";
			logfeatures << "2nd,2nd" << "\t";
			logfeatures << "2nd,3rd" << "\t";
			logfeatures << "MR1" << "\t";
			logfeatures << "MR2" << "\t";

			logfeatures<< "L0" << "\t" <<"L1" << "\t" <<"L2" << "\t" <<"L3" << "\t" <<"L4" << "\t" <<"L5" << "\t" <<"L6" << "\t" <<"L7" << "\t" ;
			logfeatures<< "R0" << "\t" <<"R1" << "\t" <<"R2" << "\t" <<"R3" << "\t" <<"R4" << "\t" <<"R5" << "\t" <<"R6" << "\t" <<"R7" << "\t" ;
			logfeatures<< "Ground Truth Label" << endl;//Feb. 23, 2011
			logfeatures.close();
		}

		//cvShowImage("prev2WaterImg", prev2WaterImg);

		else
		{
			logfeatures.open("C:\\dxFiles\\COACH4ASDresourceFiles\\TapFeatures.txt", ios::app);
				
			logfeatures << testCase << "\t";
			logfeatures << entropy1 << "\t";
			logfeatures << entropy2 << "\t";
			logfeatures << firstOrderDerivativeHistFirstMoment << "\t";
			logfeatures << firstOrderDerivativeHistSecondMoment << "\t";
			logfeatures << firstOrderDerivativeHistThirdMoment << "\t";
			logfeatures << secondOrderDerivativeHistFirstMoment << "\t";
			logfeatures << secondOrderDerivativeHistSecondMoment << "\t";
			logfeatures << secondOrderDerivativeHistThirdMoment << "\t";
			logfeatures << maskedRegion1Size << "\t";
			logfeatures << maskedRegion2Size << "\t";


			//int leftarea = 57*57;
			//int rightarea = 53*53;
			
			logfeatures << bins_left[0] << "\t";
			logfeatures << bins_left[1] << "\t";
			logfeatures << bins_left[2] << "\t";
			logfeatures << bins_left[3] << "\t";
			logfeatures << bins_left[4] << "\t";
			logfeatures << bins_left[5] << "\t";
			logfeatures << bins_left[6] << "\t";
			logfeatures << bins_left[7] << "\t";

			logfeatures << bins_right[0] << "\t";
			logfeatures << bins_right[1] << "\t";
			logfeatures << bins_right[2] << "\t";
			logfeatures << bins_right[3] << "\t";
			logfeatures << bins_right[4] << "\t";
			logfeatures << bins_right[5] << "\t";
			logfeatures << bins_right[6] << "\t";
			logfeatures << bins_right[7] << "\t";
			logfeatures << gt_label << endl;//Feb. 23, 2011

			logfeatures.close();
		}
	}

	//dx oct 23, 2012: not used
	////order of copying is importnt here
	////copy to prev2WaterImg first, then prevWaterImg
	//
	////move previmgs to prev2imgs
	//cvCopy(prevWaterImg, prev2WaterImg);
	//cvCopy(prevSkinMaskImgBW, prev2SkinMaskImgBW);

	////move currentimgs to previmgs
	//cvCopy(currentWaterImg, prevWaterImg);
	//cvCopy(currentSkinMaskImgBW, prevSkinMaskImgBW);

	////DebugWindow("PrevImg", currentWaterImg);
	//
	//cvResetImageROI(currentFrame);
	//cvResetImageROI(skinMaskImg);

	////cvSaveImage("prevWaterImg.jpg", prevWaterImg);
	////cvSaveImage("prev2WaterImg.jpg", prev2WaterImg);
	////cvSaveImage("prevSkinMaskImgBW.jpg", prevSkinMaskImgBW);
	////cvSaveImage("prev2SkinMaskImgBW.jpg", prev2SkinMaskImgBW);
	////cvSaveImage("currentWaterImg.jpg", currentWaterImg);
	////cvSaveImage("currentSkinMaskImgBW.jpg", currentSkinMaskImgBW);
}

//void CWaterDetector::SaveVideoFeatures(CString filename)
//{	
//	std::ofstream videoFeaturesFile;
//	videoFeaturesFile.open(filename);
//	CString s;
//
//	//videoFeaturesFile << "Video Features\n";
//	for (unsigned int ii=1; ii<maskedRegion1Size.size(); ii++)	// start from 1 (not 0) since frames start at 1. 
//																// frame 1 is also wasted (it's grabbed when the video is loaded and playing starts from frame 2)
//																// Also, note that since video features depend on prev and prev_prev frames, the video features for the next two frames are invalid as well.
//																// so the first frame with valid video features is frame #4
//	{
//		videoFeaturesFile << ii << "  ";
//
//		#ifdef VIDEO_FEATURES_EXPORT_HISTOGRAM_MOMENTS
//			videoFeaturesFile << entropy1.at(ii) << "  ";
//			videoFeaturesFile << entropy2.at(ii) << "  ";
//			videoFeaturesFile << firstOrderDerivativeHistFirstMoment.at(ii) << "  ";
//			videoFeaturesFile << firstOrderDerivativeHistSecondMoment.at(ii) << "  ";
//			videoFeaturesFile << firstOrderDerivativeHistThirdMoment.at(ii) << "  ";
//			videoFeaturesFile << secondOrderDerivativeHistFirstMoment.at(ii) << "  ";
//			videoFeaturesFile << secondOrderDerivativeHistSecondMoment.at(ii) << "  ";
//			videoFeaturesFile << secondOrderDerivativeHistThirdMoment.at(ii) << "  ";
//		#endif
//
//
//
//		videoFeaturesFile << maskedRegion1Size.at(ii) << "  ";
//		videoFeaturesFile << maskedRegion2Size.at(ii) << "\n";
//		
//	}
//	videoFeaturesFile.close();
//}



void CWaterDetector::UpdateImages(IplImage* currentFrame)
{
//Mark the region of interest in the current frame
	
	//cvSetImageROI(currentFrame, waterRect); // mark the region of interest //Should not need this//Dec.13, 2010

	//if (num_images_set == 0)
	//{
	//	cvSaveImage("prevWaterImg1.jpg", prevWaterImg);
	//	cvSaveImage("prev2WaterImg1.jpg", prev2WaterImg);
	//	cvSaveImage("prevSkinMaskImgBW1.jpg", prevSkinMaskImgBW);
	//	cvSaveImage("prev2SkinMaskImgBW1.jpg", prev2SkinMaskImgBW);
	//	cvSaveImage("currentWaterImg1.jpg", currentWaterImg);
	//	cvSaveImage("currentSkinMaskImgBW1.jpg", currentSkinMaskImgBW);
	//}
	//else if (num_images_set == 1)
	//{
	//	cvSaveImage("prevWaterImg2.jpg", prevWaterImg);
	//	cvSaveImage("prev2WaterImg2.jpg", prev2WaterImg);
	//	cvSaveImage("prevSkinMaskImgBW2.jpg", prevSkinMaskImgBW);
	//	cvSaveImage("prev2SkinMaskImgBW2.jpg", prev2SkinMaskImgBW);
	//	cvSaveImage("currentWaterImg2.jpg", currentWaterImg);
	//	cvSaveImage("currentSkinMaskImgBW2.jpg", currentSkinMaskImgBW);
	//}

	//move previmgs to prev2imgs
	cvCopy(prevWaterImg, prev2WaterImg);
	cvCopy(prevSkinMaskImgBW, prev2SkinMaskImgBW);

	//move currentimgs to previmg
	cvCopy(currentWaterImg, prevWaterImg);
	cvCopy(currentSkinMaskImgBW, prevSkinMaskImgBW);

	cvResetImageROI(currentFrame);
	cvResetImageROI(skinMaskImg);
}

void CWaterDetector::SetWaterImages(IplImage* currentFrame)
{
	UpdateImages(currentFrame);
	num_images_set++;
	if (num_images_set == 100)
		water_images_set = true;
}

void CWaterDetector::EndWaterDetection()
{
	water_images_set = false;
	num_images_set = 0;
	CleanUp();
}

void CWaterDetector::DebugWindow(CString windowname, IplImage*& im)
{
	cvShowImage(windowname, im);
	Sleep(2000);
	cvDestroyWindow(windowname);
}