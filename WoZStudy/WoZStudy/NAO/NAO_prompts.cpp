#include "StdAfx.h"
#include "NAO_prompts.h"

//note: bezier is much better than simplified in terms of motion fidelity
NAO_prompts::NAO_prompts()
: NAO_motion()
, Prompts()
, m_IsLoose(true)
, m_Stopped(false)
, m_TakeRest(false)
, led_id(-1)
, head_angles(2)
, motion_id(-1)
, tts_id(-1)
{
	tts.setLanguage("English");
	tts.setVoice("Kenny22Enhanced");

	stiffen(false);

	head_angles.resize(2);
	head_angles[0] = DefaultHeadAngle1;
	head_angles[1] = DefaultHeadAngle2;
	motion.setAngles("Head", head_angles, 0.2f);
}

NAO_prompts::~NAO_prompts() {
};

DWORD WINAPI NAO_prompts::PromptingThread()
{
	if (m_TakeRest) {
		m_TakeRest = false;
		takeRest1();
	}
	else {
		
		if (taskName.compare(L"Reward") == 0) {
			reward();
		}
		else {

			if (led.isRunning(led_id))
				led.stop(led_id);
			led_id = led.post.rasta(0.3);

			head_angles = motion.getAngles("Head", true);
			faceTracker.stopTracker();

			if (m_IsLoose)
				stiffen(true);

			if (taskName.compare(L"AttentionGrabber") == 0) {
				attentionGrabber();
			}
			else if (taskName.compare(L"Intro") == 0) {
				intro();
			}
			else if (taskName.compare(L"LetUsContinue") == 0) {
				letUsContinue();
			}
			else if (taskName.compare(L"TurnOnWater") == 0) {
				turnOnWater();
			}
			else if (taskName.compare(L"WetYourHands") == 0) {
				wetYourHands();
			}
			else if (taskName.compare(L"GetSomeSoap") == 0) {
				getSomeSoap();
			}
			else if (taskName.compare(L"ScrubYourHands") == 0) {
				scrubYourHands();
			}
			else if (taskName.compare(L"RinseYourHands") == 0) {
				rinseYourHands();
			}
			else if (taskName.compare(L"TurnOffWater") == 0) {
				turnOffWater();
			}
			else if (taskName.compare(L"DryYourHands") == 0) {
				dryYourHands();
			}
			else if (taskName.compare(L"AllDone") == 0) {
				allDone();
			}
			else {
				ASSERT(FALSE);
			}
		}

		//if (led.isRunning(led_id))
		//	led.stop(led_id);
		led.fadeRGB("AllLeds", 0xffffff, 0.1);
	}

	m_hPromptingThread = NULL;
	return 0;
}

void NAO_prompts::stiffen(bool on) {
	m_Stopped = false;
	if (on) {
		faceTracker.stopTracker();
		motion.stiffnessInterpolation("Body", 1.0f, 0.1);
		m_IsLoose = false;
	}
	else {
		faceTracker.startTracker();
		motion.stiffnessInterpolation("Body", 0.0f, 0.1);
		motion.stiffnessInterpolation("Head", 0.6f, 0.1);
		m_IsLoose = true;
	}
}

void NAO_prompts::emergencyStop() { //set stiffness off for all joints immediately
	if (!m_Stopped) {
		motion.setStiffnesses("Body", 0.0f);
		faceTracker.stopTracker();
		m_IsLoose = true;
		m_Stopped = true;
	}
}

void NAO_prompts::immediatePause(bool takeRest) {
	if (m_Stopped) {
		head_angles.resize(2);
		head_angles[0] = DefaultHeadAngle1;
		head_angles[1] = DefaultHeadAngle2;
		stiffen(true);
		takeRest1();
	}
	else if (m_ExitThread == -1) {
		//head_angles.resize(2);
		//head_angles[0] = DefaultHeadAngle1;
		//head_angles[1] = DefaultHeadAngle2;
		m_ExitThread = takeRest ? 1 : 0;
		//while (isPrompting()) {} //wait till exited the prompting loop before continueing
		//WaitForSingleObject(m_hPromptingThread, INFINITE);
	}
}

int NAO_prompts::waitForMotion() {
	while (motion.isRunning(motion_id)) {
		if (m_ExitThread != -1) {
			tts.stopAll();
			motion.stop(motion_id);
			if (m_ExitThread == 1)
				takeRest1();
			//m_ExitThread = -1;
			return -1;
		}
	}
	return 0;
}

void NAO_prompts::convertHandValues() {
	//note when importing motion scripts from Choregraphe:
	//the RHand and LHand values are incorrect, they are small by a factor of 57.3, making the hands always closed.
	//Need to change them to the correct values before executing the motion
	for (int i = 0; i < names.size(); i++) {
		if (names[i].compare("LHand") == 0 || names[i].compare("RHand") == 0) {
			for (int j = 0; j < keys[i].getSize(); j++) {
				keys[i][j][0] = (float) (keys[i][j][0]) * 57.3;
			}
		}
	}
}

void NAO_prompts::convertHandValuesSimplified() {
	//note when importing motion scripts from Choregraphe:
	//the RHand and LHand values are incorrect, they are small by a factor of 57.3, making the hands always closed.
	//Need to change them to the correct values before executing the motion
	for (int i = 0; i < names.size(); i++) {
		if (names[i].compare("LHand") == 0 || names[i].compare("RHand") == 0) {
			for (int j = 0; j < keys[i].getSize(); j++) {
				keys[i][j] = (float) (keys[i][j]) * 57.3;
			}
		}
	}
}

void NAO_prompts::takeRest() {
	if (taskName.compare(L"AllDone") == 0) {
		if (!m_Stopped) {
			names.clear();
			times.clear();
			keys.clear();
			setRestPosture();
			convertHandValues();
			//no face tracking, facing forward
			motion.stiffnessInterpolation("Body", 1.0f, 0.1f);
			head_angles.resize(2);
			head_angles[0] = 0.0f;
			head_angles[1] = 0.0f;
			motion.setAngles("Head", head_angles, 0.2f);
			motion_id = motion.post.angleInterpolationBezier(names, times, keys);
			while (motion.isRunning(motion_id)) {}
			emergencyStop();
		}
	}
	else {
		if (!m_IsLoose || m_Stopped) {
			m_TakeRest = true;
			m_hPromptingThread = CreateThread(NULL, 0, PromptingStaticThread, (PVOID)this, 0, 0);
		}
	}
}

void NAO_prompts::takeRest1() {
	try
	{
		names.clear();
		times.clear();
		keys.clear();
		setRestPosture();
		convertHandValues();
		motion.setAngles("Head", head_angles, 0.2f);
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		while (motion.isRunning(motion_id)) {}
		stiffen(false); //makes m_Stopped = false and m_IsLoose = true
		faceTracker.startTracker();
	}
	catch (const std::exception&)
	{
		m_IsLoose = false;
		ASSERT(FALSE);
	}
}

void NAO_prompts::attentionGrabber() {
	try
	{
		tts.stopAll();
		tts_id = postSay(std::string("Hi, ") + USER_NAME);

		names.clear();
		times.clear();
		keys.clear();
		setWaveMotion();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish
	}
	catch (const std::exception&)
	{
		assert(false);
	}
}

void NAO_prompts::reward() {
	try
	{
		takeRest1(); //no motions needed

		if (led.isRunning(led_id))
			led.stop(led_id);
		led_id = led.post.rasta(1);

		tts.stopAll();
		tts_id = postSay(std::string("\\VCT=100\\Great!")); //higher pitch sounds better

		//names.clear();
		//times.clear();
		//keys.clear();
		//setHoorayMotion();
		//convertHandValues();
		//motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		//if (waitForMotion() == -1)
		//	return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish
		
		if (led.isRunning(led_id))
			led.stop(led_id);
	}
	catch (const std::exception&)
	{
		assert(false);
	}
}

void NAO_prompts::intro() {
	
	attentionGrabber();

	try
	{
		tts.stopAll();
		tts_id = postSay("Let's start washing hands.");

		names.clear();
		times.clear();
		keys.clear();
		setIntroMotion();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish
	}
	catch (const std::exception&)
	{
		assert(false);
	}
}


void NAO_prompts::letUsContinue() {
	
	try
	{
		tts.stopAll();
		tts_id = postSay("Let's continue washing hands.");

		names.clear();
		times.clear();
		keys.clear();
		setIntroMotion();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish
	}
	catch (const std::exception&)
	{
		assert(false);
	}
}

void NAO_prompts::turnOnWater() {
	try
	{
		tts.stopAll();
		tts_id = postSay("Turn on, the water.");

		names.clear();
		times.clear();
		keys.clear();
		setTurnOnWaterMotion1();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		names.clear();
		times.clear();
		keys.clear();
		setTurnOnWaterMotion2();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish
	}
	catch (const std::exception&)
	{
		assert(false);
	}
}

void NAO_prompts::wetYourHands() {
	try
	{
		tts.stopAll();
		tts_id = postSay("Wet, your hands.");

		names.clear();
		times.clear();
		keys.clear();
		setWetYourHandsMotion1();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		names.clear();
		times.clear();
		keys.clear();
		setWetYourHandsMotion2();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish
	}
	catch (const std::exception&)
	{
		assert(false);
	}
}

void NAO_prompts::getSomeSoap() {
	try
	{
		tts.stopAll();
		tts_id = postSay("Get, some soap.");

		names.clear();
		times.clear();
		keys.clear();
		setGetSomeSoapMotion1();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		names.clear();
		times.clear();
		keys.clear();
		setGetSomeSoapMotion2();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish
	}
	catch (const std::exception&)
	{
		assert(false);
	}
}

void NAO_prompts::scrubYourHands() {
	try
	{
		tts.stopAll();
		tts_id = postSay("Scrub, your hands.");

		names.clear();
		times.clear();
		keys.clear();
		setScrubYourHandsMotion1();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		//names.clear();
		//times.clear();
		//keys.clear();
		//setScrubYourHandsMotion2();
		//convertHandValues();
		//motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		//if (waitForMotion() == -1)
		//	return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish
	}
	catch (const std::exception&)
	{
		assert(false);
	}
}

void NAO_prompts::rinseYourHands() {
	try
	{
		tts.stopAll();
		tts_id = postSay("Rinse, your hands.");

		names.clear();
		times.clear();
		keys.clear();
		setRinseYourHandsMotion1();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		names.clear();
		times.clear();
		keys.clear();
		setRinseYourHandsMotion2();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish
	}
	catch (const std::exception&)
	{
		assert(false);
	}
}

void NAO_prompts::turnOffWater() {
	try
	{
		tts.stopAll();
		tts_id = postSay("Turn off, the water.");

		names.clear();
		times.clear();
		keys.clear();
		setTurnOffWaterMotion1();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		names.clear();
		times.clear();
		keys.clear();
		setTurnOffWaterMotion2();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish
	}
	catch (const std::exception&)
	{
		assert(false);
	}
}

void NAO_prompts::dryYourHands() {
	try
	{
		tts.stopAll();
		tts_id = postSay("Dry, your hands.");

		names.clear();
		times.clear();
		keys.clear();
		setDryYourHandsMotion1();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		names.clear();
		times.clear();
		keys.clear();
		setDryYourHandsMotion2();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish
	}
	catch (const std::exception&)
	{
		assert(false);
	}
}


void NAO_prompts::allDone() {
	try
	{

		if (led.isRunning(led_id))
			led.stop(led_id);
		led_id = led.post.rasta(10);

		tts.stopAll();

		tts_id = postSay(std::string("Good job, ") + USER_NAME);

		names.clear();
		times.clear();
		keys.clear();
		setHoorayMotion();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish
		
		tts_id = postSay("You are all done!");

		names.clear();
		times.clear();
		keys.clear();
		setAllDoneMotion();
		convertHandValues();
		motion_id = motion.post.angleInterpolationBezier(names, times, keys);
		if (waitForMotion() == -1)
			return;

		while (tts.isRunning(tts_id)) {} //wait for speech to finish

		if (led.isRunning(led_id))
			led.stop(led_id);

	}
	catch (const std::exception&)
	{
		assert(false);
	}
}

