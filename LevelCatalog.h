#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct LevelCatalogEntry
{
	std::string path;
	int enemyCount = 0;
	std::uint32_t enemySpawnSeed = 0;
};

struct LevelCatalogLoadResult
{
	bool isSuccessful = false;
	std::vector<LevelCatalogEntry> levels;
	std::string errorMessage;
};

LevelCatalogLoadResult LoadLevelCatalogFromFile(const std::string& filePath);