#pragma once
#include "WideCharMultiByteConverter.h"

class Logging
{
public:
	Logging() {};
	~Logging() {};

	void openLog(CString logPath);
	void openLog(CString logPath, CString tag);
	void writeSessionStart(int sessionNumber);
	void writeSessionEnd();
	int getNTicks() { return nTicks; };
	void tick() { SetTimeStamp(CTime::GetCurrentTime(), GetTickCount() % 10000000); nTicks++; newMessage = false; };
	void Write(CString message);
	bool isSessionStarted() { return sessionStarted; };
	int getSessionNumber() { return sessionNumber; };
	CString TimeStampToString(bool hasComma);
	std::string getTimeStamp_msec() {
		CString formatedTime; formatedTime.Format(L"%d", timeStamp_msec);
		return WideCharMultiByteConverter::WideChar2MultiByte(formatedTime.GetString()); };
	bool hasNewMessage() { return newMessage; };
	std::string getLastMessage() { return WideCharMultiByteConverter::WideChar2MultiByte(lastMessage.GetString()); };

private:
	void Open(CString path);
	int GetLastSessionNumber();
	void WriteNewLine(int nLine);
	void SetTimeStamp(CTime timeStamp, DWORD timeStamp_msec) { this->timeStamp = timeStamp; this->timeStamp_msec = timeStamp_msec; }

	CString logPath;
	CStdioFile logFile;
	CString logTag;
	int sessionNumber;
	CString lastLogMessage;
	CTime timeStamp;
	DWORD timeStamp_msec;
	int nTicks;
	bool sessionStarted;

	bool newMessage;
	CString lastMessage;
};

