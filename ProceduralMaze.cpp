#include "ProceduralMaze.h"

#include <algorithm>
#include <array>
#include <queue>
#include <random>
#include <vector>

namespace
{
	constexpr char kWallCell = '*';
	constexpr char kPassageCell = ' ';
	constexpr char kPlayerStartCell = 'P';
	constexpr char kExitCell = 'X';

	struct PassageCell
	{
		int row = 0;
		int column = 0;
	};

	constexpr std::array<PassageCell, 4> kNeighborOffsets =
	{
		PassageCell{-1, 0},
		PassageCell{1, 0},
		PassageCell{0, -1},
		PassageCell{0, 1}
	};

	int ToMazeCoordinate(int passageCoordinate) noexcept
	{
		return passageCoordinate * 2 + 1;
	}

	std::string ValidateConfig(const ProceduralMazeConfig& config)
	{
		if (config.passageRowCount <= 0 ||
			config.passageColumnCount <= 0)
		{
			return "Passage row and column counts must be positive.";
		}

		if (config.passageRowCount == 1 &&
			config.passageColumnCount == 1)
		{
			return "Maze must contain at least two passage cells.";
		}

		return {};
	}

	MazeDefinition CreateWallFilledMaze(const ProceduralMazeConfig& config)
	{
		const int mazeHeight = ToMazeCoordinate(config.passageRowCount);
		const int mazeWidth = ToMazeCoordinate(config.passageColumnCount);

		MazeDefinition maze;
		maze.cells.assign(
			mazeHeight,
			std::string(mazeWidth, kWallCell));

		return maze;
	}

	void CarvePassages(
		MazeDefinition& maze,
		const ProceduralMazeConfig& config)
	{
		std::mt19937 randomEngine(config.seed);

		std::vector<std::vector<bool>> visited(
			config.passageRowCount,
			std::vector<bool>(config.passageColumnCount, false));

		std::vector<PassageCell> passageStack;

		const PassageCell startCell{ 0, 0 };
		passageStack.push_back(startCell);
		visited[startCell.row][startCell.column] = true;

		maze.cells[ToMazeCoordinate(startCell.row)][ToMazeCoordinate(startCell.column)] = kPassageCell;

		while (!passageStack.empty())
		{
			const PassageCell currentCell = passageStack.back();

			std::array<PassageCell, 4> shuffledOffsets = kNeighborOffsets;
			std::shuffle(
				shuffledOffsets.begin(),
				shuffledOffsets.end(),
				randomEngine);

			bool didAdvance = false;

			for (const PassageCell& offset : shuffledOffsets)
			{
				const PassageCell nextCell
				{
					currentCell.row + offset.row,
					currentCell.column + offset.column
				};

				const bool isInside =
					nextCell.row >= 0 &&
					nextCell.row < config.passageRowCount &&
					nextCell.column >= 0 &&
					nextCell.column < config.passageColumnCount;

				if (!isInside ||
					visited[nextCell.row][nextCell.column])
				{
					continue;
				}

				const int currentMazeRow = ToMazeCoordinate(currentCell.row);
				const int currentMazeColumn = ToMazeCoordinate(currentCell.column);
				const int nextMazeRow = ToMazeCoordinate(nextCell.row);
				const int nextMazeColumn = ToMazeCoordinate(nextCell.column);

				maze.cells[(currentMazeRow + nextMazeRow) / 2][(currentMazeColumn + nextMazeColumn) / 2]
					= kPassageCell;
				maze.cells[nextMazeRow][nextMazeColumn] = kPassageCell;

				visited[nextCell.row][nextCell.column] = true;
				passageStack.push_back(nextCell);
				didAdvance = true;
				break;
			}

			if (!didAdvance)
			{
				passageStack.pop_back();
			}
		}
	}

	PassageCell FindFarthestPassageCell(
		const MazeDefinition& maze,
		const ProceduralMazeConfig& config,
		const PassageCell& startCell)
	{
		std::vector<std::vector<int>> distances(
			config.passageRowCount,
			std::vector<int>(config.passageColumnCount, -1));

		std::queue<PassageCell> searchQueue;
		searchQueue.push(startCell);
		distances[startCell.row][startCell.column] = 0;

		PassageCell farthestCell = startCell;
		int farthestDistance = 0;

		while (!searchQueue.empty())
		{
			const PassageCell currentCell = searchQueue.front();
			searchQueue.pop();

			for (const PassageCell& offset : kNeighborOffsets)
			{
				const PassageCell nextCell
				{
					currentCell.row + offset.row,
					currentCell.column + offset.column
				};

				const bool isInside =
					nextCell.row >= 0 &&
					nextCell.row < config.passageRowCount &&
					nextCell.column >= 0 &&
					nextCell.column < config.passageColumnCount;

				if (!isInside ||
					distances[nextCell.row][nextCell.column] >= 0)
				{
					continue;
				}

				const int currentMazeRow = ToMazeCoordinate(currentCell.row);
				const int currentMazeColumn = ToMazeCoordinate(currentCell.column);
				const int nextMazeRow = ToMazeCoordinate(nextCell.row);
				const int nextMazeColumn = ToMazeCoordinate(nextCell.column);

				const int wallRow = (currentMazeRow + nextMazeRow) / 2;
				const int wallColumn = (currentMazeColumn + nextMazeColumn) / 2;

				if (maze.GetCell(wallRow, wallColumn) == kWallCell)
				{
					continue;
				}

				const int nextDistance = distances[currentCell.row][currentCell.column] + 1;

				distances[nextCell.row][nextCell.column] = nextDistance;
				searchQueue.push(nextCell);

				if (nextDistance > farthestDistance)
				{
					farthestDistance = nextDistance;
					farthestCell = nextCell;
				}
			}
		}

		return farthestCell;
	}
}

ProceduralMazeResult GenerateProceduralMaze(const ProceduralMazeConfig& config)
{
	ProceduralMazeResult result;
	result.errorMessage = ValidateConfig(config);

	if (!result.errorMessage.empty())
	{
		return result;
	}

	result.maze = CreateWallFilledMaze(config);
	CarvePassages(result.maze, config);

	const PassageCell startCell{ 0, 0 };
	const PassageCell exitCell =
		FindFarthestPassageCell(
			result.maze,
			config,
			startCell);

	result.maze.playerStart =
		MazeCellPosition
	{
		ToMazeCoordinate(startCell.row),
		ToMazeCoordinate(startCell.column)
	};

	result.maze.exit =
		MazeCellPosition
	{
		ToMazeCoordinate(exitCell.row),
		ToMazeCoordinate(exitCell.column)
	};

	result.maze.cells
		[result.maze.playerStart.row]
		[result.maze.playerStart.column] =
		kPlayerStartCell;

	result.maze.cells
		[result.maze.exit.row]
		[result.maze.exit.column] =
		kExitCell;

	result.isSuccessful = true;
	return result;
}