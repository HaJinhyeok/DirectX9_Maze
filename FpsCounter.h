#pragma once

class FpsCounter
{
private:
	int m_fps, m_count;
	float m_elapsedTimeSeconds;
	float m_averageFrameTimeMilliseconds;

public:
	void Initialize();
	void Update(float deltaTimeSeconds);
	int GetFps() const;
	float GetAverageFrameTimeMilliseconds() const;
};