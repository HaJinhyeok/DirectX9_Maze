#include "PlayerCollision.h"

namespace
{
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
		return wallBounds.minX < targetPosition.x + kPlayerRadius &&
			wallBounds.maxX > targetPosition.x - kPlayerRadius &&
			wallBounds.minZ < targetPosition.z + kPlayerRadius &&
			wallBounds.maxZ > targetPosition.z - kPlayerRadius;
	}

	bool IsWall(const MazeDefinition& maze, int row, int column)
	{
		return maze.IsInside(row, column) && maze.GetCell(row, column) == '*';
	}

	MazeCellPosition CalculateCellPosition(const MazeDefinition& maze, const D3DXVECTOR3& position)
	{
		MazeCellPosition cell;

		cell.column = static_cast<int>(floorf(position.x / kTileSize + maze.GetWidth() / 2.0f));
		cell.row = static_cast<int>(ceilf(maze.GetHeight() / 2.0f - position.z / kTileSize)) - 1;

		return cell;
	}

	D3DXVECTOR3 ResolveXAxisCollision(
		const MazeDefinition& maze,
		const D3DXVECTOR3& currentPosition,
		const D3DXVECTOR3& targetPosition)
	{
		D3DXVECTOR3 resolvedPosition = currentPosition;
		resolvedPosition.x = targetPosition.x;
		resolvedPosition.y = targetPosition.y;

		const float movementX = targetPosition.x - currentPosition.x;

		const float minX = -maze.GetWidth() / 2.0f * kTileSize + kPlayerRadius;
		const float maxX = maze.GetWidth() / 2.0f * kTileSize - kPlayerRadius;

		if (resolvedPosition.x < minX)
		{
			resolvedPosition.x = minX;
		}
		else if (resolvedPosition.x > maxX)
		{
			resolvedPosition.x = maxX;
		}

		if (fabsf(movementX) <= kEpsilon)
			return resolvedPosition;

		const MazeCellPosition cell = CalculateCellPosition(maze, currentPosition);

		const int wallColumn = cell.column + (movementX > 0.0f ? 1 : -1);

		for (int offset = -1; offset <= 1; offset++)
		{
			const int wallRow = cell.row + offset;

			if (!IsWall(maze, wallRow, wallColumn))
				continue;

			const MazeCellBounds wallBounds = CalculateMazeCellBounds(maze, wallRow, wallColumn);

			if (!OverlapsPlayerBounds(wallBounds, resolvedPosition))
				continue;

			resolvedPosition.x =
				movementX > 0.0f
				? wallBounds.minX - kPlayerRadius
				: wallBounds.maxX + kPlayerRadius;
		}

		return resolvedPosition;
	}

	D3DXVECTOR3 ResolveZAxisCollision(
		const MazeDefinition& maze,
		const D3DXVECTOR3& currentPosition,
		const D3DXVECTOR3& targetPosition)
	{
		D3DXVECTOR3 resolvedPosition = currentPosition;
		resolvedPosition.z = targetPosition.z;
		resolvedPosition.y = targetPosition.y;

		const float movementZ = targetPosition.z - currentPosition.z;

		const float minZ = -maze.GetHeight() / 2.0f * kTileSize + kPlayerRadius;
		const float maxZ = maze.GetHeight() / 2.0f * kTileSize - kPlayerRadius;

		if (resolvedPosition.z < minZ)
		{
			resolvedPosition.z = minZ;
		}
		else if (resolvedPosition.z > maxZ)
		{
			resolvedPosition.z = maxZ;
		}

		if (fabsf(movementZ) <= kEpsilon)
			return resolvedPosition;

		const MazeCellPosition cell = CalculateCellPosition(maze, currentPosition);

		const int wallRow = cell.row + (movementZ > 0.0f ? -1 : 1);

		for (int offset = -1; offset <= 1; offset++)
		{
			const int wallColumn = cell.column + offset;

			if (!IsWall(maze, wallRow, wallColumn))
				continue;

			const MazeCellBounds wallBounds = CalculateMazeCellBounds(maze, wallRow, wallColumn);

			if (!OverlapsPlayerBounds(wallBounds, resolvedPosition))
				continue;

			resolvedPosition.z =
				movementZ > 0.0f
				? wallBounds.minZ - kPlayerRadius
				: wallBounds.maxZ + kPlayerRadius;
		}

		return resolvedPosition;
	}
}

D3DXVECTOR3 ResolvePlayerMazeCollision(
	const MazeDefinition& maze,
	const D3DXVECTOR3& currentPosition,
	const D3DXVECTOR3& targetPosition)
{
	const D3DXVECTOR3 xResolvedPosition = ResolveXAxisCollision(maze, currentPosition, targetPosition);

	return ResolveZAxisCollision(maze, xResolvedPosition, targetPosition);
}