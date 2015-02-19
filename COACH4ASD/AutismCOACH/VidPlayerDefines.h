#pragma once


// ========== image size
// 640*480 if defined, 320*240 otherwise
// #define IMAGE_SIZE_ORIGINAL 
// ---

// ========== task file

#define TASK_ACTIONS_FILENAME  "D:\\Users\\Babak\\Documents\\Projects\\RERC\\Annotator\\Annotator\\Task.txt"

// ========== using multimedia timer or regular timer
//#define USE_MULTIMEDIA_TIMER

#ifdef USE_MULTIMEDIA_TIMER
	// units: mS. 0 means highest priority. 
	#define MULTIMEDIA_TIMER_RESOLUTION	10
#endif
// ---


// ========== play audio
//#define PLAY_AND_PROCESS_AUDIO//March 26 - Justin
// ---


// ========== water detection
//#define PERFORM_WATER_DETECTION


// these have no effect if PERFORM_WATER_DETECTION is not defined

#define VIDEO_FEATURES_EXPORT_HISTOGRAM_MOMENTS


// intermediate water detection images (no effect if PERFORM_WATER_DETECTION is not defined)
//#define SHOW_INTERMEDIATE_WATER_DETECTION_IMAGES

//#define SAVE_INTERMEDIATE_WATER_DETECTION_IMAGES
//#define SAVE_FRAME_NO 467

// other flags for displaying intermediate images:
// - displayIntermediateImages in RANSAC_fit_ellipse()
// - showIntermediateSinkDetectionImages in CRERCAnnotatorDoc::DetectSinkRegion()


// ========== action detection
//#define PERFORM_ACTION_DETECTION

// these have no effect if PERFORM_ACTION_DETECTION is not defined

//#define LUCAS_KANADE_OPTICAL_FLOW_FEATURES
#define FARNEBACH_OPTICAL_FLOW_FEATURES

//#define SHOW_ACTION_DETECTOR_INTERMEDIATE_IMAGES

// ========== open, play, extract & save video features, and quit! (to extract videos features of several videos in a batch)
//#define BATCH_VIDEO_FEATURE_EXTRACT

