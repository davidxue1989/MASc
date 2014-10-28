#pragma once
#include "Prompts.h"

#include "../WoZStudyView.h"
#include "../ShowImageDlg.h"

#include "../COACH/defines.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

#include "../PlaybackDlg.h"

class Video_prompts :
	public Prompts
{
public:
	Video_prompts(CWoZStudyView *pView);

	void takeRest();
	void emergencyStop();
	void immediatePause(bool takeRest);
	
private:
	//ShowImageDlg imgDlg;
	CWoZStudyView *pView;
	Mat restImage;
	CString videoAlias, verbalAlias;

	DWORD WINAPI PromptingThread();
	int waitForVideoAndVerbal();
	bool playVideo(std::wstring filename);
	bool playVerbal(std::wstring filename);


	bool isPlaying(CString alias);
	void stopMedia(CString alias);
};
