#include "FpsCounter.h"

void FpsCounter::Initialize()
{
	m_fps = 0;
	m_count = 0;
	m_elapsedTimeSeconds = 0.0f;
	m_averageFrameTimeMilliseconds = 0.0f;
}

void FpsCounter::Update(float deltaTimeSeconds)
{
	m_count++;
	m_elapsedTimeSeconds += deltaTimeSeconds;

	// 약 1초 구간의 FPS와 평균 프레임 시간을 갱신한다.
	if (m_elapsedTimeSeconds >= 1.0f)
	{
		m_fps = static_cast<int>(m_count / m_elapsedTimeSeconds);
		m_averageFrameTimeMilliseconds = m_elapsedTimeSeconds / static_cast<float>(m_count) * 1000.0f;
		// 카운트 초기화
		m_count = 0;
		// 시간 경과 리셋
		m_elapsedTimeSeconds = 0.0f;
	}
}

int FpsCounter::GetFps() const
{
	return m_fps;
}

float FpsCounter::GetAverageFrameTimeMilliseconds() const
{
	return m_averageFrameTimeMilliseconds;
}