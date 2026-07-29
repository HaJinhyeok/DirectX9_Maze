#include "BulletCollision.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float kSweepEpsilon = 0.000001f;

	struct AxisAlignedBounds
	{
		float minX;
		float minY;
		float minZ;
		float maxX;
		float maxY;
		float maxZ;
	};

	bool UpdateSweepInterval(
		float start,
		float movement,
		float minimum,
		float maximum,
		float& entryTime,
		float& exitTime) noexcept
	{
		if (std::fabs(movement) <= kSweepEpsilon)
		{
			return start >= minimum && start <= maximum;
		}

		float axisEntryTime = (minimum - start) / movement;
		float axisExitTime = (maximum - start) / movement;

		if (axisEntryTime > axisExitTime)
		{
			std::swap(axisEntryTime, axisExitTime);
		}

		entryTime = std::max(entryTime, axisEntryTime);
		exitTime = std::min(exitTime, axisExitTime);

		return entryTime <= exitTime;
	}

	bool SweepPointAgainstBounds(
		const MazeWorldPosition& startPosition,
		const MazeWorldPosition& endPosition,
		const AxisAlignedBounds& bounds,
		float& hitTime) noexcept
	{
		const float movementX = endPosition.x - startPosition.x;
		const float movementY = endPosition.y - startPosition.y;
		const float movementZ = endPosition.z - startPosition.z;

		float entryTime = 0.0f;
		float exitTime = 1.0f;

		if (!UpdateSweepInterval(
			startPosition.x,
			movementX,
			bounds.minX,
			bounds.maxX,
			entryTime,
			exitTime))
		{
			return false;
		}

		if (!UpdateSweepInterval(
			startPosition.y,
			movementY,
			bounds.minY,
			bounds.maxY,
			entryTime,
			exitTime))
		{
			return false;
		}

		if (!UpdateSweepInterval(
			startPosition.z,
			movementZ,
			bounds.minZ,
			bounds.maxZ,
			entryTime,
			exitTime))
		{
			return false;
		}

		hitTime = entryTime;

		return entryTime >= 0.0f && entryTime <= 1.0f;
	}

	AxisAlignedBounds CalculateExpandedWallBounds(
		const MazeDefinition& maze,
		int row,
		int column,
		float bulletRadius,
		float tileSize) noexcept
	{
		const MazeWorldPosition center =
			CalculateMazeCellCenterPosition(
				maze,
				row,
				column,
				tileSize);

		const float halfTileSize = tileSize / 2.0f;

		return
		{
			center.x - halfTileSize - bulletRadius,
			-bulletRadius,
			center.z - halfTileSize - bulletRadius,
			center.x + halfTileSize + bulletRadius,
			tileSize + bulletRadius,
			center.z + halfTileSize + bulletRadius
		};
	}

	MazeWorldPosition InterpolatePosition(
		const MazeWorldPosition& startPosition,
		const MazeWorldPosition& endPosition,
		float time) noexcept
	{
		MazeWorldPosition position;

		position.x = startPosition.x + (endPosition.x - startPosition.x) * time;
		position.y = startPosition.y + (endPosition.y - startPosition.y) * time;
		position.z = startPosition.z + (endPosition.z - startPosition.z) * time;

		return position;
	}
}

BulletSweepResult SweepBulletAgainstMaze(
	const MazeDefinition& maze,
	const MazeWorldPosition& startPosition,
	const MazeWorldPosition& endPosition,
	float bulletRadius,
	float tileSize) noexcept
{
	BulletSweepResult result;
	result.hitPosition = endPosition;

	if (maze.GetWidth() <= 0 ||
		maze.GetHeight() <= 0 ||
		bulletRadius < 0.0f ||
		tileSize <= 0.0f)
	{
		return result;
	}

	float earliestHitTime = 1.0f;

	for (int row = 0; row < maze.GetHeight(); row++)
	{
		for (int column = 0; column < maze.GetWidth(); column++)
		{
			if (maze.GetCell(row, column) != '*')
				continue;

			const AxisAlignedBounds wallBounds =
				CalculateExpandedWallBounds(
					maze,
					row,
					column,
					bulletRadius,
					tileSize);

			float hitTime = 1.0f;

			if (!SweepPointAgainstBounds(
				startPosition,
				endPosition,
				wallBounds,
				hitTime))
			{
				continue;
			}

			if (hitTime > earliestHitTime)
				continue;

			earliestHitTime = hitTime;
			result.didHitWall = true;
		}
	}

	const float mazeHalfWidth = maze.GetWidth() * tileSize / 2.0f;
	const float mazeHalfHeight = maze.GetHeight() * tileSize / 2.0f;

	const float mazeMinX = -mazeHalfWidth;
	const float mazeMaxX = mazeHalfWidth;
	const float mazeMinZ = -mazeHalfHeight;
	const float mazeMaxZ = mazeHalfHeight;

	const AxisAlignedBounds outerWallBounds[4] =
	{
		// 왼쪽 외곽 벽
		{
			mazeMinX - tileSize - bulletRadius,
			-bulletRadius,
			mazeMinZ - tileSize - bulletRadius,
			mazeMinX + bulletRadius,
			tileSize + bulletRadius,
			mazeMaxZ + tileSize + bulletRadius
		},
		// 오른쪽 외곽 벽
		{
			mazeMaxX - bulletRadius,
			-bulletRadius,
			mazeMinZ - tileSize - bulletRadius,
			mazeMaxX + tileSize + bulletRadius,
			tileSize + bulletRadius,
			mazeMaxZ + tileSize + bulletRadius
		},
		// 아래쪽 외곽 벽
		{
			mazeMinX - tileSize - bulletRadius,
			-bulletRadius,
			mazeMinZ - tileSize - bulletRadius,
			mazeMaxX + tileSize + bulletRadius,
			tileSize + bulletRadius,
			mazeMinZ + bulletRadius
		},
		// 위쪽 외곽 벽
		{
			mazeMinX - tileSize - bulletRadius,
			-bulletRadius,
			mazeMaxZ - bulletRadius,
			mazeMaxX + tileSize + bulletRadius,
			tileSize + bulletRadius,
			mazeMaxZ + tileSize + bulletRadius
		}
	};

	for (const AxisAlignedBounds& bounds : outerWallBounds)
	{
		float hitTime = 1.0f;
		
		if (!SweepPointAgainstBounds(
			startPosition,
			endPosition,
			bounds,
			hitTime))
		{
			continue;
		}

		if (hitTime > earliestHitTime)
			continue;

		earliestHitTime = hitTime;
		result.didHitWall = true;
	}

	if (result.didHitWall)
	{
		result.hitTime = earliestHitTime;
		result.hitPosition = InterpolatePosition(
			startPosition,
			endPosition,
			earliestHitTime);
	}

	return result;
}