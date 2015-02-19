#include "StdAfx.h"
#include "ActionModule.h"
#include <assert.h>


CActionModule::CActionModule(void)
{
	fileName = "C:\\dxFiles\\COACH4ASDresourceFiles\\ActionModule.txt";
	tempFileName = "C:\\dxFiles\\COACH4ASDresourceFiles\\ActionModule2.txt";
}

CActionModule::~CActionModule(void)
{
}

//******************** CActionModule commands  **************************************//

//******************** Beginning a Trial ***********************************************//

void CActionModule::OnFileStartPrompting()
{
	SetStartingPromptLevels();
}

//void CActionModule::SetStartingPromptLevels()
//{
//	//populate cutoffs array
//	cutoffs[0] = 0.81;
//	cutoffs[1] = 0.64;
//	cutoffs[2] = 0.49;
//	cutoffs[3] = 0.36;
//	cutoffs[4] = 0.25;
//	cutoffs[5] = 0.16;
//
//	/***************************************************************************************
//			SR						Prompt Level - Greater Prompt Level Means Greater Detail 
//	SR >= cutoffs[5]					0 - No Prompt
//	cutoffs[4] <= SR < cutoffs[5]		1
//	cutoffs[3] <= SR < cutoffs[4]		2
//	cutoffs[2] <= SR < cutoffs[3]		3
//	cutoffs[1] <= SR < cutoffs[2]		4
//	cutoffs[0] <= SR < cutoffs[1]		5
//	***************************************************************************************/
//
//
//	bool fileExists = CheckIfUserPerformanceHistoryExists();
//	if (!fileExists)
//	{
//		SetDefaultStartingPromptLevels();
//		Debug(2);
//	}
//	else
//	{
//		ReadAverageNumberOfPromptsNeededFromFile();//store values in averageNumberOfPromptsNeededForEachStep[]
//		//for each step, compare the average USR to the cutoffs, determine the starting prompt level
//		for (int i = 0; i < 5; i++)
//		{
//			for (int j = 0; j <= 5; j++)
//			{
//				inv = (1/averageNumberOfPromptsNeededForEachStep[i]);
//				Debug(7);
//				if ( inv >= cutoffs[j])
//				{
//					startingPromptLevel[i] = j;
//					break;
//				}
//			}
//		}
//		Debug(2);
//	}
//}

void CActionModule::SetStartingPromptLevels()
{
	//populate cutoffs array
	//cutoffs[0] = 0.81;
	//cutoffs[1] = 0.64;
	//cutoffs[2] = 0.49;
	//cutoffs[3] = 0.36;
	//cutoffs[4] = 0.25;
	//cutoffs[5] = 0.16;


	cutoffs[0] = 1;
	cutoffs[1] = 2;
	cutoffs[2] = 3;
	cutoffs[3] = 4;
	cutoffs[4] = 5;


	/***************************************************************************************
			SR						Prompt Level - Greater Prompt Level Means Greater Detail 
	SR >= cutoffs[5]					0 - No Prompt
	cutoffs[4] <= SR < cutoffs[5]		1
	cutoffs[3] <= SR < cutoffs[4]		2
	cutoffs[2] <= SR < cutoffs[3]		3
	cutoffs[1] <= SR < cutoffs[2]		4
	cutoffs[0] <= SR < cutoffs[1]		5
	***************************************************************************************/


	bool fileExists = CheckIfUserPerformanceHistoryExists();
	if (!fileExists)
	{
		SetDefaultStartingPromptLevels();
		Debug(2);
	}
	else
	{
		ReadAverageNumberOfPromptsNeededFromFile();//store values in averageNumberOfPromptsNeededForEachStep[]
		//for each step, compare the average USR to the cutoffs, determine the starting prompt level
		for (int i = 0; i < 5; i++)
		{
			startingPromptLevel[i] = 5;//January 13, 2012
			for (int j = 0; j <5 ; j++)
			{
				val = averageNumberOfPromptsNeededForEachStep[i];
				Debug(7);
				
				if ( val <= cutoffs[j])
				{
					startingPromptLevel[i] = j+1;//January 13, 2012
					break;
				}
			}
		}
		Debug(2);
	}
}

bool CActionModule::CheckIfUserPerformanceHistoryExists()
{
	ifstream input;
	//Try to open file
	input.open(fileName.c_str(), ifstream::in);
	input.close();

	//now check input.fail() to see if the file could be opened, create file if needed
	if(input.fail())
	{
			input.clear(ios::failbit);
			return false;
	}
	else
		return true;
}

void CActionModule::SetDefaultStartingPromptLevels()
{
	for (int i = 0; i < 5; i++)
	{
		startingPromptLevel[i] = 5;//Default prompt level = highest detail prompt
	}
}

