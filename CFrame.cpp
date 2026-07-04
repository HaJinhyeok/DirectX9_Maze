#include "CFrame.h"
void CFrame::Initialize()
{
	mFps = 0;
	mCount = 0;
	mStartTime = timeGetTime();
}
void CFrame::Frame()
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
int CFrame::GetFps()
{
	return mFps;
}
void CFrame::WatchStart()
{
	mStopWatch.StartTime();
}
DWORD CFrame::WatchTimeCheck()
{
	return mStopWatch.TimeCheck();
}
DWORD CFrame::WatchEnd()
{
	return mStopWatch.EndTime();
}

BOOL CFrame::IsWatchWorking()
{
	return mStopWatch.IsWorking();
}