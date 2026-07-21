#pragma once
#pragma comment(lib,"winmm.lib")
#include<Windows.h>
class Stopwatch
{
public:
	void Start();
	DWORD GetElapsedTime();
	DWORD Stop();

	BOOL IsRunning();

private:
	DWORD m_startTimePoint;
	BOOL m_bIsWorking = FALSE;
};

