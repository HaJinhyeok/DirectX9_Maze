#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class MazeSourceType
{
	File,
	Procedural
};

struct LevelCatalogEntry
{
	MazeSourceType mazeSourceType = MazeSourceType::File;

	std::string path;

	int passageRowCount = 0;
	int passageColumnCount = 0;
	std::uint32_t mazeSeed = 0;

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