#pragma once

#include <vector>
#include <alvalue/alvalue.h>
#include <alproxies/almotionproxy.h>
#include <alproxies/alrobotpostureproxy.h>
#include <alproxies/altexttospeechproxy.h>
#include <alproxies/alledsproxy.h>
#include <alproxies/alfacetrackerproxy.h>
#include <string>

#include "../COACH/defines.h"

class NAO_motion
{
public:
	NAO_motion() : motion(robotIP, robotPort)
		, tts(robotIP, robotPort)
		, led(robotIP, robotPort)
		, faceTracker(robotIP, robotPort)
	{
	};

	int postSay(std::string msg) {
		//return tts.post.say(msg);
		return tts.post.say("\\VCT=90\\" + msg + "\\RST\\");
	};

protected:
	AL::ALMotionProxy motion;
	//AL::ALRobotPostureProxy posture;
	AL::ALTextToSpeechProxy tts;
	AL::ALLedsProxy led;
	AL::ALFaceTrackerProxy faceTracker;

	std::vector<std::string> names;
	AL::ALValue times, keys;
};
