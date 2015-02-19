#include "stdafx.h"
#include "AG_NAO.h"

void AG_NAO::executeAG() {
	setWaveMotion();
	try
	{
		motion.stiffnessInterpolation("Body", 1.0f, 1.0f);

		tts.post.say("Hi David");
		motion.angleInterpolationBezier(names, times, keys);
		tts.stopAll();
	} 
	catch (const std::exception&)
	{
	}
}


void AG_NAO::executePrompt() {
	setPromptMotion();
	try
	{
		motion.stiffnessInterpolation("Body", 1.0f, 1.0f);

		tts.post.say("Please look at the computer");
		motion.angleInterpolationBezier(names, times, keys);
		tts.stopAll();
	}
	catch (const std::exception&)
	{
	}
}


void AG_NAO::executeReward() {
	setRewardMotion();
	try
	{
		motion.stiffnessInterpolation("Body", 1.0f, 1.0f);

		tts.post.say("Yay, good job, David!");
		motion.angleInterpolationBezier(names, times, keys);
		tts.stopAll();

		posture.applyPosture("Crouch", 0.1f);
		motion.stiffnessInterpolation("Body", 0.0f, 1.0f);
	}
	catch (const std::exception&)
	{
	}
}
void AG_NAO::setWaveMotion() {

	names.clear();
	times.clear();
	keys.clear();

	names.reserve(12);
	times.arraySetSize(12);
	keys.arraySetSize(12);

	names.push_back("LElbowRoll");
	times[0].arraySetSize(4);
	keys[0].arraySetSize(4);

	times[0][0] = 0.520000;
	keys[0][0] = AL::ALValue::array(-1.37902, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.186667, 0.000000));
	times[0][1] = 1.08000;
	keys[0][1] = AL::ALValue::array(-1.29005, AL::ALValue::array(3, -0.186667, -0.0286348), AL::ALValue::array(3, 0.240000, 0.0368162));
	times[0][2] = 1.80000;
	keys[0][2] = AL::ALValue::array(-1.18267, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.320000, 0.000000));
	times[0][3] = 2.76000;
	keys[0][3] = AL::ALValue::array(-1.18421, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[1].arraySetSize(4);
	keys[1].arraySetSize(4);

	times[1][0] = 0.520000;
	keys[1][0] = AL::ALValue::array(-0.803859, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.186667, 0.000000));
	times[1][1] = 1.08000;
	keys[1][1] = AL::ALValue::array(-0.691876, AL::ALValue::array(3, -0.186667, -0.00954581), AL::ALValue::array(3, 0.240000, 0.0122732));
	times[1][2] = 1.80000;
	keys[1][2] = AL::ALValue::array(-0.679603, AL::ALValue::array(3, -0.240000, -0.00306805), AL::ALValue::array(3, 0.320000, 0.00409073));
	times[1][3] = 2.76000;
	keys[1][3] = AL::ALValue::array(-0.670400, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[2].arraySetSize(2);
	keys[2].arraySetSize(2);

	times[2][0] = 1.08000;
	keys[2][0] = AL::ALValue::array(0.00415750, AL::ALValue::array(3, -0.360000, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[2][1] = 2.76000;
	keys[2][1] = AL::ALValue::array(0.00418923, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[3].arraySetSize(4);
	keys[3].arraySetSize(4);

	times[3][0] = 0.520000;
	keys[3][0] = AL::ALValue::array(1.11824, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.186667, 0.000000));
	times[3][1] = 1.08000;
	keys[3][1] = AL::ALValue::array(0.928028, AL::ALValue::array(3, -0.186667, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[3][2] = 1.80000;
	keys[3][2] = AL::ALValue::array(0.940300, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.320000, 0.000000));
	times[3][3] = 2.76000;
	keys[3][3] = AL::ALValue::array(0.842125, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[4].arraySetSize(4);
	keys[4].arraySetSize(4);

	times[4][0] = 0.520000;
	keys[4][0] = AL::ALValue::array(0.363515, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.186667, 0.000000));
	times[4][1] = 1.08000;
	keys[4][1] = AL::ALValue::array(0.226991, AL::ALValue::array(3, -0.186667, 0.0178970), AL::ALValue::array(3, 0.240000, -0.0230104));
	times[4][2] = 1.80000;
	keys[4][2] = AL::ALValue::array(0.203980, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.320000, 0.000000));
	times[4][3] = 2.76000;
	keys[4][3] = AL::ALValue::array(0.226991, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[5].arraySetSize(2);
	keys[5].arraySetSize(2);

	times[5][0] = 1.08000;
	keys[5][0] = AL::ALValue::array(0.147222, AL::ALValue::array(3, -0.360000, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[5][1] = 2.76000;
	keys[5][1] = AL::ALValue::array(0.119610, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[6].arraySetSize(5);
	keys[6].arraySetSize(5);

	times[6][0] = 0.960000;
	keys[6][0] = AL::ALValue::array(0.242414, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[6][1] = 1.28000;
	keys[6][1] = AL::ALValue::array(0.349066, AL::ALValue::array(3, -0.106667, -0.0970996), AL::ALValue::array(3, 0.146667, 0.133512));
	times[6][2] = 1.72000;
	keys[6][2] = AL::ALValue::array(0.934249, AL::ALValue::array(3, -0.146667, -0.000000), AL::ALValue::array(3, 0.106667, 0.000000));
	times[6][3] = 2.04000;
	keys[6][3] = AL::ALValue::array(0.680678, AL::ALValue::array(3, -0.106667, -0.000000), AL::ALValue::array(3, 0.213333, 0.000000));
	times[6][4] = 2.68000;
	keys[6][4] = AL::ALValue::array(1.26559, AL::ALValue::array(3, -0.213333, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[7].arraySetSize(3);
	keys[7].arraySetSize(3);

	times[7][0] = 1.00000;
	keys[7][0] = AL::ALValue::array(0.564471, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[7][1] = 1.72000;
	keys[7][1] = AL::ALValue::array(0.391128, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.320000, 0.000000));
	times[7][2] = 2.68000;
	keys[7][2] = AL::ALValue::array(0.826783, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[8].arraySetSize(2);
	keys[8].arraySetSize(2);

	times[8][0] = 1.00000;
	keys[8][0] = AL::ALValue::array(0.0148960, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[8][1] = 2.68000;
	keys[8][1] = AL::ALValue::array(0.00741967, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[9].arraySetSize(3);
	keys[9].arraySetSize(3);

	times[9][0] = 1.00000;
	keys[9][0] = AL::ALValue::array(-1.17193, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[9][1] = 1.72000;
	keys[9][1] = AL::ALValue::array(-1.08910, AL::ALValue::array(3, -0.240000, -0.0828368), AL::ALValue::array(3, 0.320000, 0.110449));
	times[9][2] = 2.68000;
	keys[9][2] = AL::ALValue::array(1.02015, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[10].arraySetSize(3);
	keys[10].arraySetSize(3);

	times[10][0] = 1.00000;
	keys[10][0] = AL::ALValue::array(-0.954191, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.240000, 0.000000));
	times[10][1] = 1.72000;
	keys[10][1] = AL::ALValue::array(-0.460242, AL::ALValue::array(3, -0.240000, -0.100587), AL::ALValue::array(3, 0.320000, 0.134115));
	times[10][2] = 2.68000;
	keys[10][2] = AL::ALValue::array(-0.250085, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[11].arraySetSize(2);
	keys[11].arraySetSize(2);

	times[11][0] = 1.00000;
	keys[11][0] = AL::ALValue::array(-0.312978, AL::ALValue::array(3, -0.333333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[11][1] = 2.68000;
	keys[11][1] = AL::ALValue::array(0.182504, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

}

void AG_NAO::setPromptMotion() {

	names.clear();
	times.clear();
	keys.clear();

	names.reserve(14);
	times.arraySetSize(14);
	keys.arraySetSize(14);

	names.push_back("HeadPitch");
	times[0].arraySetSize(3);
	keys[0].arraySetSize(3);

	times[0][0] = 0.960000;
	keys[0][0] = AL::ALValue::array(-0.0429940, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[0][1] = 2.68000;
	keys[0][1] = AL::ALValue::array(-0.0429939, AL::ALValue::array(3, -0.573333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[0][2] = 4.36000;
	keys[0][2] = AL::ALValue::array(-0.151908, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("HeadYaw");
	times[1].arraySetSize(3);
	keys[1].arraySetSize(3);

	times[1][0] = 0.960000;
	keys[1][0] = AL::ALValue::array(-0.690342, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[1][1] = 2.68000;
	keys[1][1] = AL::ALValue::array(-0.690342, AL::ALValue::array(3, -0.573333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[1][2] = 4.36000;
	keys[1][2] = AL::ALValue::array(-0.0475960, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowRoll");
	times[2].arraySetSize(3);
	keys[2].arraySetSize(3);

	times[2][0] = 0.960000;
	keys[2][0] = AL::ALValue::array(-1.32227, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[2][1] = 2.68000;
	keys[2][1] = AL::ALValue::array(-1.32227, AL::ALValue::array(3, -0.573333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[2][2] = 4.36000;
	keys[2][2] = AL::ALValue::array(-1.32227, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[3].arraySetSize(3);
	keys[3].arraySetSize(3);

	times[3][0] = 0.960000;
	keys[3][0] = AL::ALValue::array(-0.714886, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[3][1] = 2.68000;
	keys[3][1] = AL::ALValue::array(-0.714885, AL::ALValue::array(3, -0.573333, -8.65527e-07), AL::ALValue::array(3, 0.560000, 8.45399e-07));
	times[3][2] = 4.36000;
	keys[3][2] = AL::ALValue::array(-0.708750, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[4].arraySetSize(3);
	keys[4].arraySetSize(3);

	times[4][0] = 0.960000;
	keys[4][0] = AL::ALValue::array(0.00529882, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[4][1] = 2.68000;
	keys[4][1] = AL::ALValue::array(0.00529882, AL::ALValue::array(3, -0.573333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[4][2] = 4.36000;
	keys[4][2] = AL::ALValue::array(0.00529882, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[5].arraySetSize(3);
	keys[5].arraySetSize(3);

	times[5][0] = 0.960000;
	keys[5][0] = AL::ALValue::array(1.36062, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[5][1] = 2.68000;
	keys[5][1] = AL::ALValue::array(1.36062, AL::ALValue::array(3, -0.573333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[5][2] = 4.36000;
	keys[5][2] = AL::ALValue::array(1.35448, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[6].arraySetSize(3);
	keys[6].arraySetSize(3);

	times[6][0] = 0.960000;
	keys[6][0] = AL::ALValue::array(0.0843280, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[6][1] = 2.68000;
	keys[6][1] = AL::ALValue::array(0.0843280, AL::ALValue::array(3, -0.573333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[6][2] = 4.36000;
	keys[6][2] = AL::ALValue::array(0.0843280, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[7].arraySetSize(3);
	keys[7].arraySetSize(3);

	times[7][0] = 0.960000;
	keys[7][0] = AL::ALValue::array(-0.360532, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[7][1] = 2.68000;
	keys[7][1] = AL::ALValue::array(-0.360533, AL::ALValue::array(3, -0.573333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[7][2] = 4.36000;
	keys[7][2] = AL::ALValue::array(-0.360532, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[8].arraySetSize(3);
	keys[8].arraySetSize(3);

	times[8][0] = 0.960000;
	keys[8][0] = AL::ALValue::array(0.309910, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[8][1] = 2.68000;
	keys[8][1] = AL::ALValue::array(0.309909, AL::ALValue::array(3, -0.573333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[8][2] = 4.36000;
	keys[8][2] = AL::ALValue::array(1.30087, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[9].arraySetSize(3);
	keys[9].arraySetSize(3);

	times[9][0] = 0.960000;
	keys[9][0] = AL::ALValue::array(2.08567, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[9][1] = 2.68000;
	keys[9][1] = AL::ALValue::array(2.08567, AL::ALValue::array(3, -0.573333, 1.33158e-07), AL::ALValue::array(3, 0.560000, -1.30061e-07));
	times[9][2] = 4.36000;
	keys[9][2] = AL::ALValue::array(0.664180, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[10].arraySetSize(3);
	keys[10].arraySetSize(3);

	times[10][0] = 0.960000;
	keys[10][0] = AL::ALValue::array(0.0149330, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[10][1] = 2.68000;
	keys[10][1] = AL::ALValue::array(0.0149330, AL::ALValue::array(3, -0.573333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[10][2] = 4.36000;
	keys[10][2] = AL::ALValue::array(0.00545241, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[11].arraySetSize(3);
	keys[11].arraySetSize(3);

	times[11][0] = 0.960000;
	keys[11][0] = AL::ALValue::array(0.148840, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[11][1] = 2.68000;
	keys[11][1] = AL::ALValue::array(0.148840, AL::ALValue::array(3, -0.573333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[11][2] = 4.36000;
	keys[11][2] = AL::ALValue::array(1.34996, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[12].arraySetSize(3);
	keys[12].arraySetSize(3);

	times[12][0] = 0.960000;
	keys[12][0] = AL::ALValue::array(-0.655060, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[12][1] = 2.68000;
	keys[12][1] = AL::ALValue::array(-0.655060, AL::ALValue::array(3, -0.573333, -0.000000), AL::ALValue::array(3, 0.560000, 0.000000));
	times[12][2] = 4.36000;
	keys[12][2] = AL::ALValue::array(-0.110490, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[13].arraySetSize(3);
	keys[13].arraySetSize(3);

	times[13][0] = 0.960000;
	keys[13][0] = AL::ALValue::array(-0.0184500, AL::ALValue::array(3, -0.320000, -0.000000), AL::ALValue::array(3, 0.573333, 0.000000));
	times[13][1] = 2.68000;
	keys[13][1] = AL::ALValue::array(-0.0184499, AL::ALValue::array(3, -0.573333, -8.53044e-08), AL::ALValue::array(3, 0.560000, 8.33206e-08));
	times[13][2] = 4.36000;
	keys[13][2] = AL::ALValue::array(0.450954, AL::ALValue::array(3, -0.560000, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));


}

void AG_NAO::setRewardMotion() {

	names.clear();
	times.clear();
	keys.clear();

	names.reserve(14);
	times.arraySetSize(14);
	keys.arraySetSize(14);

	names.push_back("HeadPitch");
	times[0].arraySetSize(3);
	keys[0].arraySetSize(3);

	times[0][0] = 0.720000;
	keys[0][0] = AL::ALValue::array(-0.151908, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[0][1] = 1.24000;
	keys[0][1] = AL::ALValue::array(-0.151908, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[0][2] = 2.28000;
	keys[0][2] = AL::ALValue::array(-0.150374, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("HeadYaw");
	times[1].arraySetSize(3);
	keys[1].arraySetSize(3);

	times[1][0] = 0.720000;
	keys[1][0] = AL::ALValue::array(-0.0460620, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[1][1] = 1.24000;
	keys[1][1] = AL::ALValue::array(-0.0460620, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[1][2] = 2.28000;
	keys[1][2] = AL::ALValue::array(-0.0445279, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowRoll");
	times[2].arraySetSize(3);
	keys[2].arraySetSize(3);

	times[2][0] = 0.720000;
	keys[2][0] = AL::ALValue::array(-1.12591, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[2][1] = 1.24000;
	keys[2][1] = AL::ALValue::array(-1.12591, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[2][2] = 2.28000;
	keys[2][2] = AL::ALValue::array(-1.27931, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LElbowYaw");
	times[3].arraySetSize(3);
	keys[3].arraySetSize(3);

	times[3][0] = 0.720000;
	keys[3][0] = AL::ALValue::array(-1.46348, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[3][1] = 1.24000;
	keys[3][1] = AL::ALValue::array(-1.46348, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[3][2] = 2.28000;
	keys[3][2] = AL::ALValue::array(-0.698011, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LHand");
	times[4].arraySetSize(3);
	keys[4].arraySetSize(3);

	times[4][0] = 0.720000;
	keys[4][0] = AL::ALValue::array(0.00529882, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[4][1] = 1.24000;
	keys[4][1] = AL::ALValue::array(0.00529882, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[4][2] = 2.28000;
	keys[4][2] = AL::ALValue::array(0.00529882, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderPitch");
	times[5].arraySetSize(3);
	keys[5].arraySetSize(3);

	times[5][0] = 0.720000;
	keys[5][0] = AL::ALValue::array(-0.728692, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[5][1] = 1.24000;
	keys[5][1] = AL::ALValue::array(-0.728692, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[5][2] = 2.28000;
	keys[5][2] = AL::ALValue::array(1.35141, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LShoulderRoll");
	times[6].arraySetSize(3);
	keys[6].arraySetSize(3);

	times[6][0] = 0.720000;
	keys[6][0] = AL::ALValue::array(0.205514, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[6][1] = 1.24000;
	keys[6][1] = AL::ALValue::array(0.205514, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[6][2] = 2.28000;
	keys[6][2] = AL::ALValue::array(0.0889301, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("LWristYaw");
	times[7].arraySetSize(3);
	keys[7].arraySetSize(3);

	times[7][0] = 0.720000;
	keys[7][0] = AL::ALValue::array(-0.897433, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[7][1] = 1.24000;
	keys[7][1] = AL::ALValue::array(-0.897433, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[7][2] = 2.28000;
	keys[7][2] = AL::ALValue::array(-0.363599, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowRoll");
	times[8].arraySetSize(3);
	keys[8].arraySetSize(3);

	times[8][0] = 0.720000;
	keys[8][0] = AL::ALValue::array(1.30854, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[8][1] = 1.24000;
	keys[8][1] = AL::ALValue::array(1.31008, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[8][2] = 2.28000;
	keys[8][2] = AL::ALValue::array(1.22571, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RElbowYaw");
	times[9].arraySetSize(3);
	keys[9].arraySetSize(3);

	times[9][0] = 0.720000;
	keys[9][0] = AL::ALValue::array(1.56157, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[9][1] = 1.24000;
	keys[9][1] = AL::ALValue::array(1.56157, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[9][2] = 2.28000;
	keys[9][2] = AL::ALValue::array(0.644238, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RHand");
	times[10].arraySetSize(3);
	keys[10].arraySetSize(3);

	times[10][0] = 0.720000;
	keys[10][0] = AL::ALValue::array(0.00543845, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[10][1] = 1.24000;
	keys[10][1] = AL::ALValue::array(0.00543845, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[10][2] = 2.28000;
	keys[10][2] = AL::ALValue::array(0.00543845, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderPitch");
	times[11].arraySetSize(3);
	keys[11].arraySetSize(3);

	times[11][0] = 0.720000;
	keys[11][0] = AL::ALValue::array(-0.653443, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[11][1] = 1.24000;
	keys[11][1] = AL::ALValue::array(-0.653443, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[11][2] = 2.28000;
	keys[11][2] = AL::ALValue::array(1.27940, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RShoulderRoll");
	times[12].arraySetSize(3);
	keys[12].arraySetSize(3);

	times[12][0] = 0.720000;
	keys[12][0] = AL::ALValue::array(-0.112024, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[12][1] = 1.24000;
	keys[12][1] = AL::ALValue::array(-0.112024, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[12][2] = 2.28000;
	keys[12][2] = AL::ALValue::array(-0.112024, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	names.push_back("RWristYaw");
	times[13].arraySetSize(3);
	keys[13].arraySetSize(3);

	times[13][0] = 0.720000;
	keys[13][0] = AL::ALValue::array(0.849794, AL::ALValue::array(3, -0.240000, -0.000000), AL::ALValue::array(3, 0.173333, 0.000000));
	times[13][1] = 1.24000;
	keys[13][1] = AL::ALValue::array(0.849794, AL::ALValue::array(3, -0.173333, -0.000000), AL::ALValue::array(3, 0.346667, 0.000000));
	times[13][2] = 2.28000;
	keys[13][2] = AL::ALValue::array(0.447886, AL::ALValue::array(3, -0.346667, -0.000000), AL::ALValue::array(3, 0.000000, 0.000000));

	
}