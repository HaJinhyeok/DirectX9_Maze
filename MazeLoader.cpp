#include "MazeLoader.h"

#include <fstream>

namespace
{
	bool ValidateMazeDimensions(
		const std::string& filePath,
		const std::vector<std::string>& cells,
		std::string& errorMessage)
	{
		if (cells.empty())
		{
			errorMessage = "Maze file is empty: " + filePath;
			return false;
		}

		const size_t expectedWidth = cells.front().size();

		if (expectedWidth == 0)
		{
			errorMessage = "Maze row is empty: " + filePath + ", row 1";

			return false;
		}

		for (size_t rowIndex = 1; rowIndex < cells.size(); rowIndex++)
		{
			const size_t actualWidth = cells[rowIndex].size();

			if (actualWidth == expectedWidth)
				continue;

			errorMessage =
				"Maze row width mismatch: " + filePath +
				", row " + std::to_string(rowIndex + 1) +
				", expected " + std::to_string(expectedWidth) +
				", actual " + std::to_string(actualWidth);

			return false;
		}

		return true;
	}

	bool IsAllowedMazeCharacter(char character) noexcept
	{
		switch (character)
		{
		case '*':
		case '.':
		case 'P':
		case 'T':
		case 'X':
		case '@':
			return true;

		default:
			return false;
		}
	}

	bool ValidateMazeCharacters(
		const std::string& filePath,
		const std::vector<std::string>& cells,
		std::string& errorMessage)
	{
		for (size_t rowIndex = 0; rowIndex < cells.size(); rowIndex++)
		{
			for (size_t columnIndex = 0; columnIndex < cells[rowIndex].size(); columnIndex++)
			{
				const char character = cells[rowIndex][columnIndex];

				if (IsAllowedMazeCharacter(character))
					continue;

				errorMessage =
					"Invalid maze character: " + filePath +
					", row " + std::to_string(rowIndex + 1) +
					", column " + std::to_string(columnIndex + 1) +
					", value '" + std::string(1, character) + "'";

				return false;
			}
		}

		return true;
	}

	bool AssignUniqueMazeMarker(
		const std::string& filePath,
		char marker,
		size_t rowIndex,
		size_t columnIndex,
		bool& hasMarker,
		MazeCellPosition& position,
		std::string& errorMessage)
	{
		if (hasMarker)
		{
			errorMessage =
				"Duplicate maze marker '" + std::string(1, marker) +
				"': " + filePath +
				", row " + std::to_string(rowIndex + 1) +
				", column " + std::to_string(columnIndex + 1);

			return false;
		}

		hasMarker = true;
		position.row = static_cast<int>(rowIndex);
		position.column = static_cast<int>(columnIndex);

		return true;
	}

	bool ParseMazeMarkers(
		const std::string& filePath,
		MazeDefinition& maze,
		std::string& errorMessage)
	{
		bool hasPlayerStart = false;
		bool hasTigerStart = false;
		bool hasExit = false;

		for (size_t rowIndex = 0; rowIndex < maze.cells.size(); rowIndex++)
		{
			for (size_t columnIndex = 0; columnIndex < maze.cells[rowIndex].size(); columnIndex++)
			{
				char& character = maze.cells[rowIndex][columnIndex];

				if (character == '*' || character == '.')
					continue;

				switch (character)
				{
				case 'P':
					if (!AssignUniqueMazeMarker(
						filePath,
						character,
						rowIndex,
						columnIndex,
						hasPlayerStart,
						maze.playerStart,
						errorMessage))
					{
						return false;
					}
					break;

				case 'T':
					if (!AssignUniqueMazeMarker(
						filePath,
						character,
						rowIndex,
						columnIndex,
						hasTigerStart,
						maze.tigerStart,
						errorMessage))
					{
						return false;
					}
					break;

				case 'X':
					if (!AssignUniqueMazeMarker(
						filePath,
						character,
						rowIndex,
						columnIndex,
						hasExit,
						maze.exit,
						errorMessage))
					{
						return false;
					}
					break;

				case '@':
					maze.notices.push_back(
						{
							static_cast<int>(rowIndex),
							static_cast<int>(columnIndex)
						});
					break;
				}

				character = '.';
			}
		}

		if (!hasPlayerStart)
		{
			errorMessage = "Missing required maze marker 'P': " + filePath;
			return false;
		}

		if (!hasTigerStart)
		{
			errorMessage = "Missing required maze marker 'T': " + filePath;
			return false;
		}

		if (!hasExit)
		{
			errorMessage = "Missing required maze marker 'X': " + filePath;
			return false;
		}

		return true;
	}
}

MazeLoadResult LoadMazeFromFile(const std::string& filePath)
{
	MazeLoadResult result;

	std::ifstream inputFile(filePath);

	if (!inputFile.is_open())
	{
		result.errorMessage = "Failed to open maze file: " + filePath;

		return result;
	}

	std::string line;

	while (std::getline(inputFile, line))
	{
		result.maze.cells.push_back(line);
	}

	if (inputFile.bad())
	{
		result.maze.cells.clear();
		result.errorMessage = "Failed while reading maze file: " + filePath;

		return result;
	}

	if (!ValidateMazeDimensions(filePath, result.maze.cells, result.errorMessage))
	{
		result.maze.cells.clear();
		return result;
	}

	if (!ValidateMazeCharacters(filePath, result.maze.cells, result.errorMessage))
	{
		result.maze.cells.clear();
		return result;
	}

	if (!ParseMazeMarkers(
		filePath,
		result.maze,
		result.errorMessage))
	{
		result.maze = MazeDefinition();
		return result;
	}

	result.isSuccessful = true;

	return result;
}