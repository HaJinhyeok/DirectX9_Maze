#include "MazePathfinding.h"

#include <array>
#include <queue>

namespace
{
	constexpr std::array<MazeCellPosition, 4> kNeighborOffsets =
	{
		MazeCellPosition{ -1, 0 },		// 위
		MazeCellPosition{ 1, 0 },		// 아래
		MazeCellPosition{ 0, -1 },		// 왼쪽
		MazeCellPosition{ 0, 1 }		// 오른쪽
	};

	bool IsWalkableCell(
		const MazeDefinition& maze,
		int row,
		int column) noexcept
	{
		return maze.IsInside(row, column) &&
			maze.GetCell(row, column) != '*';
	}
}

MazePathfindingResult BuildMazeDistanceField(
	const MazeDefinition& maze,
	const MazeCellPosition& target)
{
	MazePathfindingResult result;

	if (!IsWalkableCell(maze, target.row, target.column))
	{
		result.errorMessage = "Pathfinding target must be a walkable maze cell.";
		return result;
	}

	result.distanceField.distances.reserve(maze.cells.size());

	for (const std::string& row : maze.cells)
	{
		result.distanceField.distances.emplace_back(
			row.size(),
			kUnreachableMazeDistance);
	}

	std::queue<MazeCellPosition> cellsToVisit;

	result.distanceField.distances[target.row][target.column] = 0;
	cellsToVisit.push(target);

	while (!cellsToVisit.empty())
	{
		const MazeCellPosition currentCell = cellsToVisit.front();
		cellsToVisit.pop();

		const int currentDistance =
			result.distanceField.GetDistance(
				currentCell.row,
				currentCell.column);

		for (const MazeCellPosition& offset : kNeighborOffsets)
		{
			const MazeCellPosition neighborCell
			{
				currentCell.row + offset.row,
				currentCell.column + offset.column
			};

			if (!IsWalkableCell(
				maze,
				neighborCell.row,
				neighborCell.column))
			{
				continue;
			}

			int& neighborDistance =
				result.distanceField.distances
				[neighborCell.row][neighborCell.column];

			if (neighborDistance != kUnreachableMazeDistance)
			{
				continue;
			}

			neighborDistance = currentDistance + 1;
			cellsToVisit.push(neighborCell);
		}
	}

	result.isSuccessful = true;
	return result;
}

bool TryGetNextMazeCell(
	const MazeDistanceField& distanceField,
	const MazeCellPosition& currentCell,
	MazeCellPosition& nextCell) noexcept
{
	const int currentDistance =
		distanceField.GetDistance(
			currentCell.row,
			currentCell.column);

	if (currentDistance <= 0)
	{
		return false;
	}

	int bestDistance = currentDistance;
	bool hasNextCell = false;

	for (const MazeCellPosition& offset : kNeighborOffsets)
	{
		const MazeCellPosition neighborCell
		{
			currentCell.row + offset.row,
			currentCell.column + offset.column
		};

		const int neighborDistance =
			distanceField.GetDistance(
				neighborCell.row,
				neighborCell.column);

		if (neighborDistance < 0 ||
			neighborDistance >= bestDistance)
		{
			continue;
		}

		bestDistance = neighborDistance;
		nextCell = neighborCell;
		hasNextCell = true;
	}

	return hasNextCell;
}