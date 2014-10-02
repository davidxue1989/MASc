
#pragma once
#include <string>

class Prompts {
public:
	
	Prompts() : 
		startedPrompting(false)
		, m_hPromptingThread(NULL)
		, taskName(L"SystemPause")
		, m_ExitThread(-1)
	{};
	virtual ~Prompts() {};

	void prompt(std::wstring taskName)
	{
		this->taskName = taskName;
		ASSERT(m_hPromptingThread == NULL); //no prompting thread should be running while we start a new one
		m_hPromptingThread = CreateThread(NULL, 0, PromptingStaticThread, (PVOID)this, 0, 0);
	};
	bool isPrompting() { return m_hPromptingThread != NULL; };
	void reset() { m_ExitThread = -1; }; //resets any flags upon changing to new task
	virtual void takeRest() = 0;
	virtual void emergencyStop() = 0;
	virtual void immediatePause(bool takeRest) = 0;
	bool isImmediatePauseIssued() { return m_ExitThread != -1; };

	bool startedPrompting;

protected:
	HANDLE m_hPromptingThread;
	std::wstring taskName;
	//bool m_ExitThread;
	int m_ExitThread; //-1 is not exiting thread, 0 is exit thread without putting to rest pose, 1 is exit thread with takeRest
	
	static DWORD WINAPI PromptingStaticThread(PVOID lpParam)
	{
		Prompts* context = static_cast<Prompts*>(lpParam);
		if (context)
		{
			return context->PromptingThread();
		}
		return 0;
	};

	virtual DWORD WINAPI PromptingThread() = 0;
};