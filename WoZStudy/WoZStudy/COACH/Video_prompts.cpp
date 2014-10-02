#include "StdAfx.h"
#include "Video_prompts.h"

#include <MMSystem.h>
#pragma comment(lib, "winmm.lib")

#include <sstream>

Video_prompts::Video_prompts(CWoZStudyView *pView)
	: Prompts()
	, pView(pView)
	, videoAlias(L"myVideo")
	, verbalAlias(L"VerbalPrompt")
{
	restImage = imread("..\\avatar_prompts\\rest.png");
	//imgDlg.Create(ShowImageDlg::IDD, pView);
}

DWORD WINAPI Video_prompts::PromptingThread() {

	std::vector<std::wstring> videos, verbals;

	if (taskName.compare(L"AttentionGrabber") == 0) {
		videos.push_back(L"AttentionGrabber");
		verbals.push_back(_T(USER_NAME));
		verbals.push_back(L"AttentionGrabber");
	}
	else if (taskName.compare(L"Reward") == 0) {
		verbals.push_back(L"Reward");
	}
	else if (taskName.compare(L"Intro") == 0) {
		videos.push_back(L"Intro");
		verbals.push_back(L"Intro");
		verbals.push_back(_T(USER_NAME));
		verbals.push_back(L"AttentionGrabber");
	}
	else if (taskName.compare(L"LetUsContinue") == 0) {
		videos.push_back(L"LetUsContinue");
		verbals.push_back(L"LetUsContinue");
	}
	else if (taskName.compare(L"AllDone") == 0) {
		videos.push_back(L"AllDone");
		verbals.push_back(L"AllDone");
		verbals.push_back(_T(USER_NAME));
		verbals.push_back(L"GoodJob");
	}
	else if (taskName.compare(L"TurnOnWater") == 0) {
		videos.push_back(L"TurnOnWater");
		verbals.push_back(L"TurnOnWater");
	}
	else if (taskName.compare(L"WetYourHands") == 0) {
		videos.push_back(L"WetYourHands");
		verbals.push_back(L"WetYourHands");
	}
	else if (taskName.compare(L"GetSomeSoap") == 0) {
		videos.push_back(L"GetSomeSoap");
		verbals.push_back(L"GetSomeSoap");
	}
	else if (taskName.compare(L"ScrubYourHands") == 0) {
		videos.push_back(L"ScrubYourHands");
		verbals.push_back(L"ScrubYourHands");
	}
	else if (taskName.compare(L"RinseYourHands") == 0) {
		videos.push_back(L"WetYourHands");//same motions
		verbals.push_back(L"RinseYourHands");
	}
	else if (taskName.compare(L"TurnOffWater") == 0) {
		videos.push_back(L"TurnOnWater");//same motions
		verbals.push_back(L"TurnOffWater");
	}
	else if (taskName.compare(L"DryYourHands") == 0) {
		videos.push_back(L"DryYourHands");
		verbals.push_back(L"DryYourHands");
	}
	else {
		ASSERT(FALSE);
	}
		
	int vidOrVerb = 0;
	while ((!videos.empty() || !verbals.empty() || vidOrVerb != 0) && vidOrVerb != -1) {
		if ((vidOrVerb == 1 || vidOrVerb == 0) && !videos.empty()) {
			playVideo(videos.back());
			videos.pop_back();
			//imgDlg.hideImg();//hide rest image
		}
		if ((vidOrVerb == 2 || vidOrVerb == 0) && !verbals.empty()) {
			playVerbal(verbals.back());
			verbals.pop_back();
		}
		vidOrVerb = waitForVideoAndVerbal();
	}

	m_hPromptingThread = NULL;
	return 0;
}

