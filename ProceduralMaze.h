#pragma once

#include <cstdint>
#include <string>

#include "MazeDefinition.h"

struct ProceduralMazeConfig
{
	int passageRowCount = 0;
	int passageColumnCount = 0;
	std::uint32_t seed = 0;
};

struct ProceduralMazeResult
{
	bool isSuccessful = false;
	MazeDefinition maze;
	std::string errorMessage;
};

ProceduralMazeResult GenerateProceduralMaze(const ProceduralMazeConfig& config);