void CActionModule::ReadAverageNumberOfPromptsNeededFromFile()
{
	//open file for reading
	ifstream input;
	input.open(fileName.c_str(), ifstream::in);
	string line;
	
	while ( getline(input, line) ){}
	input.close();
	//line now stores contents of bottom line in the text file
	DebugByWritingToFile(line);
	
	ParseUserHistory(line);
	for(int j = 0; j < 5; j++)
	{
		averageNumberOfPromptsNeededForEachStep[j] = tempArray[j];
	}
	Debug(1);
}

void CActionModule::ParseUserHistory(string line)
{
	string numberstring;
	for (int i = 0; i < 5; i++)
	{
		//Erase everything upto and including first tab in 'line':
		int foundtab;
		foundtab = int (line.find("\t"));
		line.erase(0, foundtab+1);
		//DebugByWritingToFile(line);

		//Extract the first number from the string
		numberstring = line;
		//DebugByWritingToFile(numberstring);
			
		if (i!=4)//if not last number
		{
			//find first tab in string
			foundtab = int (numberstring.find("\t"));
			//Erase first tab and everything after in numberstring
			numberstring.erase(foundtab, numberstring.length() - foundtab);
		}
		//DebugByWritingToFile(numberstring);
		//convert to float	and save number to array
		tempArray[i] = atof(numberstring.c_str());
	}
}

//******************** Ending a Trial ***********************************************//

void CActionModule::EndPrompting()
{
	GetUSRsfromTrial();
	UpdateUserHistory();
}

void CActionModule::GetUSRsfromTrial()
{
	//USRForStep[0] = 1;
	//USRForStep[1] = 1;
	//USRForStep[2] = 1;
	//USRForStep[3] = 1;
	//USRForStep[4] = 1;
}

void CActionModule::UpdateUserHistory()
{
	//open the history file
	//count number of lines in the file
	//copy contents to another file - if length(user history)=10, omit most outdated user performance
	//append the latest user performance to the end
	//Calculate new average USRs for each step
	//Add new average USRs for each step to the file

	//update history with latest performance - USRForStep[]
	//	see if file exists - if not just create AM2.txt file and print USRs
	//	if file exists - get length - if less than 10, copy to AM2.txt (except last line), add new line
	//	if > 10 - skip copying first line, copy to AM2.txt (except last line), add new line
	//	close AM2.txt
	UpdateWithLatestPerformance();


	//reopen AM2.txt - calculate new average USR, close file
	CalculateNewAverageUSRs();
	
	//Print average USRs to file
	UpdateWithNewAverageUSRs();
}

void CActionModule::UpdateWithLatestPerformance()
{
	
	//update ActionModule with latest performance - USRForStep[]
	
	//Now copy old file, deleting the most outdated user history and append the latest user performance

	//Check to see if file exists - if not just create AM2.txt file and print USRs

	bool fileExists = CheckIfUserPerformanceHistoryExists();
	if (!fileExists)
	{
		CreateUserPerformanceHistoryFile();//Print USRs to file
		userHistoryLength = 1;//increment userHistoryLength
	}
	else
	{
		CalculateUserHistoryLength();
		CopyUserHistory(userHistoryLength);
		//if (userHistoryLength <10) 
		//userHistoryLength += 1;//increment userHistoryLength
		
		//APPEND latest user performance
		ofstream output;
		output.open(tempFileName.c_str(), ios::app);//be sure to append
		output << "\t" << USRForStep[0] << "\t" << USRForStep[1] << "\t" << USRForStep[2] << "\t" << USRForStep[3] << "\t" << USRForStep[4] << endl;
		output.close();
		//increment userHistoryLength
		if (userHistoryLength !=10)
			userHistoryLength++;
	}
}

void CActionModule::CreateUserPerformanceHistoryFile()
{
	ofstream output;
	//open the file for writing
	output.open(tempFileName.c_str(), ofstream::out);
	//Print Header
	//output << "Step" << "\t" <<"1" << "\t" << "2" << "\t" << "3" << "\t" << "4" << "\t" <<"5" << endl;
	//Print average user success rates - zero for now
	output << "\t" << USRForStep[0] << "\t" << USRForStep[1] << "\t" << USRForStep[2] << "\t" << USRForStep[3] << "\t" << USRForStep[4] << endl;
	output.close();
}

void CActionModule::CalculateUserHistoryLength()
{
	//open file for reading
	ifstream input;
	input.open(fileName.c_str(), ifstream::in);
	string line;

	//count number of lines in file
	userHistoryLength = 0;

	while ( getline(input, line) )
	{
		userHistoryLength++;
	}
	input.close();
	assert(userHistoryLength>=1);
	userHistoryLength -= 1;
}