bool Video_prompts::playVerbal(std::wstring filename) {
	MCIERROR rs;
	//open
	rs = mciSendString((L"open \"..\\avatar_prompts\\" + filename + L".wav\" type MPEGVideo alias VerbalPrompt").c_str(), NULL, 0, 0);
	if (rs != 0) {
		wchar_t bf[128];
		mciGetErrorString(rs, bf, 128);
		ASSERT(FALSE);
		return false;
	}
	//play
	rs = mciSendString(L"play VerbalPrompt from 0", NULL, 0, 0);
	if (rs != 0) {
		wchar_t bf[128];
		mciGetErrorString(rs, bf, 128);
		ASSERT(FALSE);
		return false;
	}
	return true;
}

bool Video_prompts::playVideo(std::wstring filename) {
	MCIERROR rs;
	//open
	rs = mciSendString((L"open \"..\\avatar_prompts\\" + filename + L".avi\" type MPEGVideo alias myVideo style popup").c_str(), NULL, 0, 0);
	if (rs != 0) {
		wchar_t bf[128];
		mciGetErrorString(rs, bf, 128);
		ASSERT(FALSE);
		return false;
	}

	//fits
	CString str;
	str.Format(L"put %s window at %d 0 %d %d", videoAlias, FIRSTMONITORWIDTH, SECONDMONITORWIDTH, SECONDMONITORHEIGHT);
	rs = mciSendString(str.GetString(), NULL, 0, 0);
	if (rs != 0) {
		wchar_t bf[128];
		mciGetErrorString(rs, bf, 128);
		ASSERT(FALSE);
		return false;
	}

	//play
	rs = mciSendString(L"play myVideo", NULL, 0, 0);
	if (rs != 0) {
		wchar_t bf[128];
		mciGetErrorString(rs, bf, 128);
		ASSERT(FALSE);
		return false;
	}

	return true;
}

int Video_prompts::waitForVideoAndVerbal() {
	CString str;
	bool videoDone = false, verbalDone = false;
	while (true) {
		if (m_ExitThread != -1) {
			//takeRest();
			//m_ExitThread = -1;
			stopMedia(videoAlias);
			stopMedia(verbalAlias);
			return -1;
		}
		if (!isPlaying(videoAlias)) {
			//takeRest();
			str.Format(L"close %s", videoAlias);
			mciSendString(str, NULL, 0, 0);
			videoDone = true;
		}
		if (!isPlaying(verbalAlias)) {
			str.Format(L"close %s", verbalAlias);
			mciSendString(str, NULL, 0, 0);
			verbalDone = true;
		}
		if (videoDone) {
			if (verbalDone)
				return 0;
			else
				return 1;
		}
		else if (verbalDone)
			return 2;
	}
}

void Video_prompts::stopMedia(CString alias) {
	if (isPlaying(alias)) {
		mciSendString(L"stop " + alias, NULL, 0, 0);
		mciSendString(L"close " + alias, NULL, 0, 0);
	}
}

bool Video_prompts::isPlaying(CString alias) {
	MCIERROR rs;
	bool isPlaying = false;
	wchar_t buffer2[128];
	CString str;
	str.Format(L"status %s mode", alias);
	rs = mciSendString(str.GetString(), buffer2, 128, 0);
	if (rs != 0) {
		wchar_t bf[128];
		mciGetErrorString(rs, bf, 128);
		isPlaying = false;
	}
	if (wcscmp(buffer2, L"playing") == 0)
	{
		isPlaying = true;
	}
	//else if (wcscmp(buffer2, L"stopped") == 0)
	else
	{
		isPlaying = false;
	}
	return isPlaying;
}

void Video_prompts::emergencyStop() {
	if (m_ExitThread == -1) {
		m_ExitThread = 0;
	}
}

void Video_prompts::takeRest() {
	stopMedia(videoAlias);
	stopMedia(verbalAlias);
	//RECT r = { FIRSTMONITORWIDTH, 0, SECONDMONITORWIDTH + FIRSTMONITORWIDTH, SECONDMONITORHEIGHT };
	//imgDlg.ShowImg(restImage, &r);
}


void Video_prompts::immediatePause(bool takeRest) {
	emergencyStop(); //for virtual avatar videos, immediatePause and emergencyStop are the same
}