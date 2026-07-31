#include "EnemySpawn.h"

#include <algorithm>
#include <random>

namespace
{
	bool IsSameCell(
		const MazeCellPosition& first,
		const MazeCellPosition& second) noexcept
	{
		return first.row == second.row &&
			first.column == second.column;
	}

	bool IsReservedCell(
		const MazeDefinition& maze,
		const MazeCellPosition& position)
	{
		if (IsSameCell(position, maze.playerStart) ||
			IsSameCell(position, maze.exit))
		{
			return true;
		}

		return std::any_of(
			maze.notices.begin(),
			maze.notices.end(),
			[&position](const MazeCellPosition& notice)
			{
				return IsSameCell(position, notice);
			});
	}
}

EnemySpawnResult GenerateEnemySpawnPositions(
	const MazeDefinition& maze,
	int enemyCount,
	std::uint32_t seed)
{
	EnemySpawnResult result;

	if (enemyCount < 0)
	{
		result.errorMessage = "Enemy count cannot be negative.";
		return result;
	}

	std::vector<MazeCellPosition> candidates;

	for (int row = 0; row < maze.GetHeight(); row++)
	{
		for (int column = 0; column < maze.GetWidth(); column++)
		{
			if (maze.GetCell(row, column) == '*')
				continue;

			const MazeCellPosition position = { row, column };

			if (IsReservedCell(maze, position))
				continue;

			candidates.push_back(position);
		}
	}

	if (enemyCount > static_cast<int>(candidates.size()))
	{
		result.errorMessage =
			"Not enough valid enemy spawn cells. Requested: " +
			std::to_string(enemyCount) +
			", available: " +
			std::to_string(candidates.size());

		return result;
	}

	std::mt19937 randomEngine(seed);
	std::shuffle(candidates.begin(), candidates.end(), randomEngine);

	result.positions.assign(
		candidates.begin(),
		candidates.begin() + enemyCount);

	result.isSuccessful = true;

	return result;
}