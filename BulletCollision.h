#pragma once

#include "MazeCoordinates.h"

struct BulletSweepResult
{
	bool didHitWall = false;		// 이동 중 벽과 충돌했는지
	float hitTime = 1.0f;			// 한 프레임 이동을 0~1로 봤을 때 충돌 시점
	MazeWorldPosition hitPosition;	// 최초 충돌 위치
};

BulletSweepResult SweepBulletAgainstMaze(
	const MazeDefinition& maze,
	const MazeWorldPosition& startPosition,
	const MazeWorldPosition& endPosition,
	float bulletRadius,
	float tileSize) noexcept;