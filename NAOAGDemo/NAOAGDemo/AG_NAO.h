#pragma once

#include <vector>
#include <alvalue/alvalue.h>
#include <alproxies/almotionproxy.h>
#include <alproxies/alrobotpostureproxy.h>
#include <alproxies/altexttospeechproxy.h>

#define  robotIP "192.168.1.3"
#define port 9559

class AG_NAO {

public:
	AG_NAO() : motion(robotIP, port), posture(robotIP, port), tts(robotIP, port) {};
	void executeAG();
	void executePrompt();
	void executeReward();
private:
	AL::ALMotionProxy motion;
	AL::ALRobotPostureProxy posture;
	AL::ALTextToSpeechProxy tts;

	std::vector<std::string> names;
	AL::ALValue times, keys;

	void setWaveMotion();
	void setPromptMotion();
	void setRewardMotion();
};

