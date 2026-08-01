#pragma once

#include "MazeDefinition.h"

struct MazeWorldPosition
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

MazeWorldPosition CalculateMazeCellCenterPosition(
	const MazeDefinition& maze,
	int row,
	int column,
	float tileSize) noexcept;

MazeCellPosition CalculateMazeCellPosition(
	const MazeDefinition& maze,
	const MazeWorldPosition& position,
	float tileSize) noexcept;