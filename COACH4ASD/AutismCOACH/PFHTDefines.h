#ifndef DEFINES_H
#define DEFINES_H
//#include <stdlib.h>
//#include "config.h" // Babak
#include "../VidPlayerDefines.h"

// define this if in using COACH for a demo
// (sets all timeout values to a smaller, constant value)
//#define DEMO_MODE 1

// define this if a caregiver action is included in the
// model. THis is also defined in beliefmonitor.cpp
#define CAREGIVER 1

// define this if a water sensor is included in the
// model. THis is also defined in beliefmonitor.cpp
//#define WATERSENSOR 1

// the raw observations. These differ from the
// pomdp_observations since they are of the locations
// of each hand independently. The pomdp, in order to
// reduce the number of observations, uses a combination of
// left and right hand locations
// if water sensor is included use 4 here
#ifdef WATERSENSOR
	#ifdef CAREGIVER 
		#define NUMBER_OBSERVATIONS  5
		#define WATERSENSOR_OBSERVATION 2
		#define TIMEOUT_OBSERVATION 3
		#define PSTEP_OBSERVATION 4
	#else //CAREGIVER
		#define NUMBER_OBSERVATIONS  4
		#define WATERSENSOR_OBSERVATION 2
		#define TIMEOUT_OBSERVATION 3
	#endif //CAREGIVER
#else //WATERSENSOR
	#ifdef CAREGIVER
		#define NUMBER_OBSERVATIONS  4
		#define TIMEOUT_OBSERVATION 2
		#define PSTEP_OBSERVATION 3
	#else //CAREGIVER
		#define NUMBER_OBSERVATIONS  3
		#define TIMEOUT_OBSERVATION 2
	#endif //CAREGIVER
#endif //WATERSENSOR

#define LEFT_HANDLOC_OBSERVATION 0
#define RIGHT_HANDLOC_OBSERVATION 1

// threshold for timeouts 
#define TIMETHRESH 7 //10 //seconds

// if P(hw=true) > HWTHRESH, the trial ends
#define HWTHRESH 0.5
//The maximum number of POMDP timeouts that can occur before we pause the
//trial and call the caregiver.
#define MAX_NUM_TIMEOUTS 10

#define FRAME_NUM NUMBER_OBSERVATIONS 

enum	// note:	text description of these regions in CMainFrame::ShowHandPositions()
		//			update the text description if you change anything here
{
	HAND_POS_AWAY = 0,
	HAND_POS_SINK,
	HAND_POS_WATER,
	HAND_POS_TAP,
	HAND_POS_SOAP,
	HAND_POS_TOWEL,
};

/* NOOOO!
#ifdef IMAGE_SIZE_ORIGINAL
	#define NUMBER_OF_PARTICLES 600
#else
	#define NUMBER_OF_PARTICLES 200
#endif
*/
#define NUMBER_OF_PARTICLES 200

#endif


