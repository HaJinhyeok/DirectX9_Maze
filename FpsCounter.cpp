#include "FpsCounter.h"

void FpsCounter::Initialize()
{
	m_fps = 0;
	m_count = 0;
	m_elapsedTimeSeconds = 0.0f;
}

void FpsCounter::Update(float deltaTimeSeconds)
{
	m_count++;
	m_elapsedTimeSeconds += deltaTimeSeconds;

	// 누적 시간이 1초 이상이면 FPS 갱신
	if (m_elapsedTimeSeconds >= 1.0f)
	{
		m_fps = static_cast<int>(m_count / m_elapsedTimeSeconds);
		// 카운트 초기화
		m_count = 0;
		// 시간 경과 리셋
		m_elapsedTimeSeconds = 0.0f;
	}
}

int FpsCounter::GetFps()
{
	return m_fps;
}