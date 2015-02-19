#include "StdAfx.h"
#include "Logging.h"

Logging::Logging(CString logPath)
	: logPath(logPath), logTag("") //dx nov 03, 2012
{
	nonVerbose = false;
}

//dx nov 03, 2012
Logging::Logging(CString logPath, CString logTag)
	: logPath(logPath), logTag(logTag)
{
	nonVerbose = false;
}

Logging::~Logging(void)
{
	//logFile.Close();
}

//Logging::Logging(CString path)
//{
//	logPath=path;
//	//Open(path);
//}

void Logging::Write(CString message)
{
	Open(logPath);

	logFile.SeekToEnd();

	//lastLogMessage = GetTimeStamp()+message;//dxnote aug 30, 2012: when using CString::Format, gotta use c_str() if using std::string
	lastLogMessage = GetTimeStamp() + "\t" + logTag + ",\t" + message; 	//dx nov 03, 2012

	if (!nonVerbose)
	{
		logFile.WriteString(lastLogMessage);
		logFile.WriteString("\n");
	}
	logFile.Close();
}

//dx nov 06, 2012:
void Logging::Write(TIMER timer, CString msg)
{	
	CString message, temp;
	//logs timerName
	message = timer.timerName + "\t" + msg + "\n";
	
	//logs nObsns
	temp.Format("%d", nObsns.wateron);
	message += temp;
	temp.Format(", %d", nObsns.faucetOccluded);
	message += temp;

	temp.Format(", %d", nObsns.nsoap_obs);
	message += temp;
	temp.Format(", %d", nObsns.nsoapspout_obs);
	message += temp;

	temp.Format(", %d", nObsns.RUBBINGHANDSDETECTEDBYOPTICALFLOW);
	message += temp;
	temp.Format(", %d", nObsns.ntapl_obs);
	message += temp;
	temp.Format(", %d", nObsns.ntapr_obs);
	message += temp;
	
	temp.Format(", %d", nObsns.towelLifted);
	message += temp;
	temp.Format(", %d", nObsns.ntowel_obs);
	message += temp;

	temp.Format(", %d", nObsns.nsink_obs);
	message += temp;
	
	//logs timer info
	temp.Format("\n%d", timer.activityDetected_ThisFrame);
	message += temp;
	temp.Format(", %d", timer.activityDetected_LastFrame);
	message += temp;
	temp.Format(", %d", timer.totalTime);
	message += temp;
	temp.Format(", %d", timer.remainingTotalTime);
	message += temp;
	temp.Format(", %d", timer.totalTimerDone);
	message += temp;
	temp.Format(", %d", timer.unitTime);
	message += temp;
	temp.Format(", %.3f", timer.remainingUnitTimeLastChecked);
	message += temp;
	temp.Format(", %d", timer.unitTimerDone);
	message += temp;

	Write(message);
}

void Logging::Write(TIMER timer, int userState)
{
	CString message, temp;
	//logs timerName
	message = timer.timerName + "\tTimer done, user state changed to ";
	temp.Format("%d!\n", userState);
	message += temp;
	
	//logs timer info
	temp.Format("%d", timer.activityDetected_ThisFrame);
	message += temp;
	temp.Format(", %d", timer.activityDetected_LastFrame);
	message += temp;
	temp.Format(", %d", timer.totalTime);
	message += temp;
	temp.Format(", %d", timer.remainingTotalTime);
	message += temp;
	temp.Format(", %d", timer.totalTimerDone);
	message += temp;
	temp.Format(", %d", timer.unitTime);
	message += temp;
	temp.Format(", %.3f", timer.remainingUnitTimeLastChecked);
	message += temp;
	temp.Format(", %d", timer.unitTimerDone);
	message += temp;

	Write(message);
}

void Logging::Open(CString path)
{
	logFile.Open(path,CFile::modeReadWrite);
}

CString Logging::GetCurrentTimeStamp(void)
{
	CTime t = CTime::GetCurrentTime();
	CString s = t.Format("%y/%m/%d,%H:%M:%S");
	CString currentTimeStamp;
	currentTimeStamp.Format("%s:%04d,", s, GetTickCount()%10000);//dx aug 30, 2012: display the last 4 digits of ticks since system start as a way to track the different frames within 1 sec (that's the resolution of CTime)
	//need 4 digits so to deal with 9** becoming 0** within a sec

	//return t.Format("%y/%m/%d,%H:%M:%S,");
	return currentTimeStamp;
}

CString Logging::GetTimeStamp(void)
{
	CString s = timeStamp.Format("%y-%m-%d_%H-%M-%S");
	CString formatedTime;
	formatedTime.Format("%s-%04d,", s, timeStamp_msec%10000);//dx aug 30, 2012: display the last 4 digits of ticks since system start as a way to track the different frames within 1 sec (that's the resolution of CTime)
	//need 4 digits so to deal with 9** becoming 0** within a sec

	return formatedTime;
}


//void Logging::SetTimeStamp(void)
//{
//	timeStamp = CTime::GetCurrentTime();
//	//dx aug 30, 2012: use the last 4 digits of ticks since system start as a way to track the different frames within 1 sec (that's the resolution of CTime) 
//	timeStamp_msec = GetTickCount()%10000;
//	return;
//}


void Logging::WriteNewLine(int nLine)
{
	Open(logPath);
	logFile.SeekToEnd();


	for (int i=0;i<nLine;i++)
	{
		logFile.WriteString("\n");
	}

	logFile.Close();
}

int Logging::GetLastSessionNumber(){

	Open(logPath);	
	//logFile.SeekToEnd();
	//logFile.
	int fileLength = logFile.GetLength();
	
	CString strBuff;
	CString temp;


	while (logFile.ReadString(temp)!=FALSE) 
	{
		strBuff.Append(temp);
		strBuff.Append("\r\n");
	}

	logFile.Close();


	int lastPos;
	int curPos = 0;

	curPos = strBuff.Find(",\tsession end,",1);
	while (curPos != -1)
	{
		strBuff = strBuff.Right(strBuff.GetLength()-curPos);
		lastPos = curPos;
		curPos = strBuff.Find(",\tsession end,",1);
	}

	
	strBuff=strBuff.Left(strBuff.Find("\r\n"));
	int a = strBuff.GetLength()-strBuff.Find(",")-1;
	strBuff=strBuff.Right(strBuff.GetLength()-strBuff.Find(",",1)-1); //remove "session end,"
	

	//Write();
	
	return atoi(strBuff);
}
