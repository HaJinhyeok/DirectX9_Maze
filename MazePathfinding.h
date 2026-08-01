#pragma once

#include <string>
#include <vector>

#include "MazeDefinition.h"

constexpr int kUnreachableMazeDistance = -1;

struct MazeDistanceField
{
	std::vector<std::vector<int>> distances;

	int GetDistance(
		int row,
		int column) const noexcept
	{
		if (row < 0 ||
			row >= static_cast<int>(distances.size()) ||
			column < 0 ||
			column >= static_cast<int>(distances[row].size()))
		{
			return kUnreachableMazeDistance;
		}

		return distances[row][column];
	}
};

struct MazePathfindingResult
{
	bool isSuccessful = false;
	MazeDistanceField distanceField;
	std::string errorMessage;
};

MazePathfindingResult BuildMazeDistanceField(
	const MazeDefinition& maze,
	const MazeCellPosition& target);

bool TryGetNextMazeCell(
	const MazeDistanceField& distanceField,
	const MazeCellPosition& currentCell,
	MazeCellPosition& nextCell) noexcept;