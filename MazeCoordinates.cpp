#include "MazeCoordinates.h"

#include <cmath>

MazeWorldPosition CalculateMazeCellCenterPosition(
	const MazeDefinition& maze,
	int row,
	int column,
	float tileSize) noexcept
{
	MazeWorldPosition position;

	position.x = (-maze.GetWidth() / 2.0f + column + 0.5f) * tileSize;

	position.y = tileSize / 2.0f;

	position.z = (maze.GetHeight() / 2.0f - row - 0.5f) * tileSize;

	return position;
}

MazeCellPosition CalculateMazeCellPosition(
	const MazeDefinition& maze,
	const MazeWorldPosition& position,
	float tileSize) noexcept
{
	if (tileSize <= 0.0f)
	{
		return MazeCellPosition{ -1, -1 };
	}

	MazeCellPosition cell;

	cell.column = static_cast<int>(
		std::floor(
			position.x / tileSize +
			maze.GetWidth() / 2.0f));

	cell.row = static_cast<int>(
		std::floor(
			maze.GetHeight() / 2.0f -
			position.z / tileSize));

	return cell;
}