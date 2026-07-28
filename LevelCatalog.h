#pragma once

#include <string>
#include <vector>

struct LevelCatalogLoadResult
{
	bool isSuccessful = false;
	std::vector<std::string> levelPaths;
	std::string errorMessage;
};

LevelCatalogLoadResult LoadLevelCatalogFromFile(const std::string& filePath);