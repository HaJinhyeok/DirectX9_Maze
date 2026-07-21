#pragma once
#include "Stopwatch.h"

class FpsCounter
{
private:
	int mFps, mCount;
	unsigned long mStartTime;

	Stopwatch mStopWatch;

public:
	void Initialize();
	void Update();
	int GetFps();

	void StartTimer();
	DWORD GetTimerElapsedTime();
	DWORD StopTimer();

	BOOL IsTimerRunning();
};