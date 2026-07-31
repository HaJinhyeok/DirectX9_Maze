#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "MazeDefinition.h"

struct EnemySpawnResult
{
	bool isSuccessful = false;
	std::vector<MazeCellPosition> positions;
	std::string errorMessage;
};

EnemySpawnResult GenerateEnemySpawnPositions(
	const MazeDefinition& maze,
	int enemyCount,
	std::uint32_t seed);