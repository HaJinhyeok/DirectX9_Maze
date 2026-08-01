#include "PerformanceRecorder.h"

#include <algorithm>
#include <cmath>

void PerformanceRecorder::CompleteCapture()
{
	m_state = PerformanceCaptureState::Completed;

	m_summary.sampleCount = static_cast<int>(m_frameTimeMilliseconds.size());

	m_summary.capturedDurationSeconds = m_capturedDurationSeconds;

	if (m_frameTimeMilliseconds.empty())
	{
		return;
	}

	double totalFrameTimeMilliseconds = 0.0;

	for (const float frameTimeMilliseconds : m_frameTimeMilliseconds)
	{
		totalFrameTimeMilliseconds += frameTimeMilliseconds;
	}

	m_summary.averageFrameTimeMilliseconds =
		static_cast<float>(
			totalFrameTimeMilliseconds /
			m_frameTimeMilliseconds.size());

	std::vector<float> sortedFrameTimes = m_frameTimeMilliseconds;

	std::sort(
		sortedFrameTimes.begin(),
		sortedFrameTimes.end());

	const std::size_t percentileIndex =
		static_cast<std::size_t>(
			std::ceil(
				sortedFrameTimes.size() * 0.95)) - 1;

	m_summary.percentile95FrameTimeMilliseconds =
		sortedFrameTimes[percentileIndex];
	m_summary.maximumFrameTimeMilliseconds =
		sortedFrameTimes.back();
}

void PerformanceRecorder::Start(
	float warmupDurationSeconds,
	float captureDurationSeconds)
{
	Reset();

	if (warmupDurationSeconds < 0.0f ||
		captureDurationSeconds <= 0.0f)
	{
		return;
	}

	m_warmupRemainingSeconds = warmupDurationSeconds;
	m_captureRemainingSeconds = captureDurationSeconds;

	m_state = warmupDurationSeconds > 0.0f
		? PerformanceCaptureState::WarmingUp
		: PerformanceCaptureState::Capturing;
}

void PerformanceRecorder::Update(float frameTimeSeconds)
{
	if (frameTimeSeconds <= 0.0f)
	{
		return;
	}

	if (m_state == PerformanceCaptureState::WarmingUp)
	{
		m_warmupRemainingSeconds -= frameTimeSeconds;

		if (m_warmupRemainingSeconds <= 0.0f)
		{
			m_warmupRemainingSeconds = 0.0f;
			m_state = PerformanceCaptureState::Capturing;
		}

		return;
	}

	if (m_state != PerformanceCaptureState::Capturing)
	{
		return;
	}

	m_frameTimeMilliseconds.push_back(frameTimeSeconds * 1000.0f);

	m_capturedDurationSeconds += frameTimeSeconds;
	m_captureRemainingSeconds -= frameTimeSeconds;

	if (m_captureRemainingSeconds <= 0.0f)
	{
		m_captureRemainingSeconds = 0.0f;
		CompleteCapture();
	}
}

void PerformanceRecorder::Reset() noexcept
{
	m_state = PerformanceCaptureState::Idle;
	m_warmupRemainingSeconds = 0.0f;
	m_captureRemainingSeconds = 0.0f;
	m_capturedDurationSeconds = 0.0f;

	m_frameTimeMilliseconds.clear();
	m_summary = PerformanceSummary{};
}

PerformanceCaptureState PerformanceRecorder::GetState() const noexcept
{
	return m_state;
}

float PerformanceRecorder::GetRemainingSeconds() const noexcept
{
	if (m_state == PerformanceCaptureState::WarmingUp)
	{
		return m_warmupRemainingSeconds;
	}

	if (m_state == PerformanceCaptureState::Capturing)
	{
		return m_captureRemainingSeconds;
	}

	return 0.0f;
}

const PerformanceSummary& PerformanceRecorder::GetSummary() const noexcept
{
	return m_summary;
}