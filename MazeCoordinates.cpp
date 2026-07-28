#include "MazeCoordinates.h"

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