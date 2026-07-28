#include "LevelCatalog.h"

#include <algorithm>
#include <fstream>

namespace
{
	std::string Trim(const std::string& text)
	{
		const size_t first = text.find_first_not_of(" \t\r\n");

		if (first == std::string::npos)
			return "";

		const size_t last = text.find_last_not_of(" \t\r\n");

		return text.substr(first, last - first + 1);
	}
}

LevelCatalogLoadResult LoadLevelCatalogFromFile(const std::string& filePath)
{
	LevelCatalogLoadResult result;
	std::ifstream inputFile(filePath);

	if (!inputFile.is_open())
	{
		result.errorMessage = "Failed to open level catalog: " + filePath;

		return result;
	}

	std::string line;
	size_t lineNumber = 0;

	while (std::getline(inputFile, line))
	{
		lineNumber++;

		const std::string levelPath = Trim(line);

		if (levelPath.empty() || levelPath.front() == '#')
			continue;

		if (std::find(
			result.levelPaths.begin(),
			result.levelPaths.end(),
			levelPath) != result.levelPaths.end())
		{
			result.errorMessage =
				"Duplicate level path: " + filePath +
				", line " + std::to_string(lineNumber) +
				", value '" + levelPath + "'";

			result.levelPaths.clear();
			return result;
		}

		result.levelPaths.push_back(levelPath);
	}

	if (inputFile.bad())
	{
		result.errorMessage = "Failed to read level catalog: " + filePath;

		result.levelPaths.clear();
		return result;
	}

	if (result.levelPaths.empty())
	{
		result.errorMessage = "Level catalog is empty: " + filePath;

		return result;
	}

	result.isSuccessful = true;

	return result;
}