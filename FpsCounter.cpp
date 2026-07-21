#include "FpsCounter.h"

void FpsCounter::Initialize()
{
	m_fps = 0;
	m_count = 0;
	m_startTime = timeGetTime();
}
void FpsCounter::Update()
{
	m_count++;
	// '현재' 시각이 '시작' 시각으로부터 1000ms(1초) 지났을 때
	if (timeGetTime() >= m_startTime + 1000)
	{
		m_fps = m_count;
		// 카운트 초기화
		m_count = 0;
		// '현재' 시각을 다시 '시작' 시각으로 설정
		m_startTime = timeGetTime();
	}
}
int FpsCounter::GetFps()
{
	return m_fps;
}
void FpsCounter::StartTimer()
{
	m_stopwatch.Start();
}
DWORD FpsCounter::GetTimerElapsedTime()
{
	return m_stopwatch.GetElapsedTime();
}
DWORD FpsCounter::StopTimer()
{
	return m_stopwatch.Stop();
}

BOOL FpsCounter::IsTimerRunning()
{
	return m_stopwatch.IsRunning();
}