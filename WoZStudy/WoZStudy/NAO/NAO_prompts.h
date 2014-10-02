#pragma once
#include "NAO_motion.h"
#include "../COACH/Prompts.h"

#include "../COACH/defines.h"

class NAO_prompts :
	public NAO_motion,
	public Prompts
{
public:
	NAO_prompts();
	~NAO_prompts();

	void takeRest();
	void emergencyStop();
	void immediatePause(bool takeRest);

private:
	int led_id;
	int motion_id;
	int tts_id;
	std::vector<float> head_angles;

	bool m_IsLoose;
	bool m_Stopped;
	bool m_TakeRest;//a flag workaround for communicating takeRest command to static thread

	DWORD WINAPI PromptingThread();
	int waitForMotion();//return -1 if exit thread, else return 0 for waited till motion finished
	void takeRest1();

	void stiffen(bool on);
	void convertHandValues();
	void convertHandValuesSimplified();

	void setRestPosture();

	void attentionGrabber();
	void setWaveMotion();

	void reward();
	void setHoorayMotion();

	void intro();
	void setIntroMotion();

	void letUsContinue();

	void turnOnWater();
	void setTurnOnWaterMotion1();
	void setTurnOnWaterMotion2();

	void wetYourHands();
	void setWetYourHandsMotion1();
	void setWetYourHandsMotion2();

	void getSomeSoap();
	void setGetSomeSoapMotion1();
	void setGetSomeSoapMotion2();

	void scrubYourHands();
	void setScrubYourHandsMotion1();
	void setScrubYourHandsMotion2();

	void rinseYourHands();
	void setRinseYourHandsMotion1();
	void setRinseYourHandsMotion2();

	void turnOffWater();
	void setTurnOffWaterMotion1();
	void setTurnOffWaterMotion2();

	void dryYourHands();
	void setDryYourHandsMotion1();
	void setDryYourHandsMotion2();

	void allDone();
	void setAllDoneMotion();
};

