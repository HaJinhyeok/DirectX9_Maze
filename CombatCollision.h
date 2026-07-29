#pragma once

#include "MazeCoordinates.h"

// 충돌 대상의 중심점과 반지름
struct CollisionSphere
{
	MazeWorldPosition center;
	float radius = 0.0f;
};

struct SphereSweepResult
{
	bool didHit = false;
	float hitTime = 1.0f;
	MazeWorldPosition hitPosition;
};

// 이동하는 구체가 대상 구체와 프레임 도중 충돌했는지 검사
SphereSweepResult SweepSphereAgainstSphere(
	const MazeWorldPosition& startPosition,
	const MazeWorldPosition& endPosition,
	float movingRadius,
	const CollisionSphere& target) noexcept;

// 현재 위치의 두 구체가 겹치는지 검사
bool DoSpheresOverlap(
	const CollisionSphere& first,
	const CollisionSphere& second) noexcept;