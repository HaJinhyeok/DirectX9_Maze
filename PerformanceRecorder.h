#pragma once

#include <vector>

enum class PerformanceCaptureState
{
	Idle,
	WarmingUp,
	Capturing,
	Completed
};

struct PerformanceSummary
{
	int sampleCount = 0;
	float capturedDurationSeconds = 0.0f;
	float averageFrameTimeMilliseconds = 0.0f;
	float percentile95FrameTimeMilliseconds = 0.0f;
	float maximumFrameTimeMilliseconds = 0.0f;
};

class PerformanceRecorder
{
private:
	PerformanceCaptureState m_state =
		PerformanceCaptureState::Idle;

	float m_warmupRemainingSeconds = 0.0f;
	float m_captureRemainingSeconds = 0.0f;
	float m_capturedDurationSeconds = 0.0f;

	std::vector<float> m_frameTimeMilliseconds;
	PerformanceSummary m_summary;

	void CompleteCapture();

public:
	void Start(
		float warmupDurationSeconds,
		float captureDurationSeconds);

	void Update(float frameTimeSeconds);
	void Reset() noexcept;

	PerformanceCaptureState GetState() const noexcept;
	float GetRemainingSeconds() const noexcept;
	const PerformanceSummary& GetSummary() const noexcept;
};