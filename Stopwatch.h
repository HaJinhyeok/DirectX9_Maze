#pragma once
#pragma comment(lib,"winmm.lib")
#include<Windows.h>
class Stopwatch
{
public:
	void StartTime();
	DWORD TimeCheck();
	DWORD EndTime();

	BOOL IsWorking();

private:
	DWORD m_startTimePoint;
	BOOL m_bIsWorking = FALSE;
};

