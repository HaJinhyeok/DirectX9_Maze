#include "LevelCatalog.h"

#include <algorithm>
#include <fstream>
#include <charconv>

namespace
{
	constexpr char kFileSource[] = "file";
	constexpr char kProceduralSource[] = "procedural";

	std::string Trim(const std::string& text)
	{
		const size_t first = text.find_first_not_of(" \t\r\n");

		if (first == std::string::npos)
			return "";

		const size_t last = text.find_last_not_of(" \t\r\n");

		return text.substr(first, last - first + 1);
	}

	std::vector<std::string> SplitFields(const std::string& line)
	{
		std::vector<std::string> fields;
		size_t fieldStart = 0;

		while (true)
		{
			const size_t separator = line.find('|', fieldStart);

			if (separator == std::string::npos)
			{
				fields.push_back(Trim(line.substr(fieldStart)));
				break;
			}

			fields.push_back(Trim(line.substr(fieldStart, separator - fieldStart)));

			fieldStart = separator + 1;
		}

		return fields;
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

	bool TryParseFileLevelCatalogEntry(
		const std::vector<std::string>& fields,
		LevelCatalogEntry& entry,
		std::string& errorMessage)
	{
		if (fields.size() != 4)
		{
			errorMessage =
				"Expected file format "
				"'file|path|enemyCount|enemySpawnSeed'.";

			return false;
		}

		LevelCatalogEntry parsedEntry;
		parsedEntry.mazeSourceType = MazeSourceType::File;
		parsedEntry.path = fields[1];

		if (parsedEntry.path.empty())
		{
			errorMessage = "Level path cannot be empty.";
			return false;
		}

		if (!TryParseInteger(
			fields[2],
			parsedEntry.enemyCount) ||
			parsedEntry.enemyCount < 0)
		{
			errorMessage = "Enemy count must be a non-negative integer.";

			return false;
		}

		if (!TryParseInteger(
			fields[3],
			parsedEntry.enemySpawnSeed))
		{
			errorMessage = "Enemy spawn seed must be an unsigned integer.";

			return false;
		}

		entry = parsedEntry;
		return true;
	}

	bool TryParseProceduralLevelCatalogEntry(
		const std::vector<std::string>& fields,
		LevelCatalogEntry& entry,
		std::string& errorMessage)
	{
		if (fields.size() != 6)
		{
			errorMessage =
				"Expected procedural format "
				"'procedural|passageRows|passageColumns|"
				"mazeSeed|enemyCount|enemySpawnSeed'.";

			return false;
		}

		LevelCatalogEntry parsedEntry;
		parsedEntry.mazeSourceType = MazeSourceType::Procedural;

		if (!TryParseInteger(
			fields[1],
			parsedEntry.passageRowCount) ||
			parsedEntry.passageRowCount <= 0)
		{
			errorMessage = "Passage row count must be a positive integer.";

			return false;
		}

		if (!TryParseInteger(
			fields[2],
			parsedEntry.passageColumnCount) ||
			parsedEntry.passageColumnCount <= 0)
		{
			errorMessage = "Passage column count must be a positive integer.";

			return false;
		}

		if (parsedEntry.passageRowCount == 1 &&
			parsedEntry.passageColumnCount == 1)
		{
			errorMessage =
				"Procedural maze must contain at least "
				"two passage cells.";

			return false;
		}

		if (!TryParseInteger(
			fields[3],
			parsedEntry.mazeSeed))
		{
			errorMessage = "Maze seed must be an unsigned integer.";

			return false;
		}

		if (!TryParseInteger(
			fields[4],
			parsedEntry.enemyCount) ||
			parsedEntry.enemyCount < 0)
		{
			errorMessage = "Enemy count must be a non-negative integer.";

			return false;
		}

		if (!TryParseInteger(
			fields[5],
			parsedEntry.enemySpawnSeed))
		{
			errorMessage = "Enemy spawn seed must be an unsigned integer.";

			return false;
		}

		entry = parsedEntry;
		return true;
	}

	bool TryParseLevelCatalogEntry(
		const std::string& line,
		LevelCatalogEntry& entry,
		std::string& errorMessage)
	{
		std::vector<std::string> fields = SplitFields(line);

		const bool isLegacyFileEntry =
			fields.size() == 3 &&
			fields[0] != kFileSource &&
			fields[0] != kProceduralSource;

		if (isLegacyFileEntry)
		{
			std::vector<std::string> normalizedFields;
			normalizedFields.reserve(4);
			normalizedFields.push_back(kFileSource);
			normalizedFields.insert(
				normalizedFields.end(),
				fields.begin(),
				fields.end());

			return TryParseFileLevelCatalogEntry(
				normalizedFields,
				entry,
				errorMessage);
		}

		if (!fields.empty() &&
			fields[0] == kFileSource)
		{
			return TryParseFileLevelCatalogEntry(
				fields,
				entry,
				errorMessage);
		}

		if (!fields.empty() &&
			fields[0] == kProceduralSource)
		{
			return TryParseProceduralLevelCatalogEntry(
				fields,
				entry,
				errorMessage);
		}

		errorMessage =
			"Expected format "
			"'file|path|enemyCount|enemySpawnSeed' or "
			"'procedural|passageRows|passageColumns|"
			"mazeSeed|enemyCount|enemySpawnSeed'.";

		return false;
	}

	bool HasSameMazeSource(
		const LevelCatalogEntry& first,
		const LevelCatalogEntry& second)
	{
		if (first.mazeSourceType != second.mazeSourceType)
		{
			return false;
		}

		if (first.mazeSourceType == MazeSourceType::File)
		{
			return first.path == second.path;
		}

		return first.passageRowCount == second.passageRowCount &&
			first.passageColumnCount == second.passageColumnCount &&
			first.mazeSeed == second.mazeSeed;
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

		const bool hasDuplicateMazeSource =
			std::any_of(
				result.levels.begin(),
				result.levels.end(),
				[&entry](
					const LevelCatalogEntry& existingEntry)
				{
					return HasSameMazeSource(
						existingEntry,
						entry);
				});


		if (hasDuplicateMazeSource)
		{
			if (entry.mazeSourceType == MazeSourceType::File)
			{
				result.errorMessage =
					"Duplicate level path: " + filePath +
					", line " +
					std::to_string(lineNumber) +
					", value '" + entry.path + "'";
			}
			else
			{
				result.errorMessage =
					"Duplicate procedural maze: " +
					filePath +
					", line " +
					std::to_string(lineNumber) +
					", rows " +
					std::to_string(entry.passageRowCount) +
					", columns " +
					std::to_string(entry.passageColumnCount) +
					", seed " +
					std::to_string(entry.mazeSeed);
			}

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