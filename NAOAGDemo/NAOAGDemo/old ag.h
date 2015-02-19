#include "stdafx.h"
#include "AG_NAO.h"

#include <cstring>
#include <alerror/alerror.h>

#include "TestMotion.h"

AG_NAO::AG_NAO() : tts(IP, PORT) {

	TestMotion test;
	test.addMotion();
	test.executeMotion();
}

AG_NAO::~AG_NAO() {
	tts.stopAll();
}

void AG_NAO::doAG() {
	say("Hello world.");
}


void AG_NAO::doGotGaze() {
	say("Got gaze!");
}

void AG_NAO::say(const std::string phraseToSay) {
	try
	{
		ttsID = tts.post.say(phraseToSay);
		//tts.say(phraseToSay);
	}
	catch (const AL::ALError& e)
	{
		CString str;
		str.Format(_T("Caught exception: %s"), e.what());
		AfxMessageBox(str);
		exit(1);
	}
}

bool AG_NAO::isBusy() {
	bool isBusy = false;
	isBusy = tts.isRunning(ttsID);
	return isBusy;
}


#pragma once
#include <string>
#include <alproxies/altexttospeechproxy.h>
using namespace std;

#define PORT 9559
#define IP "192.168.1.3"
//#define IP "127.0.0.1"

class AG_NAO
{
public:
	AG_NAO();
	~AG_NAO();
	void doAG();
	void doGotGaze();

	bool isBusy();
private:
	void say(const std::string phraseToSay);

	AL::ALTextToSpeechProxy tts;
	int ttsID;
};

