#pragma once
#include <string>
#include <iostream>
#include <fstream>


using namespace std;

class CActionModule
{
public:
	CActionModule(void);
	~CActionModule(void);

public:
	void OnFileStartPrompting();
	//data for the action module
	float cutoffs[6];
	int startingPromptLevel[5];
	int numPromptsIssuedForEachStep[5];
	float USRForStep[5];//user success rates
	float averageNumberOfPromptsNeededForEachStep[5];//for last 10 trials//this array will be used at the start and end of prompting
	float tempArray[5];
	//float inv;
	float val;//January 3

	//members
	string fileName, tempFileName;
	CView* view;
	int userHistoryLength;

	//Functions
	void SetDefaultStartingPromptLevels();
	void ReadAverageNumberOfPromptsNeededFromFile();
	void GetUSRsfromTrial();
	void CopyUserHistory(int userHistoryLength);

	bool CheckIfUserPerformanceHistoryExists();
	void CreateUserPerformanceHistoryFile();
	void ParseUserHistory(string line);
	void SetStartingPromptLevels();
	void UpdateUserHistory();
	void CalculateUserHistoryLength();
	void UpdateWithLatestPerformance();
	void CalculateNewAverageUSRs();
	void UpdateWithNewAverageUSRs();

	void DebugByWritingToFile(string str);
	void DebugByWritingToFile(int num);
	void DebugByWritingToFile(float num);
	void Debug(int num);
public:
	void EndPrompting();
};
