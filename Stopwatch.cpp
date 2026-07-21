#include "Stopwatch.h"

void Stopwatch::Start()
{
	m_startTimePoint = timeGetTime();
	m_bIsWorking = TRUE;
}
DWORD Stopwatch::GetElapsedTime()
{
	DWORD checkPoint = timeGetTime();
	DWORD duration = checkPoint - m_startTimePoint;
	return duration;
}
DWORD Stopwatch::Stop()
{
	DWORD endTimePoint = timeGetTime();
	DWORD duration = endTimePoint - m_startTimePoint;
	m_startTimePoint = endTimePoint;
	m_bIsWorking = FALSE;
	return duration;
}
BOOL Stopwatch::IsRunning()
{
	return m_bIsWorking;
}