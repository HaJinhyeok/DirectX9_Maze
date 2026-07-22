#pragma once

class FpsCounter
{
private:
	int m_fps, m_count;
	float m_elapsedTimeSeconds;

public:
	void Initialize();
	void Update(float deltaTimeSeconds);
	int GetFps();
};