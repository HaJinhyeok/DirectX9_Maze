#include "PlayerCollision.h"

namespace
{
	constexpr int kCollisionNeighborCellCount = 3;
	constexpr float kCollisionSeparation = 0.1f;

	struct MazeCellBounds
	{
		FLOAT minX;
		FLOAT minZ;
		FLOAT maxX;
		FLOAT maxZ;
	};

	MazeCellBounds CalculateMazeCellBounds(int row, int column)
	{
		MazeCellBounds bounds;
		bounds.minX = (column - kMazeColumnCount / 2) * kTileSize;
		bounds.minZ = (kMazeRowCount / 2 - row - 1) * kTileSize;
		bounds.maxX = (column + 1 - kMazeColumnCount / 2) * kTileSize;
		bounds.maxZ = (kMazeRowCount / 2 - row) * kTileSize;

		return bounds;
	}

	bool OverlapsPlayerBounds(const MazeCellBounds& wallBounds, const D3DXVECTOR3& targetPosition)
	{
		return wallBounds.minX <= targetPosition.x + kPlayerRadius &&
			wallBounds.maxX >= targetPosition.x - kPlayerRadius &&
			wallBounds.minZ <= targetPosition.z + kPlayerRadius &&
			wallBounds.maxZ >= targetPosition.z - kPlayerRadius;
	}

	bool IsWall(const char (*map)[kMazeColumnCount + 1], int row, int column)
	{
		if (row < 0 || row >= kMazeRowCount ||
			column < 0 || column >= kMazeColumnCount)
		{
			return false;
		}

		return map[row][column] == '*';
	}
}

D3DXVECTOR3 ResolvePlayerMazeCollision(
	const char (*map)[kMazeColumnCount + 1],
	const D3DXVECTOR3& currentPosition,
	D3DXVECTOR3 targetPosition,
	const D3DXVECTOR3& movementDirection)
{
	const int column = static_cast<int>(floorf(currentPosition.x / kTileSize)) + kMazeColumnCount / 2;
	const int row = kMazeRowCount / 2 - static_cast<int>(floorf(currentPosition.z / kTileSize)) - 1;

	// x축 음의 방향으로 이동일 경우
	if (movementDirection.x < 0)
	{
		for (int offset = 0; offset < kCollisionNeighborCellCount; offset++)
		{
			const int wallRow = row - 1 + offset;
			const int wallColumn = column - 1;
			if (column == 0)
			{
				if (targetPosition.x - kPlayerRadius <= -kMazeColumnCount / 2 * kTileSize)
				{
					targetPosition.x = -kMazeColumnCount / 2 * kTileSize + kPlayerRadius;
				}
			}
			else if (IsWall(map, wallRow, wallColumn))
			{
				const MazeCellBounds wallBounds = CalculateMazeCellBounds(wallRow, wallColumn);

				if (OverlapsPlayerBounds(wallBounds, targetPosition))
				{
					if (IsWall(map, row, wallColumn))
						targetPosition.x = wallBounds.maxX + kPlayerRadius + kCollisionSeparation;
					break;
				}
			}
		}
	}
	// x축 양의 방향으로 이동일 경우
	else if (movementDirection.x > 0)
	{
		for (int offset = 0; offset < kCollisionNeighborCellCount; offset++)
		{
			const int wallRow = row - 1 + offset;
			const int wallColumn = column + 1;
			if (column == kMazeColumnCount - 1)
			{
				if (targetPosition.x + kPlayerRadius >= kMazeColumnCount / 2 * kTileSize)
				{
					targetPosition.x = kMazeColumnCount / 2 * kTileSize - kPlayerRadius;
				}
			}
			else if (IsWall(map, wallRow, wallColumn))
			{
				const MazeCellBounds wallBounds = CalculateMazeCellBounds(wallRow, wallColumn);
				if (OverlapsPlayerBounds(wallBounds, targetPosition))
				{
					if (IsWall(map, row, wallColumn))
						targetPosition.x = wallBounds.minX - kPlayerRadius - kCollisionSeparation;
					break;
				}
			}
		}
	}

	// z축 음의 방향으로 이동일 경우
	if (movementDirection.z < 0)
	{
		for (int offset = 0; offset < kCollisionNeighborCellCount; offset++)
		{
			const int wallRow = row + 1;
			const int wallColumn = column - 1 + offset;
			if (row == kMazeRowCount - 1)
			{
				if (targetPosition.z - kPlayerRadius <= -kMazeRowCount / 2 * kTileSize)
				{
					targetPosition.z = -kMazeRowCount / 2 * kTileSize + kPlayerRadius;
				}
			}
			else if (IsWall(map, wallRow, wallColumn))
			{
				const MazeCellBounds wallBounds = CalculateMazeCellBounds(wallRow, wallColumn);
				if (OverlapsPlayerBounds(wallBounds, targetPosition))
				{
					targetPosition.z = wallBounds.maxZ + kPlayerRadius + kCollisionSeparation;
					break;
				}
			}
		}
	}
	// z축 양의 방향으로 이동일 경우
	else if (movementDirection.z > 0)
	{
		for (int offset = 0; offset < kCollisionNeighborCellCount; offset++)
		{
			const int wallRow = row - 1;
			const int wallColumn = column - 1 + offset;
			if (row == 0)
			{
				if (targetPosition.z + kPlayerRadius >= kMazeRowCount / 2 * kTileSize)
				{
					targetPosition.z = kMazeRowCount / 2 * kTileSize - kPlayerRadius;
				}
			}
			else if (IsWall(map, wallRow, wallColumn))
			{
				const MazeCellBounds wallBounds = CalculateMazeCellBounds(wallRow, wallColumn);
				if (OverlapsPlayerBounds(wallBounds, targetPosition))
				{
					targetPosition.z = wallBounds.minZ - kPlayerRadius - kCollisionSeparation;
					break;
				}
			}
		}
	}

	return targetPosition;
}