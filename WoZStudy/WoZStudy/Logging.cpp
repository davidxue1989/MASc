#include "stdafx.h"
#include "Logging.h"


void Logging::openLog(CString logPath) {
	this->logPath = logPath;
	logTag = "";
	nTicks = 0;
	sessionStarted = false;
	newMessage = false;
	lastMessage = "";
}
void Logging::openLog(CString logPath, CString tag) {
	openLog(logPath);
	logTag = tag;
}

void Logging::Open(CString path)
{
	logFile.Open(path, CFile::modeReadWrite);
}

int Logging::GetLastSessionNumber(){

	Open(logPath);
	int fileLength = logFile.GetLength();

	CString strBuff;
	CString temp;

	while (logFile.ReadString(temp) != FALSE)
	{
		strBuff.Append(temp);
		strBuff.Append(L"\r\n");
	}

	logFile.Close();


	int lastPos;
	int curPos = 0;

	curPos = strBuff.Find(L",\tSession End,", 1);
	while (curPos != -1)
	{
		strBuff = strBuff.Right(strBuff.GetLength() - curPos);
		lastPos = curPos;
		curPos = strBuff.Find(L",\tSession End,", 1);
	}

	strBuff = strBuff.Left(strBuff.Find(L"\r\n"));
	int a = strBuff.GetLength() - strBuff.Find(L",") - 1;
	strBuff = strBuff.Right(strBuff.GetLength() - strBuff.Find(L",", 1) - 1); //remove "session end,"

	return _ttoi(strBuff);
}

void Logging::WriteNewLine(int nLine)
{
	Open(logPath);
	logFile.SeekToEnd();

	for (int i = 0; i<nLine; i++) {
		logFile.WriteString(L"\n");
	}

	logFile.Close();
}

CString Logging::TimeStampToString(bool hasComma)
{
	CString s = timeStamp.Format("%y-%m-%d_%H-%M-%S");
	CString formatedTime;
	if (hasComma)
		formatedTime.Format(L"%s-%04d,", s, timeStamp_msec);
	else
		formatedTime.Format(L"%s-%04d", s, timeStamp_msec);
	return formatedTime;
}

void Logging::Write(CString message)
{
	newMessage = true;
	lastMessage = message;

	Open(logPath);
	logFile.SeekToEnd();

	lastLogMessage = TimeStampToString(true) + "\t" + logTag + ",\t" + message;
	logFile.WriteString(lastLogMessage);
	logFile.WriteString(L"\n");

	logFile.Close();
}


void Logging::writeSessionStart(int sessionNumber) {
	sessionStarted = true;
	if (sessionNumber == -1)
		this->sessionNumber = GetLastSessionNumber()+1;
	else
		this->sessionNumber = sessionNumber;
	WriteNewLine(3);
	CString s;
	s.Format(L"Session Start,%d", this->sessionNumber);
	tick();
	Write(s);
}

void Logging::writeSessionEnd() {
	sessionStarted = false;
	CString s;
	s.Format(L"Session End,%d", sessionNumber);
	Write(s);
}