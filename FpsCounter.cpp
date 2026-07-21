#include "FpsCounter.h"

void FpsCounter::Initialize()
{
	mFps = 0;
	mCount = 0;
	mStartTime = timeGetTime();
}
void FpsCounter::Update()
{
	mCount++;
	// Sleep(1000 / nLimit);
	// Sleep(15);
	// '현재' 시각이 '시작' 시각으로부터 1000ms(1초) 지났을 때
	if (timeGetTime() >= mStartTime + 1000)
	{
		mFps = mCount;
		// 카운트 초기화
		mCount = 0;
		// '현재' 시각을 다시 '시작' 시각으로 설정
		mStartTime = timeGetTime();
	}
}
int FpsCounter::GetFps()
{
	return mFps;
}
void FpsCounter::StartTimer()
{
	mStopWatch.Start();
}
DWORD FpsCounter::GetTimerElapsedTime()
{
	return mStopWatch.GetElapsedTime();
}
DWORD FpsCounter::StopTimer()
{
	return mStopWatch.Stop();
}

BOOL FpsCounter::IsTimerRunning()
{
	return mStopWatch.IsRunning();
}