void NAO_prompts::setRestPosture() {
	names.reserve(12);
	times.arraySetSize(12);
	keys.arraySetSize(12);

	names.push_back("LElbowRoll");
	times[0].arraySetSize(1);
	keys[0].arraySetSize(1);

	times[0][0] = 0.680000;
	keys[0][0] = AL::ALValue::array(-1.13052, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[1].arraySetSize(1);
	keys[1].arraySetSize(1);

	times[1][0] = 0.680000;
	keys[1][0] = AL::ALValue::array(-1.20730, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[2].arraySetSize(1);
	keys[2].arraySetSize(1);

	times[2][0] = 0.680000;
	keys[2][0] = AL::ALValue::array(0.00609469, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[3].arraySetSize(1);
	keys[3].arraySetSize(1);

	times[3][0] = 0.680000;
	keys[3][0] = AL::ALValue::array(1.25017, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[4].arraySetSize(1);
	keys[4].arraySetSize(1);

	times[4][0] = 0.680000;
	keys[4][0] = AL::ALValue::array(0.00916204, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[5].arraySetSize(1);
	keys[5].arraySetSize(1);

	times[5][0] = 0.680000;
	keys[5][0] = AL::ALValue::array(-0.0644700, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[6].arraySetSize(1);
	keys[6].arraySetSize(1);

	times[6][0] = 0.680000;
	keys[6][0] = AL::ALValue::array(1.13052, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[7].arraySetSize(1);
	keys[7].arraySetSize(1);

	times[7][0] = 0.680000;
	keys[7][0] = AL::ALValue::array(1.20730, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[8].arraySetSize(1);
	keys[8].arraySetSize(1);

	times[8][0] = 0.680000;
	keys[8][0] = AL::ALValue::array(0.00609469, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[9].arraySetSize(1);
	keys[9].arraySetSize(1);

	times[9][0] = 0.680000;
	keys[9][0] = AL::ALValue::array(1.25017, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[10].arraySetSize(1);
	keys[10].arraySetSize(1);

	times[10][0] = 0.680000;
	keys[10][0] = AL::ALValue::array(-0.00916204, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[11].arraySetSize(1);
	keys[11].arraySetSize(1);

	times[11][0] = 0.680000;
	keys[11][0] = AL::ALValue::array(0.0644700, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));
}

void NAO_prompts::setHoorayMotion() {
	names.reserve(12);
	times.arraySetSize(12);
	keys.arraySetSize(12);

	names.push_back("LElbowRoll");
	times[0].arraySetSize(5);
	keys[0].arraySetSize(5);

	times[0][0] = 0.840000;
	keys[0][0] = AL::ALValue::array(-0.598765, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.133333, 0.000000));
	times[0][1] = 1.24000;
	keys[0][1] = AL::ALValue::array(-0.577920, AL::ALValue::array(3, -0.133333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][2] = 1.48000;
	keys[0][2] = AL::ALValue::array(-0.577920, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][3] = 1.72000;
	keys[0][3] = AL::ALValue::array(-0.577920, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][4] = 1.96000;
	keys[0][4] = AL::ALValue::array(-0.577920, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[1].arraySetSize(5);
	keys[1].arraySetSize(5);

	times[1][0] = 0.840000;
	keys[1][0] = AL::ALValue::array(-2.06931, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.133333, 0.000000));
	times[1][1] = 1.24000;
	keys[1][1] = AL::ALValue::array(-2.08167, AL::ALValue::array(3, -0.133333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][2] = 1.48000;
	keys[1][2] = AL::ALValue::array(-2.08167, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][3] = 1.72000;
	keys[1][3] = AL::ALValue::array(-2.08167, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][4] = 1.96000;
	keys[1][4] = AL::ALValue::array(-2.08167, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[2].arraySetSize(1);
	keys[2].arraySetSize(1);

	times[2][0] = 0.840000;
	keys[2][0] = AL::ALValue::array(0.00351649, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[3].arraySetSize(5);
	keys[3].arraySetSize(5);

	times[3][0] = 0.840000;
	keys[3][0] = AL::ALValue::array(1.64360, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.133333, 0.000000));
	times[3][1] = 1.24000;
	keys[3][1] = AL::ALValue::array(1.66208, AL::ALValue::array(3, -0.133333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][2] = 1.48000;
	keys[3][2] = AL::ALValue::array(1.66208, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][3] = 1.72000;
	keys[3][3] = AL::ALValue::array(1.66208, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][4] = 1.96000;
	keys[3][4] = AL::ALValue::array(1.66208, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[4].arraySetSize(5);
	keys[4].arraySetSize(5);

	times[4][0] = 0.840000;
	keys[4][0] = AL::ALValue::array(0.118076, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.133333, 0.000000));
	times[4][1] = 1.24000;
	keys[4][1] = AL::ALValue::array(0.116542, AL::ALValue::array(3, -0.133333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][2] = 1.48000;
	keys[4][2] = AL::ALValue::array(0.116542, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][3] = 1.72000;
	keys[4][3] = AL::ALValue::array(0.116542, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][4] = 1.96000;
	keys[4][4] = AL::ALValue::array(0.116542, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[5].arraySetSize(1);
	keys[5].arraySetSize(1);

	times[5][0] = 0.840000;
	keys[5][0] = AL::ALValue::array(-0.621405, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[6].arraySetSize(5);
	keys[6].arraySetSize(5);

	times[6][0] = 0.840000;
	keys[6][0] = AL::ALValue::array(1.43587, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.133333, 0.000000));
	times[6][1] = 1.24000;
	keys[6][1] = AL::ALValue::array(1.54462, AL::ALValue::array(3, -0.133333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][2] = 1.48000;
	keys[6][2] = AL::ALValue::array(0.638187, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][3] = 1.72000;
	keys[6][3] = AL::ALValue::array(1.54462, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][4] = 1.96000;
	keys[6][4] = AL::ALValue::array(0.638187, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[7].arraySetSize(5);
	keys[7].arraySetSize(5);

	times[7][0] = 0.840000;
	keys[7][0] = AL::ALValue::array(-0.0736740, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.133333, 0.000000));
	times[7][1] = 1.24000;
	keys[7][1] = AL::ALValue::array(1.76098, AL::ALValue::array(3, -0.133333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][2] = 1.48000;
	keys[7][2] = AL::ALValue::array(1.48334, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][3] = 1.72000;
	keys[7][3] = AL::ALValue::array(1.76098, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][4] = 1.96000;
	keys[7][4] = AL::ALValue::array(1.48334, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[8].arraySetSize(5);
	keys[8].arraySetSize(5);

	times[8][0] = 0.840000;
	keys[8][0] = AL::ALValue::array(0.000279713, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.133333, 0.000000));
	times[8][1] = 1.24000;
	keys[8][1] = AL::ALValue::array(0.00404218, AL::ALValue::array(3, -0.133333, -2.32710e-05), AL::ALValue::array(3, 0.0800000, 1.39626e-05));
	times[8][2] = 1.48000;
	keys[8][2] = AL::ALValue::array(0.00405614, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[8][3] = 1.72000;
	keys[8][3] = AL::ALValue::array(0.00404218, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[8][4] = 1.96000;
	keys[8][4] = AL::ALValue::array(0.00405614, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[9].arraySetSize(5);
	keys[9].arraySetSize(5);

	times[9][0] = 0.840000;
	keys[9][0] = AL::ALValue::array(0.383541, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.133333, 0.000000));
	times[9][1] = 1.24000;
	keys[9][1] = AL::ALValue::array(0.159578, AL::ALValue::array(3, -0.133333, 0.223963), AL::ALValue::array(3, 0.0800000, -0.134378));
	times[9][2] = 1.48000;
	keys[9][2] = AL::ALValue::array(-0.897349, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][3] = 1.72000;
	keys[9][3] = AL::ALValue::array(0.159578, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][4] = 1.96000;
	keys[9][4] = AL::ALValue::array(-0.897349, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[10].arraySetSize(5);
	keys[10].arraySetSize(5);

	times[10][0] = 0.840000;
	keys[10][0] = AL::ALValue::array(-1.13520, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.133333, 0.000000));
	times[10][1] = 1.24000;
	keys[10][1] = AL::ALValue::array(-0.394280, AL::ALValue::array(3, -0.133333, -0.224348), AL::ALValue::array(3, 0.0800000, 0.134609));
	times[10][2] = 1.48000;
	keys[10][2] = AL::ALValue::array(-0.0583340, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][3] = 1.72000;
	keys[10][3] = AL::ALValue::array(-0.394280, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][4] = 1.96000;
	keys[10][4] = AL::ALValue::array(-0.0583340, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[11].arraySetSize(5);
	keys[11].arraySetSize(5);

	times[11][0] = 0.840000;
	keys[11][0] = AL::ALValue::array(0.0168320, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.133333, 0.000000));
	times[11][1] = 1.24000;
	keys[11][1] = AL::ALValue::array(-0.193327, AL::ALValue::array(3, -0.133333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][2] = 1.48000;
	keys[11][2] = AL::ALValue::array(0.585945, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][3] = 1.72000;
	keys[11][3] = AL::ALValue::array(-0.193327, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][4] = 1.96000;
	keys[11][4] = AL::ALValue::array(0.585945, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));
}

void NAO_prompts::setWaveMotion() {
	names.reserve(12);
	times.arraySetSize(12);
	keys.arraySetSize(12);

	names.push_back("LElbowRoll");
	times[0].arraySetSize(4);
	keys[0].arraySetSize(4);

	times[0][0] = 0.520000;
	keys[0][0] = AL::ALValue::array(-1.37902, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[0][1] = 1.00000;
	keys[0][1] = AL::ALValue::array(-1.29005, AL::ALValue::array(3, -0.160000, -0.0392706), AL::ALValue::array(3, 0.106667, 0.0261804));
	times[0][2] = 1.32000;
	keys[0][2] = AL::ALValue::array(-1.18267, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[0][3] = 1.64000;
	keys[0][3] = AL::ALValue::array(-1.24863, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[1].arraySetSize(4);
	keys[1].arraySetSize(4);

	times[1][0] = 0.520000;
	keys[1][0] = AL::ALValue::array(-0.803859, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[1][1] = 1.00000;
	keys[1][1] = AL::ALValue::array(-0.691876, AL::ALValue::array(3, -0.160000, -0.0184098), AL::ALValue::array(3, 0.106667, 0.0122732));
	times[1][2] = 1.32000;
	keys[1][2] = AL::ALValue::array(-0.679603, AL::ALValue::array(3, -0.106667, -0.0122732), AL::ALValue::array(3, 0.106667, 0.0122732));
	times[1][3] = 1.64000;
	keys[1][3] = AL::ALValue::array(-0.610574, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[2].arraySetSize(1);
	keys[2].arraySetSize(1);

	times[2][0] = 1.00000;
	keys[2][0] = AL::ALValue::array(0.00415750, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[3].arraySetSize(4);
	keys[3].arraySetSize(4);

	times[3][0] = 0.520000;
	keys[3][0] = AL::ALValue::array(1.11824, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[3][1] = 1.00000;
	keys[3][1] = AL::ALValue::array(0.928028, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[3][2] = 1.32000;
	keys[3][2] = AL::ALValue::array(0.940300, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[3][3] = 1.64000;
	keys[3][3] = AL::ALValue::array(0.862065, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[4].arraySetSize(4);
	keys[4].arraySetSize(4);

	times[4][0] = 0.520000;
	keys[4][0] = AL::ALValue::array(0.363515, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[4][1] = 1.00000;
	keys[4][1] = AL::ALValue::array(0.226991, AL::ALValue::array(3, -0.160000, 0.0319071), AL::ALValue::array(3, 0.106667, -0.0212714));
	times[4][2] = 1.32000;
	keys[4][2] = AL::ALValue::array(0.203980, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[4][3] = 1.64000;
	keys[4][3] = AL::ALValue::array(0.217786, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[5].arraySetSize(1);
	keys[5].arraySetSize(1);

	times[5][0] = 1.00000;
	keys[5][0] = AL::ALValue::array(0.147222, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[6].arraySetSize(8);
	keys[6].arraySetSize(8);

	times[6][0] = 0.440000;
	keys[6][0] = AL::ALValue::array(1.38524, AL::ALValue::array(3, -0.146667, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[6][1] = 0.960000;
	keys[6][1] = AL::ALValue::array(0.242414, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.0133333, 0.000000));
	times[6][2] = 1.00000;
	keys[6][2] = AL::ALValue::array(0.349066, AL::ALValue::array(3, -0.0133333, -0.0288264), AL::ALValue::array(3, 0.0933333, 0.201785));
	times[6][3] = 1.28000;
	keys[6][3] = AL::ALValue::array(0.934249, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0133333, 0.000000));
	times[6][4] = 1.32000;
	keys[6][4] = AL::ALValue::array(0.680678, AL::ALValue::array(3, -0.0133333, 0.0309276), AL::ALValue::array(3, 0.0933333, -0.216493));
	times[6][5] = 1.60000;
	keys[6][5] = AL::ALValue::array(0.191986, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0133333, 0.000000));
	times[6][6] = 1.64000;
	keys[6][6] = AL::ALValue::array(0.261799, AL::ALValue::array(3, -0.0133333, -0.0190826), AL::ALValue::array(3, 0.106667, 0.152661));
	times[6][7] = 1.96000;
	keys[6][7] = AL::ALValue::array(0.707216, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[7].arraySetSize(5);
	keys[7].arraySetSize(5);

	times[7][0] = 0.440000;
	keys[7][0] = AL::ALValue::array(-0.312978, AL::ALValue::array(3, -0.146667, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[7][1] = 0.960000;
	keys[7][1] = AL::ALValue::array(0.564471, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[7][2] = 1.28000;
	keys[7][2] = AL::ALValue::array(0.391128, AL::ALValue::array(3, -0.106667, 0.0360492), AL::ALValue::array(3, 0.106667, -0.0360492));
	times[7][3] = 1.60000;
	keys[7][3] = AL::ALValue::array(0.348176, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.120000, 0.000000));
	times[7][4] = 1.96000;
	keys[7][4] = AL::ALValue::array(0.381923, AL::ALValue::array(3, -0.120000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[8].arraySetSize(2);
	keys[8].arraySetSize(2);

	times[8][0] = 0.960000;
	keys[8][0] = AL::ALValue::array(0.0148960, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.333333, 0.000000));
	times[8][1] = 1.96000;
	keys[8][1] = AL::ALValue::array(0.0149214, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[9].arraySetSize(5);
	keys[9].arraySetSize(5);

	times[9][0] = 0.440000;
	keys[9][0] = AL::ALValue::array(0.247016, AL::ALValue::array(3, -0.146667, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[9][1] = 0.960000;
	keys[9][1] = AL::ALValue::array(-1.17193, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[9][2] = 1.28000;
	keys[9][2] = AL::ALValue::array(-1.08910, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[9][3] = 1.60000;
	keys[9][3] = AL::ALValue::array(-1.26091, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.120000, 0.000000));
	times[9][4] = 1.96000;
	keys[9][4] = AL::ALValue::array(-1.14892, AL::ALValue::array(3, -0.120000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[10].arraySetSize(5);
	keys[10].arraySetSize(5);

	times[10][0] = 0.440000;
	keys[10][0] = AL::ALValue::array(-0.242414, AL::ALValue::array(3, -0.146667, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[10][1] = 0.960000;
	keys[10][1] = AL::ALValue::array(-0.954191, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[10][2] = 1.28000;
	keys[10][2] = AL::ALValue::array(-0.460242, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[10][3] = 1.60000;
	keys[10][3] = AL::ALValue::array(-0.960325, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.120000, 0.000000));
	times[10][4] = 1.96000;
	keys[10][4] = AL::ALValue::array(-0.328317, AL::ALValue::array(3, -0.120000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[11].arraySetSize(2);
	keys[11].arraySetSize(2);

	times[11][0] = 0.960000;
	keys[11][0] = AL::ALValue::array(-0.312978, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.333333, 0.000000));
	times[11][1] = 1.96000;
	keys[11][1] = AL::ALValue::array(-0.303775, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));
}


void NAO_prompts::setDryYourHandsMotion1() {
	names.reserve(14);
	times.arraySetSize(14);
	keys.arraySetSize(14);

	names.push_back("HeadPitch");
	times[0].arraySetSize(6);
	keys[0].arraySetSize(6);

	times[0][0] = 0.800000;
	keys[0][0] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[0][1] = 1.08000;
	keys[0][1] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][2] = 1.32000;
	keys[0][2] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[0][3] = 1.80000;
	keys[0][3] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[0][4] = 2.08000;
	keys[0][4] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][5] = 2.32000;
	keys[0][5] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("HeadYaw");
	times[1].arraySetSize(6);
	keys[1].arraySetSize(6);

	times[1][0] = 0.800000;
	keys[1][0] = AL::ALValue::array(-0.0107800, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[1][1] = 1.08000;
	keys[1][1] = AL::ALValue::array(-0.0107800, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][2] = 1.32000;
	keys[1][2] = AL::ALValue::array(-0.0107800, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[1][3] = 1.80000;
	keys[1][3] = AL::ALValue::array(-0.00771196, AL::ALValue::array(3, -0.160000, -0.000968841), AL::ALValue::array(3, 0.0933333, 0.000565158));
	times[1][4] = 2.08000;
	keys[1][4] = AL::ALValue::array(-0.00617796, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][5] = 2.32000;
	keys[1][5] = AL::ALValue::array(-0.00617796, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowRoll");
	times[2].arraySetSize(6);
	keys[2].arraySetSize(6);

	times[2][0] = 0.800000;
	keys[2][0] = AL::ALValue::array(-0.987855, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[2][1] = 1.08000;
	keys[2][1] = AL::ALValue::array(-0.986320, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[2][2] = 1.32000;
	keys[2][2] = AL::ALValue::array(-0.986320, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[2][3] = 1.80000;
	keys[2][3] = AL::ALValue::array(-1.08296, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[2][4] = 2.08000;
	keys[2][4] = AL::ALValue::array(-1.08296, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[2][5] = 2.32000;
	keys[2][5] = AL::ALValue::array(-1.08296, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[3].arraySetSize(6);
	keys[3].arraySetSize(6);

	times[3][0] = 0.800000;
	keys[3][0] = AL::ALValue::array(-0.0552660, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[3][1] = 1.08000;
	keys[3][1] = AL::ALValue::array(-0.147306, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][2] = 1.32000;
	keys[3][2] = AL::ALValue::array(-0.147306, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[3][3] = 1.80000;
	keys[3][3] = AL::ALValue::array(-1.08305, AL::ALValue::array(3, -0.160000, 0.0184067), AL::ALValue::array(3, 0.0933333, -0.0107373));
	times[3][4] = 2.08000;
	keys[3][4] = AL::ALValue::array(-1.09378, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][5] = 2.32000;
	keys[3][5] = AL::ALValue::array(-1.09378, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[4].arraySetSize(6);
	keys[4].arraySetSize(6);

	times[4][0] = 0.800000;
	keys[4][0] = AL::ALValue::array(0.0156451, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[4][1] = 1.08000;
	keys[4][1] = AL::ALValue::array(0.0156451, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][2] = 1.32000;
	keys[4][2] = AL::ALValue::array(0.0156451, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[4][3] = 1.80000;
	keys[4][3] = AL::ALValue::array(0.0156382, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[4][4] = 2.08000;
	keys[4][4] = AL::ALValue::array(0.0156382, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][5] = 2.32000;
	keys[4][5] = AL::ALValue::array(0.0156382, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[5].arraySetSize(6);
	keys[5].arraySetSize(6);

	times[5][0] = 0.800000;
	keys[5][0] = AL::ALValue::array(-0.170316, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[5][1] = 1.08000;
	keys[5][1] = AL::ALValue::array(0.168698, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[5][2] = 1.32000;
	keys[5][2] = AL::ALValue::array(0.168698, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[5][3] = 1.80000;
	keys[5][3] = AL::ALValue::array(0.249999, AL::ALValue::array(3, -0.160000, -0.0813011), AL::ALValue::array(3, 0.0933333, 0.0474256));
	times[5][4] = 2.08000;
	keys[5][4] = AL::ALValue::array(0.756220, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[5][5] = 2.32000;
	keys[5][5] = AL::ALValue::array(0.756220, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[6].arraySetSize(6);
	keys[6].arraySetSize(6);

	times[6][0] = 0.800000;
	keys[6][0] = AL::ALValue::array(0.00149204, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[6][1] = 1.08000;
	keys[6][1] = AL::ALValue::array(-0.00157596, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][2] = 1.32000;
	keys[6][2] = AL::ALValue::array(-0.00157596, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[6][3] = 1.80000;
	keys[6][3] = AL::ALValue::array(-0.314159, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[6][4] = 2.08000;
	keys[6][4] = AL::ALValue::array(-0.314159, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][5] = 2.32000;
	keys[6][5] = AL::ALValue::array(-0.314159, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[7].arraySetSize(6);
	keys[7].arraySetSize(6);

	times[7][0] = 0.800000;
	keys[7][0] = AL::ALValue::array(0.0735901, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[7][1] = 1.08000;
	keys[7][1] = AL::ALValue::array(0.0735901, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][2] = 1.32000;
	keys[7][2] = AL::ALValue::array(0.0735901, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[7][3] = 1.80000;
	keys[7][3] = AL::ALValue::array(-1.82387, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[7][4] = 2.08000;
	keys[7][4] = AL::ALValue::array(-1.82387, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][5] = 2.32000;
	keys[7][5] = AL::ALValue::array(-1.82387, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[8].arraySetSize(6);
	keys[8].arraySetSize(6);

	times[8][0] = 0.800000;
	keys[8][0] = AL::ALValue::array(0.943452, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[8][1] = 1.08000;
	keys[8][1] = AL::ALValue::array(0.943452, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[8][2] = 1.32000;
	keys[8][2] = AL::ALValue::array(0.943452, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[8][3] = 1.80000;
	keys[8][3] = AL::ALValue::array(0.906636, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[8][4] = 2.08000;
	keys[8][4] = AL::ALValue::array(0.908170, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[8][5] = 2.32000;
	keys[8][5] = AL::ALValue::array(0.908170, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[9].arraySetSize(6);
	keys[9].arraySetSize(6);

	times[9][0] = 0.800000;
	keys[9][0] = AL::ALValue::array(0.934165, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[9][1] = 1.08000;
	keys[9][1] = AL::ALValue::array(0.935697, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][2] = 1.32000;
	keys[9][2] = AL::ALValue::array(0.935697, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[9][3] = 1.80000;
	keys[9][3] = AL::ALValue::array(0.937231, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[9][4] = 2.08000;
	keys[9][4] = AL::ALValue::array(0.937231, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][5] = 2.32000;
	keys[9][5] = AL::ALValue::array(0.937231, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[10].arraySetSize(6);
	keys[10].arraySetSize(6);

	times[10][0] = 0.800000;
	keys[10][0] = AL::ALValue::array(0.0143606, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[10][1] = 1.08000;
	keys[10][1] = AL::ALValue::array(0.0143606, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][2] = 1.32000;
	keys[10][2] = AL::ALValue::array(0.0143606, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[10][3] = 1.80000;
	keys[10][3] = AL::ALValue::array(0.0143466, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[10][4] = 2.08000;
	keys[10][4] = AL::ALValue::array(0.0143466, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][5] = 2.32000;
	keys[10][5] = AL::ALValue::array(0.0143466, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[11].arraySetSize(6);
	keys[11].arraySetSize(6);

	times[11][0] = 0.800000;
	keys[11][0] = AL::ALValue::array(0.862151, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[11][1] = 1.08000;
	keys[11][1] = AL::ALValue::array(0.883625, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][2] = 1.32000;
	keys[11][2] = AL::ALValue::array(0.883625, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[11][3] = 1.80000;
	keys[11][3] = AL::ALValue::array(0.891296, AL::ALValue::array(3, -0.160000, -0.00419835), AL::ALValue::array(3, 0.0933333, 0.00244904));
	times[11][4] = 2.08000;
	keys[11][4] = AL::ALValue::array(0.903567, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][5] = 2.32000;
	keys[11][5] = AL::ALValue::array(0.903567, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[12].arraySetSize(6);
	keys[12].arraySetSize(6);

	times[12][0] = 0.800000;
	keys[12][0] = AL::ALValue::array(0.210117, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[12][1] = 1.08000;
	keys[12][1] = AL::ALValue::array(0.210117, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[12][2] = 1.32000;
	keys[12][2] = AL::ALValue::array(0.210117, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[12][3] = 1.80000;
	keys[12][3] = AL::ALValue::array(0.170232, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[12][4] = 2.08000;
	keys[12][4] = AL::ALValue::array(0.170232, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[12][5] = 2.32000;
	keys[12][5] = AL::ALValue::array(0.170232, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[13].arraySetSize(6);
	keys[13].arraySetSize(6);

	times[13][0] = 0.800000;
	keys[13][0] = AL::ALValue::array(1.82387, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[13][1] = 1.08000;
	keys[13][1] = AL::ALValue::array(1.82387, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[13][2] = 1.32000;
	keys[13][2] = AL::ALValue::array(1.82387, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[13][3] = 1.80000;
	keys[13][3] = AL::ALValue::array(1.82387, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[13][4] = 2.08000;
	keys[13][4] = AL::ALValue::array(1.82387, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[13][5] = 2.32000;
	keys[13][5] = AL::ALValue::array(1.82387, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));
}
void NAO_prompts::setDryYourHandsMotion2() {
	names.reserve(8);
	times.arraySetSize(8);
	keys.arraySetSize(8);

	names.push_back("HeadPitch");
	times[0].arraySetSize(2);
	keys[0].arraySetSize(2);

	times[0][0] = 0.800000;
	keys[0][0] = AL::ALValue::array(-0.0874799, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[0][1] = 1.52000;
	keys[0][1] = AL::ALValue::array(-0.0874799, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("HeadYaw");
	times[1].arraySetSize(2);
	keys[1].arraySetSize(2);

	times[1][0] = 0.800000;
	keys[1][0] = AL::ALValue::array(0.516916, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[1][1] = 1.52000;
	keys[1][1] = AL::ALValue::array(0.516916, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowRoll");
	times[2].arraySetSize(2);
	keys[2].arraySetSize(2);

	times[2][0] = 0.800000;
	keys[2][0] = AL::ALValue::array(-0.0367741, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[2][1] = 1.52000;
	keys[2][1] = AL::ALValue::array(-0.0367741, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[3].arraySetSize(2);
	keys[3].arraySetSize(2);

	times[3][0] = 0.800000;
	keys[3][0] = AL::ALValue::array(-0.920441, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[3][1] = 1.52000;
	keys[3][1] = AL::ALValue::array(-0.920441, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[4].arraySetSize(2);
	keys[4].arraySetSize(2);

	times[4][0] = 0.800000;
	keys[4][0] = AL::ALValue::array(0.0152263, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[4][1] = 1.52000;
	keys[4][1] = AL::ALValue::array(0.0152263, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[5].arraySetSize(2);
	keys[5].arraySetSize(2);

	times[5][0] = 0.800000;
	keys[5][0] = AL::ALValue::array(-0.102820, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[5][1] = 1.52000;
	keys[5][1] = AL::ALValue::array(-0.102820, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[6].arraySetSize(2);
	keys[6].arraySetSize(2);

	times[6][0] = 0.800000;
	keys[6][0] = AL::ALValue::array(0.532256, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[6][1] = 1.52000;
	keys[6][1] = AL::ALValue::array(0.532256, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[7].arraySetSize(2);
	keys[7].arraySetSize(2);

	times[7][0] = 0.800000;
	keys[7][0] = AL::ALValue::array(0.670316, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[7][1] = 1.52000;
	keys[7][1] = AL::ALValue::array(0.670316, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));
}


void NAO_prompts::setTurnOffWaterMotion1() {
	names.reserve(12);
	times.arraySetSize(12);
	keys.arraySetSize(12);

	names.push_back("LElbowRoll");
	times[0].arraySetSize(4);
	keys[0].arraySetSize(4);

	times[0][0] = 0.600000;
	keys[0][0] = AL::ALValue::array(-1.12898, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0666667, 0.000000));
	times[0][1] = 0.800000;
	keys[0][1] = AL::ALValue::array(-1.12898, AL::ALValue::array(3, -0.0666667, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][2] = 1.04000;
	keys[0][2] = AL::ALValue::array(-1.12898, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[0][3] = 1.32000;
	keys[0][3] = AL::ALValue::array(-1.12898, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[1].arraySetSize(4);
	keys[1].arraySetSize(4);

	times[1][0] = 0.600000;
	keys[1][0] = AL::ALValue::array(-1.19963, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0666667, 0.000000));
	times[1][1] = 0.800000;
	keys[1][1] = AL::ALValue::array(-1.19963, AL::ALValue::array(3, -0.0666667, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][2] = 1.04000;
	keys[1][2] = AL::ALValue::array(-1.19963, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[1][3] = 1.32000;
	keys[1][3] = AL::ALValue::array(-1.19963, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[2].arraySetSize(4);
	keys[2].arraySetSize(4);

	times[2][0] = 0.600000;
	keys[2][0] = AL::ALValue::array(0.00609469, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0666667, 0.000000));
	times[2][1] = 0.800000;
	keys[2][1] = AL::ALValue::array(0.00609469, AL::ALValue::array(3, -0.0666667, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[2][2] = 1.04000;
	keys[2][2] = AL::ALValue::array(0.00609469, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[2][3] = 1.32000;
	keys[2][3] = AL::ALValue::array(0.00609469, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[3].arraySetSize(4);
	keys[3].arraySetSize(4);

	times[3][0] = 0.600000;
	keys[3][0] = AL::ALValue::array(1.24863, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0666667, 0.000000));
	times[3][1] = 0.800000;
	keys[3][1] = AL::ALValue::array(1.24863, AL::ALValue::array(3, -0.0666667, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][2] = 1.04000;
	keys[3][2] = AL::ALValue::array(1.24863, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[3][3] = 1.32000;
	keys[3][3] = AL::ALValue::array(1.24863, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[4].arraySetSize(4);
	keys[4].arraySetSize(4);

	times[4][0] = 0.600000;
	keys[4][0] = AL::ALValue::array(-0.00310997, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0666667, 0.000000));
	times[4][1] = 0.800000;
	keys[4][1] = AL::ALValue::array(-0.00157596, AL::ALValue::array(3, -0.0666667, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][2] = 1.04000;
	keys[4][2] = AL::ALValue::array(-0.00310997, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[4][3] = 1.32000;
	keys[4][3] = AL::ALValue::array(-0.00157596, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[5].arraySetSize(4);
	keys[5].arraySetSize(4);

	times[5][0] = 0.600000;
	keys[5][0] = AL::ALValue::array(-0.0644700, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0666667, 0.000000));
	times[5][1] = 0.800000;
	keys[5][1] = AL::ALValue::array(-0.0644700, AL::ALValue::array(3, -0.0666667, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[5][2] = 1.04000;
	keys[5][2] = AL::ALValue::array(-0.0644700, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[5][3] = 1.32000;
	keys[5][3] = AL::ALValue::array(-0.0644700, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[6].arraySetSize(4);
	keys[6].arraySetSize(4);

	times[6][0] = 0.600000;
	keys[6][0] = AL::ALValue::array(0.721022, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0666667, 0.000000));
	times[6][1] = 0.800000;
	keys[6][1] = AL::ALValue::array(0.734827, AL::ALValue::array(3, -0.0666667, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][2] = 1.04000;
	keys[6][2] = AL::ALValue::array(0.721022, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[6][3] = 1.32000;
	keys[6][3] = AL::ALValue::array(0.734827, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[7].arraySetSize(4);
	keys[7].arraySetSize(4);

	times[7][0] = 0.600000;
	keys[7][0] = AL::ALValue::array(0.0966000, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0666667, 0.000000));
	times[7][1] = 0.800000;
	keys[7][1] = AL::ALValue::array(0.0904641, AL::ALValue::array(3, -0.0666667, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][2] = 1.04000;
	keys[7][2] = AL::ALValue::array(0.0966000, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[7][3] = 1.32000;
	keys[7][3] = AL::ALValue::array(0.0904641, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[8].arraySetSize(4);
	keys[8].arraySetSize(4);

	times[8][0] = 0.600000;
	keys[8][0] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0666667, 0.000000));
	times[8][1] = 0.800000;
	keys[8][1] = AL::ALValue::array(0.00508938, AL::ALValue::array(3, -0.0666667, 0.000895936), AL::ALValue::array(3, 0.0800000, -0.00107512));
	times[8][2] = 1.04000;
	keys[8][2] = AL::ALValue::array(0.00401426, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[8][3] = 1.32000;
	keys[8][3] = AL::ALValue::array(0.00508938, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[9].arraySetSize(4);
	keys[9].arraySetSize(4);

	times[9][0] = 0.600000;
	keys[9][0] = AL::ALValue::array(0.141170, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0666667, 0.000000));
	times[9][1] = 0.800000;
	keys[9][1] = AL::ALValue::array(0.202530, AL::ALValue::array(3, -0.0666667, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][2] = 1.04000;
	keys[9][2] = AL::ALValue::array(0.141170, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[9][3] = 1.32000;
	keys[9][3] = AL::ALValue::array(0.202530, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[10].arraySetSize(4);
	keys[10].arraySetSize(4);

	times[10][0] = 0.600000;
	keys[10][0] = AL::ALValue::array(-0.237812, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0666667, 0.000000));
	times[10][1] = 0.800000;
	keys[10][1] = AL::ALValue::array(-0.216335, AL::ALValue::array(3, -0.0666667, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][2] = 1.04000;
	keys[10][2] = AL::ALValue::array(-0.237812, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[10][3] = 1.32000;
	keys[10][3] = AL::ALValue::array(-0.216335, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[11].arraySetSize(4);
	keys[11].arraySetSize(4);

	times[11][0] = 0.600000;
	keys[11][0] = AL::ALValue::array(-0.220938, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0666667, 0.000000));
	times[11][1] = 0.800000;
	keys[11][1] = AL::ALValue::array(0.839057, AL::ALValue::array(3, -0.0666667, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][2] = 1.04000;
	keys[11][2] = AL::ALValue::array(-0.220938, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[11][3] = 1.32000;
	keys[11][3] = AL::ALValue::array(0.839057, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));
}
void NAO_prompts::setTurnOffWaterMotion2() {
	setTurnOnWaterMotion2();//points to the tap the same way before
}


void NAO_prompts::setRinseYourHandsMotion1() {
	setWetYourHandsMotion1();
}
void NAO_prompts::setRinseYourHandsMotion2() {
	setWetYourHandsMotion2();
}

void NAO_prompts::setScrubYourHandsMotion1() {

	names.reserve(14);
	times.arraySetSize(14);
	keys.arraySetSize(14);

	names.push_back("HeadPitch");
	times[0].arraySetSize(12);
	keys[0].arraySetSize(12);

	times[0][0] = 0.720000;
	keys[0][0] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][1] = 0.960000;
	keys[0][1] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][2] = 1.20000;
	keys[0][2] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][3] = 1.44000;
	keys[0][3] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[0][4] = 1.92000;
	keys[0][4] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[0][5] = 2.20000;
	keys[0][5] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][6] = 2.44000;
	keys[0][6] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[0][7] = 2.72000;
	keys[0][7] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.400000, 0.000000));
	times[0][8] = 3.92000;
	keys[0][8] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.400000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[0][9] = 4.20000;
	keys[0][9] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][10] = 4.44000;
	keys[0][10] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[0][11] = 4.72000;
	keys[0][11] = AL::ALValue::array(-0.161112, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("HeadYaw");
	times[1].arraySetSize(12);
	keys[1].arraySetSize(12);

	times[1][0] = 0.720000;
	keys[1][0] = AL::ALValue::array(-0.00617796, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][1] = 0.960000;
	keys[1][1] = AL::ALValue::array(-0.00617796, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][2] = 1.20000;
	keys[1][2] = AL::ALValue::array(-0.00617796, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][3] = 1.44000;
	keys[1][3] = AL::ALValue::array(-0.00617796, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[1][4] = 1.92000;
	keys[1][4] = AL::ALValue::array(-0.00617796, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[1][5] = 2.20000;
	keys[1][5] = AL::ALValue::array(-0.00617796, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][6] = 2.44000;
	keys[1][6] = AL::ALValue::array(-0.00617796, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[1][7] = 2.72000;
	keys[1][7] = AL::ALValue::array(-0.00617796, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.400000, 0.000000));
	times[1][8] = 3.92000;
	keys[1][8] = AL::ALValue::array(0.00617796, AL::ALValue::array(3, -0.400000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[1][9] = 4.20000;
	keys[1][9] = AL::ALValue::array(0.00617796, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][10] = 4.44000;
	keys[1][10] = AL::ALValue::array(0.00617796, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[1][11] = 4.72000;
	keys[1][11] = AL::ALValue::array(0.00617796, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowRoll");
	times[2].arraySetSize(13);
	keys[2].arraySetSize(13);

	times[2][0] = 0.720000;
	keys[2][0] = AL::ALValue::array(-0.446352, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[2][1] = 0.960000;
	keys[2][1] = AL::ALValue::array(-0.417205, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[2][2] = 1.20000;
	keys[2][2] = AL::ALValue::array(-0.446352, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[2][3] = 1.44000;
	keys[2][3] = AL::ALValue::array(-0.417205, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[2][4] = 1.92000;
	keys[2][4] = AL::ALValue::array(-0.679520, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[2][5] = 2.20000;
	keys[2][5] = AL::ALValue::array(-0.664180, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[2][6] = 2.44000;
	keys[2][6] = AL::ALValue::array(-0.679520, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[2][7] = 2.72000;
	keys[2][7] = AL::ALValue::array(-0.664180, AL::ALValue::array(3, -0.0933333, -0.00569438), AL::ALValue::array(3, 0.200000, 0.0122022));
	times[2][8] = 3.32000;
	keys[2][8] = AL::ALValue::array(-0.625830, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[2][9] = 3.92000;
	keys[2][9] = AL::ALValue::array(-0.995607, AL::ALValue::array(3, -0.200000, 0.00328739), AL::ALValue::array(3, 0.0933333, -0.00153411));
	times[2][10] = 4.20000;
	keys[2][10] = AL::ALValue::array(-0.997141, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[2][11] = 4.44000;
	keys[2][11] = AL::ALValue::array(-0.995607, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[2][12] = 4.72000;
	keys[2][12] = AL::ALValue::array(-0.997141, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[3].arraySetSize(13);
	keys[3].arraySetSize(13);

	times[3][0] = 0.720000;
	keys[3][0] = AL::ALValue::array(-1.04470, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][1] = 0.960000;
	keys[3][1] = AL::ALValue::array(-1.06004, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][2] = 1.20000;
	keys[3][2] = AL::ALValue::array(-1.04470, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][3] = 1.44000;
	keys[3][3] = AL::ALValue::array(-1.06004, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[3][4] = 1.92000;
	keys[3][4] = AL::ALValue::array(0.0229680, AL::ALValue::array(3, -0.160000, -0.0262972), AL::ALValue::array(3, 0.0933333, 0.0153400));
	times[3][5] = 2.20000;
	keys[3][5] = AL::ALValue::array(0.0383081, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][6] = 2.44000;
	keys[3][6] = AL::ALValue::array(0.0229680, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[3][7] = 2.72000;
	keys[3][7] = AL::ALValue::array(0.0383081, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[3][8] = 3.32000;
	keys[3][8] = AL::ALValue::array(-0.289967, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[3][9] = 3.92000;
	keys[3][9] = AL::ALValue::array(-0.113474, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[3][10] = 4.20000;
	keys[3][10] = AL::ALValue::array(-0.144154, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][11] = 4.44000;
	keys[3][11] = AL::ALValue::array(-0.113474, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[3][12] = 4.72000;
	keys[3][12] = AL::ALValue::array(-0.144154, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[4].arraySetSize(13);
	keys[4].arraySetSize(13);

	times[4][0] = 0.720000;
	keys[4][0] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][1] = 0.960000;
	keys[4][1] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][2] = 1.20000;
	keys[4][2] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][3] = 1.44000;
	keys[4][3] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[4][4] = 1.92000;
	keys[4][4] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[4][5] = 2.20000;
	keys[4][5] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][6] = 2.44000;
	keys[4][6] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[4][7] = 2.72000;
	keys[4][7] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[4][8] = 3.32000;
	keys[4][8] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[4][9] = 3.92000;
	keys[4][9] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[4][10] = 4.20000;
	keys[4][10] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][11] = 4.44000;
	keys[4][11] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[4][12] = 4.72000;
	keys[4][12] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[5].arraySetSize(13);
	keys[5].arraySetSize(13);

	times[5][0] = 0.720000;
	keys[5][0] = AL::ALValue::array(0.487771, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[5][1] = 0.960000;
	keys[5][1] = AL::ALValue::array(0.561403, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[5][2] = 1.20000;
	keys[5][2] = AL::ALValue::array(0.487771, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[5][3] = 1.44000;
	keys[5][3] = AL::ALValue::array(0.561403, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[5][4] = 1.92000;
	keys[5][4] = AL::ALValue::array(0.329768, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[5][5] = 2.20000;
	keys[5][5] = AL::ALValue::array(0.361981, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[5][6] = 2.44000;
	keys[5][6] = AL::ALValue::array(0.329768, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[5][7] = 2.72000;
	keys[5][7] = AL::ALValue::array(0.361981, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[5][8] = 3.32000;
	keys[5][8] = AL::ALValue::array(0.156426, AL::ALValue::array(3, -0.200000, 0.0562326), AL::ALValue::array(3, 0.200000, -0.0562326));
	times[5][9] = 3.92000;
	keys[5][9] = AL::ALValue::array(0.0245859, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[5][10] = 4.20000;
	keys[5][10] = AL::ALValue::array(0.0798099, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[5][11] = 4.44000;
	keys[5][11] = AL::ALValue::array(0.0245859, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[5][12] = 4.72000;
	keys[5][12] = AL::ALValue::array(0.0798099, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[6].arraySetSize(13);
	keys[6].arraySetSize(13);

	times[6][0] = 0.720000;
	keys[6][0] = AL::ALValue::array(-0.274628, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][1] = 0.960000;
	keys[6][1] = AL::ALValue::array(-0.270025, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][2] = 1.20000;
	keys[6][2] = AL::ALValue::array(-0.274628, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][3] = 1.44000;
	keys[6][3] = AL::ALValue::array(-0.270025, AL::ALValue::array(3, -0.0800000, -0.00460242), AL::ALValue::array(3, 0.160000, 0.00920485));
	times[6][4] = 1.92000;
	keys[6][4] = AL::ALValue::array(-0.0552660, AL::ALValue::array(3, -0.160000, -0.0262972), AL::ALValue::array(3, 0.0933333, 0.0153400));
	times[6][5] = 2.20000;
	keys[6][5] = AL::ALValue::array(-0.0399260, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][6] = 2.44000;
	keys[6][6] = AL::ALValue::array(-0.0552660, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[6][7] = 2.72000;
	keys[6][7] = AL::ALValue::array(-0.0399260, AL::ALValue::array(3, -0.0933333, -0.0122023), AL::ALValue::array(3, 0.200000, 0.0261477));
	times[6][8] = 3.32000;
	keys[6][8] = AL::ALValue::array(0.0597840, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[6][9] = 3.92000;
	keys[6][9] = AL::ALValue::array(4.19617e-05, AL::ALValue::array(3, -0.200000, 0.0564601), AL::ALValue::array(3, 0.0933333, -0.0263480));
	times[6][10] = 4.20000;
	keys[6][10] = AL::ALValue::array(-0.188640, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][11] = 4.44000;
	keys[6][11] = AL::ALValue::array(4.19617e-05, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[6][12] = 4.72000;
	keys[6][12] = AL::ALValue::array(-0.188640, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[7].arraySetSize(13);
	keys[7].arraySetSize(13);

	times[7][0] = 0.720000;
	keys[7][0] = AL::ALValue::array(-0.816130, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][1] = 0.960000;
	keys[7][1] = AL::ALValue::array(-0.0537319, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][2] = 1.20000;
	keys[7][2] = AL::ALValue::array(-0.816130, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][3] = 1.44000;
	keys[7][3] = AL::ALValue::array(-0.0537319, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[7][4] = 1.92000;
	keys[7][4] = AL::ALValue::array(-0.265424, AL::ALValue::array(3, -0.160000, 0.0105169), AL::ALValue::array(3, 0.0933333, -0.00613484));
	times[7][5] = 2.20000;
	keys[7][5] = AL::ALValue::array(-0.271559, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][6] = 2.44000;
	keys[7][6] = AL::ALValue::array(-0.265424, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[7][7] = 2.72000;
	keys[7][7] = AL::ALValue::array(-0.271559, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[7][8] = 3.32000;
	keys[7][8] = AL::ALValue::array(0.0137640, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[7][9] = 3.92000;
	keys[7][9] = AL::ALValue::array(-0.118076, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[7][10] = 4.20000;
	keys[7][10] = AL::ALValue::array(-0.111940, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][11] = 4.44000;
	keys[7][11] = AL::ALValue::array(-0.118076, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[7][12] = 4.72000;
	keys[7][12] = AL::ALValue::array(-0.111940, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[8].arraySetSize(13);
	keys[8].arraySetSize(13);

	times[8][0] = 0.720000;
	keys[8][0] = AL::ALValue::array(0.782382, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[8][1] = 0.960000;
	keys[8][1] = AL::ALValue::array(0.788519, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[8][2] = 1.20000;
	keys[8][2] = AL::ALValue::array(0.782382, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[8][3] = 1.44000;
	keys[8][3] = AL::ALValue::array(0.788519, AL::ALValue::array(3, -0.0800000, -0.00613652), AL::ALValue::array(3, 0.160000, 0.0122730));
	times[8][4] = 1.92000;
	keys[8][4] = AL::ALValue::array(0.995607, AL::ALValue::array(3, -0.160000, -0.00262991), AL::ALValue::array(3, 0.0933333, 0.00153411));
	times[8][5] = 2.20000;
	keys[8][5] = AL::ALValue::array(0.997141, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[8][6] = 2.44000;
	keys[8][6] = AL::ALValue::array(0.995607, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[8][7] = 2.72000;
	keys[8][7] = AL::ALValue::array(0.997141, AL::ALValue::array(3, -0.0933333, -0.00153411), AL::ALValue::array(3, 0.200000, 0.00328739));
	times[8][8] = 3.32000;
	keys[8][8] = AL::ALValue::array(1.16895, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[8][9] = 3.92000;
	keys[8][9] = AL::ALValue::array(0.679520, AL::ALValue::array(3, -0.200000, 0.0328709), AL::ALValue::array(3, 0.0933333, -0.0153397));
	times[8][10] = 4.20000;
	keys[8][10] = AL::ALValue::array(0.664180, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[8][11] = 4.44000;
	keys[8][11] = AL::ALValue::array(0.679520, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[8][12] = 4.72000;
	keys[8][12] = AL::ALValue::array(0.664180, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[9].arraySetSize(13);
	keys[9].arraySetSize(13);

	times[9][0] = 0.720000;
	keys[9][0] = AL::ALValue::array(0.645772, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][1] = 0.960000;
	keys[9][1] = AL::ALValue::array(0.906552, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][2] = 1.20000;
	keys[9][2] = AL::ALValue::array(0.645772, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][3] = 1.44000;
	keys[9][3] = AL::ALValue::array(0.906552, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[9][4] = 1.92000;
	keys[9][4] = AL::ALValue::array(0.113474, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[9][5] = 2.20000;
	keys[9][5] = AL::ALValue::array(0.144154, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][6] = 2.44000;
	keys[9][6] = AL::ALValue::array(0.113474, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[9][7] = 2.72000;
	keys[9][7] = AL::ALValue::array(0.144154, AL::ALValue::array(3, -0.0933333, -0.0306801), AL::ALValue::array(3, 0.200000, 0.0657431));
	times[9][8] = 3.32000;
	keys[9][8] = AL::ALValue::array(0.702530, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[9][9] = 3.92000;
	keys[9][9] = AL::ALValue::array(-0.0229680, AL::ALValue::array(3, -0.200000, 0.0328715), AL::ALValue::array(3, 0.0933333, -0.0153401));
	times[9][10] = 4.20000;
	keys[9][10] = AL::ALValue::array(-0.0383081, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][11] = 4.44000;
	keys[9][11] = AL::ALValue::array(-0.0229680, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[9][12] = 4.72000;
	keys[9][12] = AL::ALValue::array(-0.0383081, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[10].arraySetSize(13);
	keys[10].arraySetSize(13);

	times[10][0] = 0.720000;
	keys[10][0] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][1] = 0.960000;
	keys[10][1] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][2] = 1.20000;
	keys[10][2] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][3] = 1.44000;
	keys[10][3] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[10][4] = 1.92000;
	keys[10][4] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[10][5] = 2.20000;
	keys[10][5] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][6] = 2.44000;
	keys[10][6] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[10][7] = 2.72000;
	keys[10][7] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[10][8] = 3.32000;
	keys[10][8] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[10][9] = 3.92000;
	keys[10][9] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[10][10] = 4.20000;
	keys[10][10] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][11] = 4.44000;
	keys[10][11] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[10][12] = 4.72000;
	keys[10][12] = AL::ALValue::array(0.000000, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[11].arraySetSize(13);
	keys[11].arraySetSize(13);

	times[11][0] = 0.720000;
	keys[11][0] = AL::ALValue::array(0.651992, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][1] = 0.960000;
	keys[11][1] = AL::ALValue::array(0.607505, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][2] = 1.20000;
	keys[11][2] = AL::ALValue::array(0.651992, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][3] = 1.44000;
	keys[11][3] = AL::ALValue::array(0.607505, AL::ALValue::array(3, -0.0800000, 0.0444867), AL::ALValue::array(3, 0.160000, -0.0889734));
	times[11][4] = 1.92000;
	keys[11][4] = AL::ALValue::array(0.0245859, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[11][5] = 2.20000;
	keys[11][5] = AL::ALValue::array(0.0798099, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][6] = 2.44000;
	keys[11][6] = AL::ALValue::array(0.0245859, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[11][7] = 2.72000;
	keys[11][7] = AL::ALValue::array(0.0798099, AL::ALValue::array(3, -0.0933333, -0.0552240), AL::ALValue::array(3, 0.200000, 0.118337));
	times[11][8] = 3.32000;
	keys[11][8] = AL::ALValue::array(0.613642, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[11][9] = 3.92000;
	keys[11][9] = AL::ALValue::array(0.329768, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[11][10] = 4.20000;
	keys[11][10] = AL::ALValue::array(0.361981, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][11] = 4.44000;
	keys[11][11] = AL::ALValue::array(0.329768, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[11][12] = 4.72000;
	keys[11][12] = AL::ALValue::array(0.361981, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[12].arraySetSize(13);
	keys[12].arraySetSize(13);

	times[12][0] = 0.720000;
	keys[12][0] = AL::ALValue::array(0.0720561, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[12][1] = 0.960000;
	keys[12][1] = AL::ALValue::array(0.105804, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[12][2] = 1.20000;
	keys[12][2] = AL::ALValue::array(0.0720561, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[12][3] = 1.44000;
	keys[12][3] = AL::ALValue::array(0.105804, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[12][4] = 1.92000;
	keys[12][4] = AL::ALValue::array(-4.19617e-05, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[12][5] = 2.20000;
	keys[12][5] = AL::ALValue::array(0.188640, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[12][6] = 2.44000;
	keys[12][6] = AL::ALValue::array(-4.19617e-05, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[12][7] = 2.72000;
	keys[12][7] = AL::ALValue::array(0.188640, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[12][8] = 3.32000;
	keys[12][8] = AL::ALValue::array(-0.500126, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[12][9] = 3.92000;
	keys[12][9] = AL::ALValue::array(0.0552660, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[12][10] = 4.20000;
	keys[12][10] = AL::ALValue::array(0.0399260, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[12][11] = 4.44000;
	keys[12][11] = AL::ALValue::array(0.0552660, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[12][12] = 4.72000;
	keys[12][12] = AL::ALValue::array(0.0399260, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[13].arraySetSize(13);
	keys[13].arraySetSize(13);

	times[13][0] = 0.720000;
	keys[13][0] = AL::ALValue::array(0.519984, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[13][1] = 0.960000;
	keys[13][1] = AL::ALValue::array(0.862065, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[13][2] = 1.20000;
	keys[13][2] = AL::ALValue::array(0.519984, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[13][3] = 1.44000;
	keys[13][3] = AL::ALValue::array(0.862065, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[13][4] = 1.92000;
	keys[13][4] = AL::ALValue::array(0.118076, AL::ALValue::array(3, -0.160000, 0.0105190), AL::ALValue::array(3, 0.0933333, -0.00613606));
	times[13][5] = 2.20000;
	keys[13][5] = AL::ALValue::array(0.111940, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[13][6] = 2.44000;
	keys[13][6] = AL::ALValue::array(0.118076, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[13][7] = 2.72000;
	keys[13][7] = AL::ALValue::array(0.111940, AL::ALValue::array(3, -0.0933333, 0.00613606), AL::ALValue::array(3, 0.200000, -0.0131487));
	times[13][8] = 3.32000;
	keys[13][8] = AL::ALValue::array(-0.349794, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[13][9] = 3.92000;
	keys[13][9] = AL::ALValue::array(0.265424, AL::ALValue::array(3, -0.200000, -0.0131461), AL::ALValue::array(3, 0.0933333, 0.00613484));
	times[13][10] = 4.20000;
	keys[13][10] = AL::ALValue::array(0.271559, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[13][11] = 4.44000;
	keys[13][11] = AL::ALValue::array(0.265424, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0933333, 0.000000));
	times[13][12] = 4.72000;
	keys[13][12] = AL::ALValue::array(0.271559, AL::ALValue::array(3, -0.0933333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));


}
void NAO_prompts::setScrubYourHandsMotion2() {
	//don't do any motions since scrub gesture is already so long
}


void NAO_prompts::setGetSomeSoapMotion1() {
	names.reserve(14);
	times.arraySetSize(14);
	keys.arraySetSize(14);

	names.push_back("HeadPitch");
	times[0].arraySetSize(3);
	keys[0].arraySetSize(3);

	times[0][0] = 0.520000;
	keys[0][0] = AL::ALValue::array(-0.158044, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[0][1] = 1.20000;
	keys[0][1] = AL::ALValue::array(-0.158044, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[0][2] = 1.52000;
	keys[0][2] = AL::ALValue::array(-0.158044, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("HeadYaw");
	times[1].arraySetSize(3);
	keys[1].arraySetSize(3);

	times[1][0] = 0.520000;
	keys[1][0] = AL::ALValue::array(-0.00464395, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[1][1] = 1.20000;
	keys[1][1] = AL::ALValue::array(-0.00464395, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[1][2] = 1.52000;
	keys[1][2] = AL::ALValue::array(-0.00464395, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowRoll");
	times[2].arraySetSize(3);
	keys[2].arraySetSize(3);

	times[2][0] = 0.520000;
	keys[2][0] = AL::ALValue::array(-1.15966, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[2][1] = 1.20000;
	keys[2][1] = AL::ALValue::array(-1.14432, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[2][2] = 1.52000;
	keys[2][2] = AL::ALValue::array(-1.15966, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[3].arraySetSize(3);
	keys[3].arraySetSize(3);

	times[3][0] = 0.520000;
	keys[3][0] = AL::ALValue::array(-1.06004, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[3][1] = 1.20000;
	keys[3][1] = AL::ALValue::array(-1.07384, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[3][2] = 1.52000;
	keys[3][2] = AL::ALValue::array(-1.06004, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[4].arraySetSize(3);
	keys[4].arraySetSize(3);

	times[4][0] = 0.520000;
	keys[4][0] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[4][1] = 1.20000;
	keys[4][1] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[4][2] = 1.52000;
	keys[4][2] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[5].arraySetSize(3);
	keys[5].arraySetSize(3);

	times[5][0] = 0.520000;
	keys[5][0] = AL::ALValue::array(1.13819, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[5][1] = 1.20000;
	keys[5][1] = AL::ALValue::array(1.13512, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[5][2] = 1.52000;
	keys[5][2] = AL::ALValue::array(1.13819, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[6].arraySetSize(3);
	keys[6].arraySetSize(3);

	times[6][0] = 0.520000;
	keys[6][0] = AL::ALValue::array(-0.314159, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[6][1] = 1.20000;
	keys[6][1] = AL::ALValue::array(-0.227074, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[6][2] = 1.52000;
	keys[6][2] = AL::ALValue::array(-0.314159, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[7].arraySetSize(3);
	keys[7].arraySetSize(3);

	times[7][0] = 0.520000;
	keys[7][0] = AL::ALValue::array(-1.45121, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[7][1] = 1.20000;
	keys[7][1] = AL::ALValue::array(-1.45274, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[7][2] = 1.52000;
	keys[7][2] = AL::ALValue::array(-1.45121, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[8].arraySetSize(3);
	keys[8].arraySetSize(3);

	times[8][0] = 0.520000;
	keys[8][0] = AL::ALValue::array(1.27786, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[8][1] = 1.20000;
	keys[8][1] = AL::ALValue::array(1.28400, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[8][2] = 1.52000;
	keys[8][2] = AL::ALValue::array(1.27786, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[9].arraySetSize(3);
	keys[9].arraySetSize(3);

	times[9][0] = 0.520000;
	keys[9][0] = AL::ALValue::array(0.638103, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[9][1] = 1.20000;
	keys[9][1] = AL::ALValue::array(0.404934, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[9][2] = 1.52000;
	keys[9][2] = AL::ALValue::array(0.638103, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[10].arraySetSize(3);
	keys[10].arraySetSize(3);

	times[10][0] = 0.520000;
	keys[10][0] = AL::ALValue::array(0.00779115, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[10][1] = 1.20000;
	keys[10][1] = AL::ALValue::array(0.00779813, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[10][2] = 1.52000;
	keys[10][2] = AL::ALValue::array(0.00779115, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[11].arraySetSize(3);
	keys[11].arraySetSize(3);

	times[11][0] = 0.520000;
	keys[11][0] = AL::ALValue::array(0.150374, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[11][1] = 1.20000;
	keys[11][1] = AL::ALValue::array(0.288433, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[11][2] = 1.52000;
	keys[11][2] = AL::ALValue::array(0.150374, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[12].arraySetSize(3);
	keys[12].arraySetSize(3);

	times[12][0] = 0.520000;
	keys[12][0] = AL::ALValue::array(-0.0752080, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[12][1] = 1.20000;
	keys[12][1] = AL::ALValue::array(-0.0598679, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[12][2] = 1.52000;
	keys[12][2] = AL::ALValue::array(-0.0752080, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[13].arraySetSize(3);
	keys[13].arraySetSize(3);

	times[13][0] = 0.520000;
	keys[13][0] = AL::ALValue::array(-0.325251, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.226667, 0.000000));
	times[13][1] = 1.20000;
	keys[13][1] = AL::ALValue::array(-0.305309, AL::ALValue::array(3, -0.226667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[13][2] = 1.52000;
	keys[13][2] = AL::ALValue::array(-0.325251, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));
}
void NAO_prompts::setGetSomeSoapMotion2() {

	names.reserve(8);
	times.arraySetSize(8);
	keys.arraySetSize(8);

	names.push_back("HeadPitch");
	times[0].arraySetSize(2);
	keys[0].arraySetSize(2);

	times[0][0] = 0.800000;
	keys[0][0] = AL::ALValue::array(-0.233209, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[0][1] = 1.52000;
	keys[0][1] = AL::ALValue::array(-0.233209, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("HeadYaw");
	times[1].arraySetSize(2);
	keys[1].arraySetSize(2);

	times[1][0] = 0.800000;
	keys[1][0] = AL::ALValue::array(1.13205, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[1][1] = 1.52000;
	keys[1][1] = AL::ALValue::array(1.13205, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowRoll");
	times[2].arraySetSize(2);
	keys[2].arraySetSize(2);

	times[2][0] = 0.800000;
	keys[2][0] = AL::ALValue::array(-0.260738, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[2][1] = 1.52000;
	keys[2][1] = AL::ALValue::array(-0.260738, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[3].arraySetSize(2);
	keys[3].arraySetSize(2);

	times[3][0] = 0.800000;
	keys[3][0] = AL::ALValue::array(-0.302240, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[3][1] = 1.52000;
	keys[3][1] = AL::ALValue::array(-0.302240, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[4].arraySetSize(2);
	keys[4].arraySetSize(2);

	times[4][0] = 0.800000;
	keys[4][0] = AL::ALValue::array(0.0157219, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[4][1] = 1.52000;
	keys[4][1] = AL::ALValue::array(0.0157219, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[5].arraySetSize(2);
	keys[5].arraySetSize(2);

	times[5][0] = 0.800000;
	keys[5][0] = AL::ALValue::array(-0.622845, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[5][1] = 1.52000;
	keys[5][1] = AL::ALValue::array(-0.622845, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[6].arraySetSize(2);
	keys[6].arraySetSize(2);

	times[6][0] = 0.800000;
	keys[6][0] = AL::ALValue::array(1.11671, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[6][1] = 1.52000;
	keys[6][1] = AL::ALValue::array(1.11671, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[7].arraySetSize(2);
	keys[7].arraySetSize(2);

	times[7][0] = 0.800000;
	keys[7][0] = AL::ALValue::array(0.454021, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[7][1] = 1.52000;
	keys[7][1] = AL::ALValue::array(0.454021, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));


}


void NAO_prompts::setWetYourHandsMotion1() {
	names.reserve(14);
	times.arraySetSize(14);
	keys.arraySetSize(14);

	names.push_back("HeadPitch");
	times[0].arraySetSize(2);
	keys[0].arraySetSize(2);

	times[0][0] = 0.840000;
	keys[0][0] = AL::ALValue::array(-0.159578, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[0][1] = 1.32000;
	keys[0][1] = AL::ALValue::array(-0.159578, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("HeadYaw");
	times[1].arraySetSize(2);
	keys[1].arraySetSize(2);

	times[1][0] = 0.840000;
	keys[1][0] = AL::ALValue::array(-0.00771196, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[1][1] = 1.32000;
	keys[1][1] = AL::ALValue::array(-0.00771196, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowRoll");
	times[2].arraySetSize(2);
	keys[2].arraySetSize(2);

	times[2][0] = 0.840000;
	keys[2][0] = AL::ALValue::array(-0.503110, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[2][1] = 1.32000;
	keys[2][1] = AL::ALValue::array(-0.503110, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[3].arraySetSize(2);
	keys[3].arraySetSize(2);

	times[3][0] = 0.840000;
	keys[3][0] = AL::ALValue::array(-0.475581, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[3][1] = 1.32000;
	keys[3][1] = AL::ALValue::array(-0.475581, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[4].arraySetSize(1);
	keys[4].arraySetSize(1);

	times[4][0] = 0.920000;
	keys[4][0] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.306667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[5].arraySetSize(2);
	keys[5].arraySetSize(2);

	times[5][0] = 0.840000;
	keys[5][0] = AL::ALValue::array(0.237728, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[5][1] = 1.32000;
	keys[5][1] = AL::ALValue::array(0.237728, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[6].arraySetSize(2);
	keys[6].arraySetSize(2);

	times[6][0] = 0.840000;
	keys[6][0] = AL::ALValue::array(-0.116626, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[6][1] = 1.32000;
	keys[6][1] = AL::ALValue::array(-0.116626, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[7].arraySetSize(2);
	keys[7].arraySetSize(2);

	times[7][0] = 0.840000;
	keys[7][0] = AL::ALValue::array(-1.19349, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[7][1] = 1.32000;
	keys[7][1] = AL::ALValue::array(-1.19349, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[8].arraySetSize(2);
	keys[8].arraySetSize(2);

	times[8][0] = 0.840000;
	keys[8][0] = AL::ALValue::array(0.796188, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[8][1] = 1.32000;
	keys[8][1] = AL::ALValue::array(0.796188, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[9].arraySetSize(2);
	keys[9].arraySetSize(2);

	times[9][0] = 0.840000;
	keys[9][0] = AL::ALValue::array(0.589014, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[9][1] = 1.32000;
	keys[9][1] = AL::ALValue::array(0.589014, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[10].arraySetSize(1);
	keys[10].arraySetSize(1);

	times[10][0] = 0.920000;
	keys[10][0] = AL::ALValue::array(0.0174533, AL::ALValue::array(3, -0.306667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[11].arraySetSize(2);
	keys[11].arraySetSize(2);

	times[11][0] = 0.840000;
	keys[11][0] = AL::ALValue::array(0.375872, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[11][1] = 1.32000;
	keys[11][1] = AL::ALValue::array(0.375872, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[12].arraySetSize(2);
	keys[12].arraySetSize(2);

	times[12][0] = 0.840000;
	keys[12][0] = AL::ALValue::array(0.0628521, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[12][1] = 1.32000;
	keys[12][1] = AL::ALValue::array(0.0628521, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[13].arraySetSize(2);
	keys[13].arraySetSize(2);

	times[13][0] = 0.840000;
	keys[13][0] = AL::ALValue::array(0.793036, AL::ALValue::array(3, -0.280000, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[13][1] = 1.32000;
	keys[13][1] = AL::ALValue::array(0.793036, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));
}
void NAO_prompts::setWetYourHandsMotion2() {

	names.reserve(8);
	times.arraySetSize(8);
	keys.arraySetSize(8);

	names.push_back("HeadPitch");
	times[0].arraySetSize(2);
	keys[0].arraySetSize(2);

	times[0][0] = 0.800000;
	keys[0][0] = AL::ALValue::array(0.0106960, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[0][1] = 1.52000;
	keys[0][1] = AL::ALValue::array(0.0106960, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("HeadYaw");
	times[1].arraySetSize(2);
	keys[1].arraySetSize(2);

	times[1][0] = 0.800000;
	keys[1][0] = AL::ALValue::array(0.705598, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[1][1] = 1.52000;
	keys[1][1] = AL::ALValue::array(0.705598, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowRoll");
	times[2].arraySetSize(2);
	keys[2].arraySetSize(2);

	times[2][0] = 0.800000;
	keys[2][0] = AL::ALValue::array(-0.411070, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[2][1] = 1.52000;
	keys[2][1] = AL::ALValue::array(-0.411070, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[3].arraySetSize(2);
	keys[3].arraySetSize(2);

	times[3][0] = 0.800000;
	keys[3][0] = AL::ALValue::array(-2.08567, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[3][1] = 1.52000;
	keys[3][1] = AL::ALValue::array(-2.08567, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[4].arraySetSize(2);
	keys[4].arraySetSize(2);

	times[4][0] = 0.800000;
	keys[4][0] = AL::ALValue::array(0.0152123, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[4][1] = 1.52000;
	keys[4][1] = AL::ALValue::array(0.0152123, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[5].arraySetSize(2);
	keys[5].arraySetSize(2);

	times[5][0] = 0.800000;
	keys[5][0] = AL::ALValue::array(0.743948, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[5][1] = 1.52000;
	keys[5][1] = AL::ALValue::array(0.743948, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[6].arraySetSize(2);
	keys[6].arraySetSize(2);

	times[6][0] = 0.800000;
	keys[6][0] = AL::ALValue::array(0.289883, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[6][1] = 1.52000;
	keys[6][1] = AL::ALValue::array(0.289883, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[7].arraySetSize(2);
	keys[7].arraySetSize(2);

	times[7][0] = 0.800000;
	keys[7][0] = AL::ALValue::array(-0.400415, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[7][1] = 1.52000;
	keys[7][1] = AL::ALValue::array(-0.400415, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));


}

void NAO_prompts::setTurnOnWaterMotion1() {
	names.reserve(12);
	times.arraySetSize(12);
	keys.arraySetSize(12);

	names.push_back("LElbowRoll");
	times[0].arraySetSize(4);
	keys[0].arraySetSize(4);

	times[0][0] = 0.600000;
	keys[0][0] = AL::ALValue::array(-1.12898, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][1] = 0.840000;
	keys[0][1] = AL::ALValue::array(-1.12898, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][2] = 1.08000;
	keys[0][2] = AL::ALValue::array(-1.12898, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[0][3] = 1.32000;
	keys[0][3] = AL::ALValue::array(-1.12898, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[1].arraySetSize(4);
	keys[1].arraySetSize(4);

	times[1][0] = 0.600000;
	keys[1][0] = AL::ALValue::array(-1.19963, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][1] = 0.840000;
	keys[1][1] = AL::ALValue::array(-1.19963, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][2] = 1.08000;
	keys[1][2] = AL::ALValue::array(-1.19963, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[1][3] = 1.32000;
	keys[1][3] = AL::ALValue::array(-1.19963, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[2].arraySetSize(4);
	keys[2].arraySetSize(4);

	times[2][0] = 0.600000;
	keys[2][0] = AL::ALValue::array(0.00609469, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[2][1] = 0.840000;
	keys[2][1] = AL::ALValue::array(0.00609469, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[2][2] = 1.08000;
	keys[2][2] = AL::ALValue::array(0.00609469, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[2][3] = 1.32000;
	keys[2][3] = AL::ALValue::array(0.00609469, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[3].arraySetSize(4);
	keys[3].arraySetSize(4);

	times[3][0] = 0.600000;
	keys[3][0] = AL::ALValue::array(1.24863, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][1] = 0.840000;
	keys[3][1] = AL::ALValue::array(1.24863, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][2] = 1.08000;
	keys[3][2] = AL::ALValue::array(1.24863, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[3][3] = 1.32000;
	keys[3][3] = AL::ALValue::array(1.24863, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[4].arraySetSize(4);
	keys[4].arraySetSize(4);

	times[4][0] = 0.600000;
	keys[4][0] = AL::ALValue::array(-0.00157596, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][1] = 0.840000;
	keys[4][1] = AL::ALValue::array(-0.00310997, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][2] = 1.08000;
	keys[4][2] = AL::ALValue::array(-0.00157596, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[4][3] = 1.32000;
	keys[4][3] = AL::ALValue::array(-0.00310997, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[5].arraySetSize(4);
	keys[5].arraySetSize(4);

	times[5][0] = 0.600000;
	keys[5][0] = AL::ALValue::array(-0.0644700, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[5][1] = 0.840000;
	keys[5][1] = AL::ALValue::array(-0.0644700, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[5][2] = 1.08000;
	keys[5][2] = AL::ALValue::array(-0.0644700, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[5][3] = 1.32000;
	keys[5][3] = AL::ALValue::array(-0.0644700, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[6].arraySetSize(4);
	keys[6].arraySetSize(4);

	times[6][0] = 0.600000;
	keys[6][0] = AL::ALValue::array(0.734827, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][1] = 0.840000;
	keys[6][1] = AL::ALValue::array(0.721022, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][2] = 1.08000;
	keys[6][2] = AL::ALValue::array(0.734827, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[6][3] = 1.32000;
	keys[6][3] = AL::ALValue::array(0.721022, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[7].arraySetSize(4);
	keys[7].arraySetSize(4);

	times[7][0] = 0.600000;
	keys[7][0] = AL::ALValue::array(0.107338, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][1] = 0.840000;
	keys[7][1] = AL::ALValue::array(0.0966000, AL::ALValue::array(3, -0.0800000, 0.00281234), AL::ALValue::array(3, 0.0800000, -0.00281234));
	times[7][2] = 1.08000;
	keys[7][2] = AL::ALValue::array(0.0904641, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[7][3] = 1.32000;
	keys[7][3] = AL::ALValue::array(0.0966000, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[8].arraySetSize(4);
	keys[8].arraySetSize(4);

	times[8][0] = 0.600000;
	keys[8][0] = AL::ALValue::array(0.0174463, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[8][1] = 0.840000;
	keys[8][1] = AL::ALValue::array(0.00401426, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[8][2] = 1.08000;
	keys[8][2] = AL::ALValue::array(0.00508938, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[8][3] = 1.32000;
	keys[8][3] = AL::ALValue::array(0.00401426, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[9].arraySetSize(4);
	keys[9].arraySetSize(4);

	times[9][0] = 0.600000;
	keys[9][0] = AL::ALValue::array(0.191792, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][1] = 0.840000;
	keys[9][1] = AL::ALValue::array(0.141170, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][2] = 1.08000;
	keys[9][2] = AL::ALValue::array(0.202530, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[9][3] = 1.32000;
	keys[9][3] = AL::ALValue::array(0.141170, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[10].arraySetSize(4);
	keys[10].arraySetSize(4);

	times[10][0] = 0.600000;
	keys[10][0] = AL::ALValue::array(-0.216335, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][1] = 0.840000;
	keys[10][1] = AL::ALValue::array(-0.237812, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][2] = 1.08000;
	keys[10][2] = AL::ALValue::array(-0.216335, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[10][3] = 1.32000;
	keys[10][3] = AL::ALValue::array(-0.237812, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[11].arraySetSize(4);
	keys[11].arraySetSize(4);

	times[11][0] = 0.600000;
	keys[11][0] = AL::ALValue::array(0.828318, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][1] = 0.840000;
	keys[11][1] = AL::ALValue::array(-0.220938, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][2] = 1.08000;
	keys[11][2] = AL::ALValue::array(0.839057, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.0800000, 0.000000));
	times[11][3] = 1.32000;
	keys[11][3] = AL::ALValue::array(-0.220938, AL::ALValue::array(3, -0.0800000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));
}

void NAO_prompts::setTurnOnWaterMotion2() {
	names.reserve(8);
	times.arraySetSize(8);
	keys.arraySetSize(8);

	names.push_back("HeadPitch");
	times[0].arraySetSize(2);
	keys[0].arraySetSize(2);

	times[0][0] = 0.800000;
	keys[0][0] = AL::ALValue::array(0.0106960, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[0][1] = 1.52000;
	keys[0][1] = AL::ALValue::array(0.0106960, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("HeadYaw");
	times[1].arraySetSize(2);
	keys[1].arraySetSize(2);

	times[1][0] = 0.800000;
	keys[1][0] = AL::ALValue::array(0.705598, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[1][1] = 1.52000;
	keys[1][1] = AL::ALValue::array(0.705598, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowRoll");
	times[2].arraySetSize(2);
	keys[2].arraySetSize(2);

	times[2][0] = 0.800000;
	keys[2][0] = AL::ALValue::array(-0.734743, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[2][1] = 1.52000;
	keys[2][1] = AL::ALValue::array(-0.734743, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[3].arraySetSize(2);
	keys[3].arraySetSize(2);

	times[3][0] = 0.800000;
	keys[3][0] = AL::ALValue::array(-2.08567, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[3][1] = 1.52000;
	keys[3][1] = AL::ALValue::array(-2.08567, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[4].arraySetSize(2);
	keys[4].arraySetSize(2);

	times[4][0] = 0.800000;
	keys[4][0] = AL::ALValue::array(0.0152542, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[4][1] = 1.52000;
	keys[4][1] = AL::ALValue::array(0.0152542, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[5].arraySetSize(2);
	keys[5].arraySetSize(2);

	times[5][0] = 0.800000;
	keys[5][0] = AL::ALValue::array(1.13052, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[5][1] = 1.52000;
	keys[5][1] = AL::ALValue::array(1.13052, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[6].arraySetSize(2);
	keys[6].arraySetSize(2);

	times[6][0] = 0.800000;
	keys[6][0] = AL::ALValue::array(0.431013, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[6][1] = 1.52000;
	keys[6][1] = AL::ALValue::array(0.431013, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[7].arraySetSize(2);
	keys[7].arraySetSize(2);

	times[7][0] = 0.800000;
	keys[7][0] = AL::ALValue::array(-1.82387, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[7][1] = 1.52000;
	keys[7][1] = AL::ALValue::array(-1.82387, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));
}

void NAO_prompts::setIntroMotion() {

	names.reserve(12);
	times.arraySetSize(12);
	keys.arraySetSize(12);

	names.push_back("LElbowRoll");
	times[0].arraySetSize(3);
	keys[0].arraySetSize(3);

	times[0][0] = 0.800000;
	keys[0][0] = AL::ALValue::array(-0.744032, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[0][1] = 1.40000;
	keys[0][1] = AL::ALValue::array(-0.747100, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.213333, 0.000000));
	times[0][2] = 2.04000;
	keys[0][2] = AL::ALValue::array(-0.747100, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[1].arraySetSize(3);
	keys[1].arraySetSize(3);

	times[1][0] = 0.800000;
	keys[1][0] = AL::ALValue::array(-1.52015, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[1][1] = 1.40000;
	keys[1][1] = AL::ALValue::array(-1.77633, AL::ALValue::array(3, -0.200000, 0.0799155), AL::ALValue::array(3, 0.213333, -0.0852432));
	times[1][2] = 2.04000;
	keys[1][2] = AL::ALValue::array(-2.01563, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[2].arraySetSize(3);
	keys[2].arraySetSize(3);

	times[2][0] = 0.800000;
	keys[2][0] = AL::ALValue::array(0.0149679, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[2][1] = 1.40000;
	keys[2][1] = AL::ALValue::array(0.0149749, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.213333, 0.000000));
	times[2][2] = 2.04000;
	keys[2][2] = AL::ALValue::array(0.0149749, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[3].arraySetSize(3);
	keys[3].arraySetSize(3);

	times[3][0] = 0.800000;
	keys[3][0] = AL::ALValue::array(0.589097, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[3][1] = 1.40000;
	keys[3][1] = AL::ALValue::array(0.943452, AL::ALValue::array(3, -0.200000, -0.00719128), AL::ALValue::array(3, 0.213333, 0.00767070));
	times[3][2] = 2.04000;
	keys[3][2] = AL::ALValue::array(0.951122, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[4].arraySetSize(3);
	keys[4].arraySetSize(3);

	times[4][0] = 0.800000;
	keys[4][0] = AL::ALValue::array(-0.242330, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[4][1] = 1.40000;
	keys[4][1] = AL::ALValue::array(-0.0628521, AL::ALValue::array(3, -0.200000, -0.0873391), AL::ALValue::array(3, 0.213333, 0.0931617));
	times[4][2] = 2.04000;
	keys[4][2] = AL::ALValue::array(0.299172, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[5].arraySetSize(3);
	keys[5].arraySetSize(3);

	times[5][0] = 0.800000;
	keys[5][0] = AL::ALValue::array(-1.15046, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[5][1] = 1.40000;
	keys[5][1] = AL::ALValue::array(-1.41124, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.213333, 0.000000));
	times[5][2] = 2.04000;
	keys[5][2] = AL::ALValue::array(-1.40970, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[6].arraySetSize(3);
	keys[6].arraySetSize(3);

	times[6][0] = 0.800000;
	keys[6][0] = AL::ALValue::array(0.744032, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[6][1] = 1.40000;
	keys[6][1] = AL::ALValue::array(0.747100, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.213333, 0.000000));
	times[6][2] = 2.04000;
	keys[6][2] = AL::ALValue::array(0.747100, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[7].arraySetSize(3);
	keys[7].arraySetSize(3);

	times[7][0] = 0.800000;
	keys[7][0] = AL::ALValue::array(1.52015, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[7][1] = 1.40000;
	keys[7][1] = AL::ALValue::array(1.77633, AL::ALValue::array(3, -0.200000, -0.0799155), AL::ALValue::array(3, 0.213333, 0.0852432));
	times[7][2] = 2.04000;
	keys[7][2] = AL::ALValue::array(2.01563, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[8].arraySetSize(3);
	keys[8].arraySetSize(3);

	times[8][0] = 0.800000;
	keys[8][0] = AL::ALValue::array(0.0149679, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[8][1] = 1.40000;
	keys[8][1] = AL::ALValue::array(0.0149749, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.213333, 0.000000));
	times[8][2] = 2.04000;
	keys[8][2] = AL::ALValue::array(0.0149749, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[9].arraySetSize(3);
	keys[9].arraySetSize(3);

	times[9][0] = 0.800000;
	keys[9][0] = AL::ALValue::array(0.589097, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[9][1] = 1.40000;
	keys[9][1] = AL::ALValue::array(0.943452, AL::ALValue::array(3, -0.200000, -0.00719128), AL::ALValue::array(3, 0.213333, 0.00767070));
	times[9][2] = 2.04000;
	keys[9][2] = AL::ALValue::array(0.951122, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[10].arraySetSize(3);
	keys[10].arraySetSize(3);

	times[10][0] = 0.800000;
	keys[10][0] = AL::ALValue::array(0.242330, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[10][1] = 1.40000;
	keys[10][1] = AL::ALValue::array(0.0628521, AL::ALValue::array(3, -0.200000, 0.0873391), AL::ALValue::array(3, 0.213333, -0.0931617));
	times[10][2] = 2.04000;
	keys[10][2] = AL::ALValue::array(-0.299172, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[11].arraySetSize(3);
	keys[11].arraySetSize(3);

	times[11][0] = 0.800000;
	keys[11][0] = AL::ALValue::array(1.15046, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.200000, 0.000000));
	times[11][1] = 1.40000;
	keys[11][1] = AL::ALValue::array(1.41124, AL::ALValue::array(3, -0.200000, -0.000000), AL::ALValue::array(3, 0.213333, 0.000000));
	times[11][2] = 2.04000;
	keys[11][2] = AL::ALValue::array(1.40970, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

}

void NAO_prompts::setAllDoneMotion() {
	names.reserve(12);
	times.arraySetSize(12);
	keys.arraySetSize(12);

	names.push_back("LElbowRoll");
	times[0].arraySetSize(4);
	keys[0].arraySetSize(4);

	times[0][0] = 0.640000;
	keys[0][0] = AL::ALValue::array(-1.31315, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.333333, 0.000000));
	times[0][1] = 1.64000;
	keys[0][1] = AL::ALValue::array(-0.625914, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[0][2] = 2.12000;
	keys[0][2] = AL::ALValue::array(-1.02629, AL::ALValue::array(3, -0.160000, 0.0979843), AL::ALValue::array(3, 0.266667, -0.163307));
	times[0][3] = 2.92000;
	keys[0][3] = AL::ALValue::array(-1.40979, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[1].arraySetSize(4);
	keys[1].arraySetSize(4);

	times[1][0] = 0.640000;
	keys[1][0] = AL::ALValue::array(0.512397, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.333333, 0.000000));
	times[1][1] = 1.64000;
	keys[1][1] = AL::ALValue::array(-0.843657, AL::ALValue::array(3, -0.333333, 0.460891), AL::ALValue::array(3, 0.160000, -0.221228));
	times[1][2] = 2.12000;
	keys[1][2] = AL::ALValue::array(-1.53396, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.266667, 0.000000));
	times[1][3] = 2.92000;
	keys[1][3] = AL::ALValue::array(-1.25784, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[2].arraySetSize(3);
	keys[2].arraySetSize(3);

	times[2][0] = 1.04000;
	keys[2][0] = AL::ALValue::array(0.0136521, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.360000, 0.000000));
	times[2][1] = 2.12000;
	keys[2][1] = AL::ALValue::array(0.0130999, AL::ALValue::array(3, -0.360000, -0.000000), AL::ALValue::array(3, 0.266667, 0.000000));
	times[2][2] = 2.92000;
	keys[2][2] = AL::ALValue::array(0.0130999, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[3].arraySetSize(4);
	keys[3].arraySetSize(4);

	times[3][0] = 0.640000;
	keys[3][0] = AL::ALValue::array(0.234743, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.333333, 0.000000));
	times[3][1] = 1.64000;
	keys[3][1] = AL::ALValue::array(-0.920358, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[3][2] = 2.12000;
	keys[3][2] = AL::ALValue::array(-0.348176, AL::ALValue::array(3, -0.160000, -0.252535), AL::ALValue::array(3, 0.266667, 0.420891));
	times[3][3] = 2.92000;
	keys[3][3] = AL::ALValue::array(1.09992, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[4].arraySetSize(4);
	keys[4].arraySetSize(4);

	times[4][0] = 0.640000;
	keys[4][0] = AL::ALValue::array(0.418823, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.333333, 0.000000));
	times[4][1] = 1.64000;
	keys[4][1] = AL::ALValue::array(0.721022, AL::ALValue::array(3, -0.333333, -0.00639228), AL::ALValue::array(3, 0.160000, 0.00306829));
	times[4][2] = 2.12000;
	keys[4][2] = AL::ALValue::array(0.724090, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.266667, 0.000000));
	times[4][3] = 2.92000;
	keys[4][3] = AL::ALValue::array(0.409620, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[5].arraySetSize(3);
	keys[5].arraySetSize(3);

	times[5][0] = 1.04000;
	keys[5][0] = AL::ALValue::array(-0.411070, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.360000, 0.000000));
	times[5][1] = 2.12000;
	keys[5][1] = AL::ALValue::array(-0.420274, AL::ALValue::array(3, -0.360000, -0.000000), AL::ALValue::array(3, 0.266667, 0.000000));
	times[5][2] = 2.92000;
	keys[5][2] = AL::ALValue::array(-0.414139, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[6].arraySetSize(4);
	keys[6].arraySetSize(4);

	times[6][0] = 0.640000;
	keys[6][0] = AL::ALValue::array(1.31315, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.333333, 0.000000));
	times[6][1] = 1.64000;
	keys[6][1] = AL::ALValue::array(0.625914, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[6][2] = 2.12000;
	keys[6][2] = AL::ALValue::array(1.02629, AL::ALValue::array(3, -0.160000, -0.0979843), AL::ALValue::array(3, 0.266667, 0.163307));
	times[6][3] = 2.92000;
	keys[6][3] = AL::ALValue::array(1.40979, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[7].arraySetSize(4);
	keys[7].arraySetSize(4);

	times[7][0] = 0.640000;
	keys[7][0] = AL::ALValue::array(-0.512397, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.333333, 0.000000));
	times[7][1] = 1.64000;
	keys[7][1] = AL::ALValue::array(0.843657, AL::ALValue::array(3, -0.333333, -0.460891), AL::ALValue::array(3, 0.160000, 0.221228));
	times[7][2] = 2.12000;
	keys[7][2] = AL::ALValue::array(1.53396, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.266667, 0.000000));
	times[7][3] = 2.92000;
	keys[7][3] = AL::ALValue::array(1.25784, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[8].arraySetSize(3);
	keys[8].arraySetSize(3);

	times[8][0] = 1.04000;
	keys[8][0] = AL::ALValue::array(0.0136521, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.360000, 0.000000));
	times[8][1] = 2.12000;
	keys[8][1] = AL::ALValue::array(0.0130999, AL::ALValue::array(3, -0.360000, -0.000000), AL::ALValue::array(3, 0.266667, 0.000000));
	times[8][2] = 2.92000;
	keys[8][2] = AL::ALValue::array(0.0130999, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[9].arraySetSize(4);
	keys[9].arraySetSize(4);

	times[9][0] = 0.640000;
	keys[9][0] = AL::ALValue::array(0.234743, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.333333, 0.000000));
	times[9][1] = 1.64000;
	keys[9][1] = AL::ALValue::array(-0.920358, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.160000, 0.000000));
	times[9][2] = 2.12000;
	keys[9][2] = AL::ALValue::array(-0.348176, AL::ALValue::array(3, -0.160000, -0.252535), AL::ALValue::array(3, 0.266667, 0.420891));
	times[9][3] = 2.92000;
	keys[9][3] = AL::ALValue::array(1.09992, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[10].arraySetSize(4);
	keys[10].arraySetSize(4);

	times[10][0] = 0.640000;
	keys[10][0] = AL::ALValue::array(-0.418823, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.333333, 0.000000));
	times[10][1] = 1.64000;
	keys[10][1] = AL::ALValue::array(-0.721022, AL::ALValue::array(3, -0.333333, 0.00639228), AL::ALValue::array(3, 0.160000, -0.00306829));
	times[10][2] = 2.12000;
	keys[10][2] = AL::ALValue::array(-0.724090, AL::ALValue::array(3, -0.160000, -0.000000), AL::ALValue::array(3, 0.266667, 0.000000));
	times[10][3] = 2.92000;
	keys[10][3] = AL::ALValue::array(-0.409620, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[11].arraySetSize(3);
	keys[11].arraySetSize(3);

	times[11][0] = 1.04000;
	keys[11][0] = AL::ALValue::array(0.411070, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.360000, 0.000000));
	times[11][1] = 2.12000;
	keys[11][1] = AL::ALValue::array(0.420274, AL::ALValue::array(3, -0.360000, -0.000000), AL::ALValue::array(3, 0.266667, 0.000000));
	times[11][2] = 2.92000;
	keys[11][2] = AL::ALValue::array(0.414139, AL::ALValue::array(3, -0.266667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));
}