void CActionModule::CopyUserHistory(int userHistoryLength)
{
	ifstream input;
	ofstream output;
	string line;

	input.open(fileName.c_str(), ifstream::in);
	output.open(tempFileName.c_str(), ofstream::out);

	if (userHistoryLength < 10)
	{
		for (int i = 1; i <=userHistoryLength; i++)
		{
			getline(input, line);
			output << line << endl;
		}
		input.close();
		output.close();
	}
	else
	{
		//skip first line
		getline(input, line);
		//copy remainder//had problems with this code before changes - November 7, 2011
		/////userHistoryLength--;
		for (int i = 1; i <=userHistoryLength - 1; i++)
		{
			getline(input, line);
			output << line << endl;
		}
		input.close();
		output.close();
	}

}

void CActionModule::CalculateNewAverageUSRs()
{
	//calculate new average User Success Rates for each step

	string line;
	//initialize averageNumberOfPromptsNeededForEachStep[]
	for(int i = 0; i < 5; i++)
	{
		averageNumberOfPromptsNeededForEachStep[i] = 0;
	}

	
	ifstream input;
	input.open(tempFileName.c_str(), ifstream::in);

	//Now keep adding the numbers to averageNumberOfPromptsNeededForEachStep
	for (int i = 1; i <= userHistoryLength; i++)
	{
		getline(input,line);
		ParseUserHistory(line);
		for(int j = 0; j < 5; j++)
		{
			averageNumberOfPromptsNeededForEachStep[j] += tempArray[j];
		}
	}
	input.close();

	//now calculate averageNumberOfPromptsNeededForEachStep
	for(int i = 0; i < 5; i++)
	{
		averageNumberOfPromptsNeededForEachStep[i] = averageNumberOfPromptsNeededForEachStep[i]/userHistoryLength;
	}
}

void CActionModule::UpdateWithNewAverageUSRs()
{
	ofstream output;
	output.open(tempFileName.c_str(), ios::app);//be sure to append
	//add a new line to the history file
	output << "Avg:";
	for (int i = 0; i < 5; i++)
	{
		output << "\t" << averageNumberOfPromptsNeededForEachStep[i];
	}
	output.close();

	int err = remove(fileName.c_str());
	rename(tempFileName.c_str(),fileName.c_str());
}
/********************************   Debugging	*****************************************/
void CActionModule::DebugByWritingToFile(string str)
{
	ofstream output;
	output.open("C:\\dxFiles\\COACH4ASDresourceFiles\\DebugFile.txt", ofstream::app);
	output << str << endl;
	output.close();
}

void CActionModule::DebugByWritingToFile(int num)
{
	ofstream output;
	output.open("C:\\dxFiles\\COACH4ASDresourceFiles\\DebugFile.txt", ofstream::app);
	output << num << endl;
	output.close();
}

void CActionModule::DebugByWritingToFile(float num)
{
	ofstream debug;
	debug.open("C:\\dxFiles\\COACH4ASDresourceFiles\\DebugFile.txt", ofstream::app);
	debug << num << endl;
	debug.close();
}

void CActionModule::Debug(int num)
{
	switch (num)
	{
	case 1:
		DebugByWritingToFile("Average USRs For Each Step - Before Trial");
		for(int i = 0; i < 5; i++)
		{
			DebugByWritingToFile(averageNumberOfPromptsNeededForEachStep[i]);
		}
		break;
	case 2:
		DebugByWritingToFile("Starting Prompt Levels");
		for(int i = 0; i < 5; i++)
		{
			DebugByWritingToFile(startingPromptLevel[i]);
		}
		break;
	case 3:
		DebugByWritingToFile("User History Length");
		DebugByWritingToFile(userHistoryLength);
		break;
	case 4:
		DebugByWritingToFile("USRs For Each Step From This Trial");
		for(int i = 0; i < 5; i++)
		{
			DebugByWritingToFile(USRForStep[i]);
		}
		break;
	case 5:
		DebugByWritingToFile("Average USRs For Each Step - After Trial");
		for(int i = 0; i < 5; i++)
		{
			DebugByWritingToFile(averageNumberOfPromptsNeededForEachStep[i]);
		}
		break;
	case 6:
		DebugByWritingToFile("Average USRs For Each Step - After Trial");
		for(int i = 0; i < 5; i++)
		{
			DebugByWritingToFile(averageNumberOfPromptsNeededForEachStep[i]);
		}
		break;
	case 7:
		DebugByWritingToFile("Average USRs For Each Step");
		DebugByWritingToFile(val);
		break;
	}
}
