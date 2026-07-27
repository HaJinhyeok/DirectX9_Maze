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

	MazeCellBounds CalculateMazeCellBounds(const MazeDefinition& maze, int row, int column)
	{
		const float halfWidth = maze.GetWidth() / 2.0f;
		const float halfHeight = maze.GetHeight() / 2.0f;

		MazeCellBounds bounds;
		bounds.minX = (column - halfWidth) * kTileSize;
		bounds.minZ = (halfHeight - row - 1) * kTileSize;
		bounds.maxX = (column + 1 - halfWidth) * kTileSize;
		bounds.maxZ = (halfHeight - row) * kTileSize;

		return bounds;
	}

	bool OverlapsPlayerBounds(const MazeCellBounds& wallBounds, const D3DXVECTOR3& targetPosition)
	{
		return wallBounds.minX <= targetPosition.x + kPlayerRadius &&
			wallBounds.maxX >= targetPosition.x - kPlayerRadius &&
			wallBounds.minZ <= targetPosition.z + kPlayerRadius &&
			wallBounds.maxZ >= targetPosition.z - kPlayerRadius;
	}

	bool IsWall(const MazeDefinition& maze, int row, int column)
	{
		return maze.IsInside(row, column) && maze.GetCell(row, column) == '*';
	}
}

D3DXVECTOR3 ResolvePlayerMazeCollision(
	const MazeDefinition& maze,
	const D3DXVECTOR3& currentPosition,
	D3DXVECTOR3 targetPosition,
	const D3DXVECTOR3& movementDirection)
{
	const int mazeWidth = maze.GetWidth();
	const int mazeHeight = maze.GetHeight();

	const int column = static_cast<int>(floorf(currentPosition.x / kTileSize + mazeWidth / 2.0f));
	const int row = static_cast<int>(ceilf(mazeHeight / 2.0f - currentPosition.z / kTileSize)) - 1;

	// x축 음의 방향으로 이동일 경우
	if (movementDirection.x < 0)
	{
		for (int offset = 0; offset < kCollisionNeighborCellCount; offset++)
		{
			const int wallRow = row - 1 + offset;
			const int wallColumn = column - 1;

			if (column == 0)
			{
				if (targetPosition.x - kPlayerRadius <= -mazeWidth / 2.0f * kTileSize)
				{
					targetPosition.x = -mazeWidth / 2.0f * kTileSize + kPlayerRadius;
				}
			}
			else if (IsWall(maze, wallRow, wallColumn))
			{
				const MazeCellBounds wallBounds = CalculateMazeCellBounds(maze, wallRow, wallColumn);

				if (OverlapsPlayerBounds(wallBounds, targetPosition))
				{
					if (IsWall(maze, row, wallColumn))
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
			if (column == mazeWidth - 1)
			{
				if (targetPosition.x + kPlayerRadius >= mazeWidth / 2.0f * kTileSize)
				{
					targetPosition.x = mazeWidth / 2.0f * kTileSize - kPlayerRadius;
				}
			}
			else if (IsWall(maze, wallRow, wallColumn))
			{
				const MazeCellBounds wallBounds = CalculateMazeCellBounds(maze, wallRow, wallColumn);
				if (OverlapsPlayerBounds(wallBounds, targetPosition))
				{
					if (IsWall(maze, row, wallColumn))
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
			if (row == mazeHeight - 1)
			{
				if (targetPosition.z - kPlayerRadius <= -mazeHeight / 2.0f * kTileSize)
				{
					targetPosition.z = -mazeHeight / 2.0f * kTileSize + kPlayerRadius;
				}
			}
			else if (IsWall(maze, wallRow, wallColumn))
			{
				const MazeCellBounds wallBounds = CalculateMazeCellBounds(maze, wallRow, wallColumn);
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
				if (targetPosition.z + kPlayerRadius >= mazeHeight / 2.0f * kTileSize)
				{
					targetPosition.z = mazeHeight / 2.0f * kTileSize - kPlayerRadius;
				}
			}
			else if (IsWall(maze, wallRow, wallColumn))
			{
				const MazeCellBounds wallBounds = CalculateMazeCellBounds(maze, wallRow, wallColumn);
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