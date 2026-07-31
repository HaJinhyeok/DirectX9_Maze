#include "LevelCatalog.h"

#include <algorithm>
#include <fstream>
#include <charconv>

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

	template <typename IntegerType>
	bool TryParseInteger(
		const std::string& text,
		IntegerType& value) noexcept
	{
		if (text.empty())
			return false;

		const char* begin = text.data();
		const char* end = begin + text.size();

		const std::from_chars_result result = std::from_chars(begin, end, value);

		return result.ec == std::errc() &&
			result.ptr == end;
	}

	bool TryParseLevelCatalogEntry(
		const std::string& line,
		LevelCatalogEntry& entry,
		std::string& errorMessage)
	{
		const size_t firstSeparator = line.find('|');

		const size_t secondSeparator =
			firstSeparator == std::string::npos
			? std::string::npos
			: line.find('|', firstSeparator + 1);

		if (firstSeparator == std::string::npos ||
			secondSeparator == std::string::npos ||
			line.find('|', secondSeparator + 1) != std::string::npos)
		{
			errorMessage = "Expected format 'path|enemyCount|enemySpawnSeed'.";

			return false;
		}

		LevelCatalogEntry parsedEntry;

		parsedEntry.path = Trim(line.substr(0, firstSeparator));

		const std::string enemyCountText = Trim(line.substr(firstSeparator + 1, secondSeparator - firstSeparator - 1));

		const std::string enemySpawnSeedText = Trim(line.substr(secondSeparator + 1));

		if (parsedEntry.path.empty())
		{
			errorMessage = "Level path cannot be empty.";
			return false;
		}

		if (!TryParseInteger(
			enemyCountText,
			parsedEntry.enemyCount) ||
			parsedEntry.enemyCount < 0)
		{
			errorMessage = "Enemy count must be a non-negative integer.";

			return false;
		}

		if (!TryParseInteger(
			enemySpawnSeedText,
			parsedEntry.enemySpawnSeed))
		{
			errorMessage = "Enemy spawn seed must be an unsigned integer.";

			return false;
		}

		entry = parsedEntry;

		return true;
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

		const std::string trimmedLine = Trim(line);

		if (trimmedLine.empty() || trimmedLine.front() == '#')
			continue;

		LevelCatalogEntry entry;
		std::string parseErrorMessage;

		if (!TryParseLevelCatalogEntry(
			trimmedLine,
			entry,
			parseErrorMessage))
		{
			result.errorMessage =
				"Invalid level catalog entry: " + filePath +
				", line " + std::to_string(lineNumber) +
				", " + parseErrorMessage;

			result.levels.clear();

			return result;
		}

		const bool isDuplicatePath =
			std::any_of(
				result.levels.begin(),
				result.levels.end(),
				[&entry](const LevelCatalogEntry& existingEntry)
				{
					return existingEntry.path == entry.path;
				});

		if (isDuplicatePath)
		{
			result.errorMessage =
				"Duplicate level path: " + filePath +
				", line " + std::to_string(lineNumber) +
				", value '" + entry.path + "'";

			result.levels.clear();

			return result;
		}

		result.levels.push_back(entry);
	}

	if (inputFile.bad())
	{
		result.errorMessage = "Failed to read level catalog: " + filePath;
		result.levels.clear();

		return result;
	}

	if (result.levels.empty())
	{
		result.errorMessage = "Level catalog is empty: " + filePath;

		return result;
	}

	result.isSuccessful = true;

	return result;
}