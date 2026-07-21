#pragma once
#include "Stopwatch.h"

class FpsCounter
{
private:
	int m_fps, m_count;
	unsigned long m_startTime;

	Stopwatch m_stopwatch;

public:
	void Initialize();
	void Update();
	int GetFps();

	void StartTimer();
	DWORD GetTimerElapsedTime();
	DWORD StopTimer();

	BOOL IsTimerRunning();
};