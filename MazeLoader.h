#pragma once

#include <string>

#include "MazeDefinition.h"

struct MazeLoadResult
{
	bool isSuccessful = false;
	MazeDefinition maze;
	std::string errorMessage;
};

MazeLoadResult LoadMazeFromFile(const std::string& filePath);