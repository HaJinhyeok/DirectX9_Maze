#include "../MazeLoader.h"
#include "../MazeCoordinates.h"
#include "../LevelCatalog.h"
#include "../BulletCollision.h"
#include "../CombatCollision.h"
#include "../EnemySpawn.h"
#include "../ProceduralMaze.h"
#include "../MazePathfinding.h"
#include "../PerformanceRecorder.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <queue>
#include <vector>

namespace
{
	constexpr char kTestMazePath[] = "MazeLoaderTestInput.txt";
	constexpr char kTestLevelCatalogPath[] = "LevelCatalogTestInput.txt";

	bool WriteTestFile(const std::string& filePath, const std::string& contents)
	{
		std::ofstream outputFile(filePath, std::ios::trunc);

		if (!outputFile.is_open())
			return false;

		outputFile << contents;

		return outputFile.good();
	}

	MazeLoadResult LoadMazeFromText(const std::string& contents)
	{
		if (!WriteTestFile(kTestMazePath, contents))
		{
			MazeLoadResult result;
			result.errorMessage = "Failed to create test maze file.";

			return result;
		}

		MazeLoadResult result = LoadMazeFromFile(kTestMazePath);

		std::remove(kTestMazePath);

		return result;
	}

	LevelCatalogLoadResult LoadLevelCatalogFromText(const std::string& contents)
	{
		if (!WriteTestFile(kTestLevelCatalogPath, contents))
		{
			LevelCatalogLoadResult result;
			result.errorMessage = "Failed to create test level catalog file.";

			return result;
		}

		LevelCatalogLoadResult result = LoadLevelCatalogFromFile(kTestLevelCatalogPath);

		std::remove(kTestLevelCatalogPath);

		return result;
	}

	bool TestLoadsValidMaze()
	{
		const MazeLoadResult result = LoadMazeFromText(
			"X*.\n"
			".@.\n"
			"..P\n");

		if (!result.isSuccessful)
		{
			std::cerr << result.errorMessage << '\n';
			return false;
		}

		return result.maze.GetWidth() == 3 &&
			result.maze.GetHeight() == 3 &&
			result.maze.playerStart.row == 2 &&
			result.maze.playerStart.column == 2 &&
			result.maze.exit.row == 0 &&
			result.maze.exit.column == 0 &&
			result.maze.notices.size() == 1;
	}

	int RunTest(const char* testName, bool (*testFunction)())
	{
		if (testFunction())
		{
			std::cout << "[PASS] " << testName << '\n';
			return 0;
		}

		std::cerr << "[FAIL] " << testName << '\n';
		return 1;
	}

	bool IsMazeExitReachable(const MazeDefinition& maze)
	{
		if (!maze.IsInside(
			maze.playerStart.row,
			maze.playerStart.column))
		{
			return false;
		}

		std::vector<std::vector<bool>> visited(
			maze.GetHeight(),
			std::vector<bool>(maze.GetWidth(), false));

		std::queue<MazeCellPosition> searchQueue;
		searchQueue.push(maze.playerStart);
		visited[maze.playerStart.row][maze.playerStart.column] = true;

		constexpr MazeCellPosition kOffsets[] =
		{
			{ -1, 0 },
			{ 1, 0 },
			{ 0, -1 },
			{ 0, 1 }
		};

		while (!searchQueue.empty())
		{
			const MazeCellPosition current = searchQueue.front();
			searchQueue.pop();

			if (current.row == maze.exit.row &&
				current.column == maze.exit.column)
			{
				return true;
			}

			for (const MazeCellPosition& offset : kOffsets)
			{
				const MazeCellPosition next
				{
					current.row + offset.row,
					current.column + offset.column
				};

				if (!maze.IsInside(next.row, next.column) ||
					maze.GetCell(next.row, next.column) == '*' ||
					visited[next.row][next.column])
				{
					continue;
				}

				visited[next.row][next.column] = true;
				searchQueue.push(next);
			}
		}

		return false;
	}

	bool Contains(const std::string& text, const std::string& expectedText)
	{
		return text.find(expectedText) != std::string::npos;
	}

	bool NearlyEqual(
		float first,
		float second,
		float tolerance = 0.0001f)
	{
		return std::fabs(first - second) <= tolerance;
	}

	bool TestDetectsOverlappingSpheres()
	{
		const CollisionSphere first =
		{
			{ 0.0f, 0.0f, 0.0f },
			1.0f
		};

		const CollisionSphere second =
		{
			{ 1.5f, 0.0f, 0.0f },
			1.0f
		};

		return DoSpheresOverlap(first, second);
	}

	bool TestRejectsSeparatedSpheres()
	{
		const CollisionSphere first =
		{
			{ 0.0f, 0.0f, 0.0f },
			1.0f
		};

		const CollisionSphere second =
		{
			{ 2.1f, 0.0f, 0.0f },
			1.0f
		};

		return !DoSpheresOverlap(first, second);
	}

	bool TestSweepsSphereIntoTarget()
	{
		const MazeWorldPosition startPosition = { -5.0f, 0.0f, 0.0f };
		const MazeWorldPosition endPosition = { 5.0f, 0.0f, 0.0f };

		const CollisionSphere target =
		{
			{ 0.0f, 0.0f, 0.0f },
			1.0f
		};

		const SphereSweepResult result =
			SweepSphereAgainstSphere(
				startPosition,
				endPosition,
				0.5f,
				target);

		return result.didHit &&
			NearlyEqual(result.hitTime, 0.35f) &&
			NearlyEqual(result.hitPosition.x, -1.5f) &&
			NearlyEqual(result.hitPosition.y, 0.0f) &&
			NearlyEqual(result.hitPosition.z, 0.0f);
	}

	bool TestDoesNotSweepSphereOutsideTarget()
	{
		const MazeWorldPosition startPosition = { -5.0f, 2.0f, 0.0f };
		const MazeWorldPosition endPosition = { 5.0f, 2.0f, 0.0f };

		const CollisionSphere target =
		{
			{ 0.0f, 0.0f, 0.0f },
			1.0f
		};

		const SphereSweepResult result =
			SweepSphereAgainstSphere(
				startPosition,
				endPosition,
				0.5f,
				target);

		return !result.didHit &&
			NearlyEqual(result.hitTime, 1.0f);
	}

	bool TestReportsInitialSphereOverlap()
	{
		const MazeWorldPosition startPosition = { 0.5f, 0.0f, 0.0f };
		const MazeWorldPosition endPosition = { 5.0f, 0.0f, 0.0f };

		const CollisionSphere target =
		{
			{ 0.0f, 0.0f, 0.0f },
			1.0f
		};

		const SphereSweepResult result =
			SweepSphereAgainstSphere(
				startPosition,
				endPosition,
				0.5f,
				target);

		return result.didHit &&
			NearlyEqual(result.hitTime, 0.0f) &&
			NearlyEqual(result.hitPosition.x, startPosition.x) &&
			NearlyEqual(result.hitPosition.y, startPosition.y) &&
			NearlyEqual(result.hitPosition.z, startPosition.z);
	}

	bool TestSweepsBulletThroughWall()
	{
		MazeDefinition maze;
		maze.cells =
		{
			"...",
			".*.",
			"..."
		};

		const MazeWorldPosition startPosition = { -12.0f, 5.0f, 0.0f };
		const MazeWorldPosition endPosition = { 12.0f, 5.0f, 0.0f };

		const BulletSweepResult result =
			SweepBulletAgainstMaze(
				maze,
				startPosition,
				endPosition,
				0.5f,
				10.0f);

		return result.didHitWall &&
			NearlyEqual(result.hitTime, 6.5f / 24.0f) &&
			NearlyEqual(result.hitPosition.x, -5.5f) &&
			NearlyEqual(result.hitPosition.y, 5.0f) &&
			NearlyEqual(result.hitPosition.z, 0.0f);
	}

	bool TestDoesNotHitWallOutsideBulletPath()
	{
		MazeDefinition maze;
		maze.cells =
		{
			"...",
			".*.",
			"..."
		};

		const MazeWorldPosition startPosition = { -12.0f, 5.0f, -10.0f };
		const MazeWorldPosition endPosition = { 12.0f, 5.0f, -10.0f };

		const BulletSweepResult result =
			SweepBulletAgainstMaze(
				maze,
				startPosition,
				endPosition,
				0.5f,
				10.0f);

		return !result.didHitWall &&
			NearlyEqual(result.hitTime, 1.0f) &&
			NearlyEqual(result.hitPosition.x, endPosition.x) &&
			NearlyEqual(result.hitPosition.y, endPosition.y) &&
			NearlyEqual(result.hitPosition.z, endPosition.z);
	}

	bool TestSweepsBulletIntoOuterWall()
	{
		MazeDefinition maze;
		maze.cells =
		{
			"...",
			"...",
			"..."
		};

		const MazeWorldPosition startPosition = { 0.0f, 5.0f, 0.0f };
		const MazeWorldPosition endPosition = { 30.0f, 5.0f, 0.0f };

		const BulletSweepResult result =
			SweepBulletAgainstMaze(
				maze,
				startPosition,
				endPosition,
				0.5f,
				10.0f);

		return result.didHitWall &&
			NearlyEqual(result.hitTime, 14.5f / 30.0f) &&
			NearlyEqual(result.hitPosition.x, 14.5f);
	}

	bool TestLoadsValidLevelCatalog()
	{
		const LevelCatalogLoadResult result =
			LoadLevelCatalogFromText(
				"# Levels\n"
				"\n"
				"  Level01.txt | 1 | 1001  \n"
				"Level02.txt|2|2001\n");

		if (!result.isSuccessful)
		{
			std::cerr << result.errorMessage << '\n';
			return false;
		}

		return result.levels.size() == 2 &&
			result.levels[0].path == "Level01.txt" &&
			result.levels[0].enemyCount == 1 &&
			result.levels[0].enemySpawnSeed == 1001u &&
			result.levels[1].path == "Level02.txt" &&
			result.levels[1].enemyCount == 2 &&
			result.levels[1].enemySpawnSeed == 2001u;
	}

	bool TestRejectsEmptyLevelCatalog()
	{
		const LevelCatalogLoadResult result =
			LoadLevelCatalogFromText(
				"# No registered levels\n"
				"\n"
				"\t\n");

		return !result.isSuccessful &&
			Contains(result.errorMessage, "Level catalog is empty");
	}

	bool TestRejectsDuplicateLevelPath()
	{
		const LevelCatalogLoadResult result =
			LoadLevelCatalogFromText(
				"Level01.txt|1|1001\n"
				"Level02.txt|2|2001\n"
				"Level01.txt|3|3001\n");

		return !result.isSuccessful &&
			Contains(result.errorMessage, "Duplicate level path") &&
			Contains(result.errorMessage, "line 3") &&
			Contains(result.errorMessage, "Level01.txt");
	}

	bool TestRejectsInvalidCharacter()
	{
		const MazeLoadResult result = LoadMazeFromText(
			"X?.\n"
			"...\n"
			"..P\n");

		return !result.isSuccessful &&
			Contains(result.errorMessage, "Invalid maze character") &&
			Contains(result.errorMessage, "row 1") &&
			Contains(result.errorMessage, "column 2");
	}

	bool TestRejectsMismatchedRowWidth()
	{
		const MazeLoadResult result = LoadMazeFromText(
			"X*.\n"
			"..\n"
			"..P\n");

		return !result.isSuccessful &&
			Contains(result.errorMessage, "Maze row width mismatch") &&
			Contains(result.errorMessage, "row 2") &&
			Contains(result.errorMessage, "expected 3") &&
			Contains(result.errorMessage, "actual 2");
	}

	bool TestRejectsEmptyFile()
	{
		const MazeLoadResult result = LoadMazeFromText("");

		return !result.isSuccessful &&
			Contains(result.errorMessage, "Maze file is empty");
	}

	bool TestRejectsMissingRequiredMarkers()
	{
		const MazeLoadResult missingPlayer =
			LoadMazeFromText(
				"X*.\n"
				"...\n"
				"...\n");

		const MazeLoadResult missingExit =
			LoadMazeFromText(
				"...\n"
				"...\n"
				"..P\n");

		return !missingPlayer.isSuccessful &&
			Contains(missingPlayer.errorMessage, "'P'") &&
			!missingExit.isSuccessful &&
			Contains(missingExit.errorMessage, "'X'");
	}

	bool TestRejectsDuplicateMarker()
	{
		const MazeLoadResult result =
			LoadMazeFromText(
				"X*.\n"
				"P..\n"
				"..P\n");

		return !result.isSuccessful &&
			Contains(result.errorMessage, "Duplicate maze marker 'P'") &&
			Contains(result.errorMessage, "row 3") &&
			Contains(result.errorMessage, "column 3");
	}

	bool TestRejectsMissingFile()
	{
		constexpr char missingFilePath[] = "MissingMazeFile.txt";

		std::remove(missingFilePath);

		const MazeLoadResult result = LoadMazeFromFile(missingFilePath);

		return !result.isSuccessful &&
			Contains(result.errorMessage, "Failed to open maze file");
	}

	bool HasPosition(
		const MazeWorldPosition& position,
		float x,
		float y,
		float z)
	{
		return position.x == x &&
			position.y == y &&
			position.z == z;
	}

	bool TestCalculatesMazeCellCenters()
	{
		MazeDefinition evenMaze;
		evenMaze.cells =
		{
			"....",
			"...."
		};

		const MazeWorldPosition evenTopLeft =
			CalculateMazeCellCenterPosition(evenMaze, 0, 0, 10.0f);

		const MazeWorldPosition evenBottomRight =
			CalculateMazeCellCenterPosition(evenMaze, 1, 3, 10.0f);

		MazeDefinition oddMaze;
		oddMaze.cells =
		{
			"...",
			"...",
			"..."
		};

		const MazeWorldPosition oddCenter =
			CalculateMazeCellCenterPosition(oddMaze, 1, 1, 10.0f);

		return HasPosition(evenTopLeft, -15.0f, 5.0f, 5.0f) &&
			HasPosition(evenBottomRight, 15.0f, 5.0f, -5.0f) &&
			HasPosition(oddCenter, 0.0f, 5.0f, 0.0f);
	}

	bool AreSameCells(
		const MazeCellPosition& first,
		const MazeCellPosition& second) noexcept
	{
		return first.row == second.row &&
			first.column == second.column;
	}

	bool TestConvertsMazeWorldPositionsToCells()
	{
		MazeDefinition maze;
		maze.cells =
		{
			"....",
			"....",
			"...."
		};

		for (int row = 0; row < maze.GetHeight(); ++row)
		{
			for (int column = 0;
				column < maze.GetWidth();
				++column)
			{
				const MazeWorldPosition worldPosition =
					CalculateMazeCellCenterPosition(
						maze,
						row,
						column,
						10.0f);

				const MazeCellPosition convertedCell =
					CalculateMazeCellPosition(
						maze,
						worldPosition,
						10.0f);

				if (!AreSameCells(
					convertedCell,
					MazeCellPosition{ row, column }))
				{
					return false;
				}
			}
		}

		const MazeCellPosition invalidCell =
			CalculateMazeCellPosition(
				maze,
				MazeWorldPosition{},
				0.0f);

		return AreSameCells(
			invalidCell,
			MazeCellPosition{ -1, -1 });
	}

	bool TestGeneratesValidEnemySpawns()
	{
		MazeDefinition maze;
		maze.cells =
		{
			"*****",
			"*...*",
			"*...*",
			"*...*",
			"*****"
		};
		maze.playerStart = { 1, 1 };
		maze.exit = { 3, 3 };
		maze.notices = { {2, 2} };

		const EnemySpawnResult result = GenerateEnemySpawnPositions(maze, 3, 1234u);

		if (!result.isSuccessful || result.positions.size() != 3)
			return false;

		for (size_t index = 0; index < result.positions.size(); index++)
		{
			const MazeCellPosition& position = result.positions[index];

			if (maze.GetCell(position.row, position.column) == '*' ||
				AreSameCells(position, maze.playerStart) ||
				AreSameCells(position, maze.exit) ||
				AreSameCells(position, maze.notices.front()))
			{
				return false;
			}

			for (size_t previousIndex = 0; previousIndex < index; previousIndex++)
			{
				if (AreSameCells(position, result.positions[previousIndex]))
					return false;
			}
		}

		return true;
	}

	bool TestReusesEnemySpawnSeed()
	{
		MazeDefinition maze;
		maze.cells =
		{
			"....",
			"....",
			"...."
		};
		maze.playerStart = { 0, 0 };
		maze.exit = { 2, 3 };

		const EnemySpawnResult first = GenerateEnemySpawnPositions(maze, 4, 5678u);
		const EnemySpawnResult second = GenerateEnemySpawnPositions(maze, 4, 5678u);

		if (!first.isSuccessful ||
			!second.isSuccessful ||
			first.positions.size() != second.positions.size())
		{
			return false;
		}

		for (size_t index = 0; index < first.positions.size(); index++)
		{
			if (!AreSameCells(first.positions[index], second.positions[index]))
				return false;
		}

		return true;
	}

	bool TestRejectsTooManyEnemySpawns()
	{
		MazeDefinition maze;
		maze.cells =
		{
			"P.X"
		};
		maze.playerStart = { 0, 0 };
		maze.exit = { 0, 2 };

		const EnemySpawnResult result = GenerateEnemySpawnPositions(maze, 2, 1234u);

		return !result.isSuccessful &&
			Contains(result.errorMessage, "Not enough valid enemy spawn cells");
	}

	bool TestRejectsMalformedLevelCatalogEntry()
	{
		const LevelCatalogLoadResult result =
			LoadLevelCatalogFromText("Level01.txt|1\n");

		return !result.isSuccessful &&
			Contains(result.errorMessage, "Invalid level catalog entry") &&
			Contains(result.errorMessage, "line 1") &&
			Contains(result.errorMessage, "Expected format");
	}

	bool TestRejectsNegativeEnemyCount()
	{
		const LevelCatalogLoadResult result =
			LoadLevelCatalogFromText("Level01.txt|-1|1001\n");

		return !result.isSuccessful &&
			Contains(result.errorMessage, "Enemy count must be a non-negative integer");
	}

	bool TestRejectsInvalidEnemySpawnSeed()
	{
		const LevelCatalogLoadResult result =
			LoadLevelCatalogFromText("Level01.txt|1|invalid\n");

		return !result.isSuccessful &&
			Contains(result.errorMessage, "Enemy spawn seed must be an unsigned integer");
	}

	bool TestRejectsInvalidProceduralMazeSize()
	{
		ProceduralMazeConfig config;
		config.passageRowCount = 0;
		config.passageColumnCount = 5;
		config.seed = 1234u;

		const ProceduralMazeResult result = GenerateProceduralMaze(config);

		return !result.isSuccessful &&
			Contains(result.errorMessage, "must be positive");
	}

	bool TestGeneratesProceduralMazeWithExpectedSize()
	{
		ProceduralMazeConfig config;
		config.passageRowCount = 3;
		config.passageColumnCount = 4;
		config.seed = 1234u;

		const ProceduralMazeResult result = GenerateProceduralMaze(config);

		if (!result.isSuccessful)
		{
			std::cerr << result.errorMessage << '\n';
			return false;
		}

		const MazeDefinition& maze = result.maze;

		return maze.GetHeight() == 7 &&
			maze.GetWidth() == 9 &&
			maze.playerStart.row == 1 &&
			maze.playerStart.column == 1 &&
			maze.GetCell(
				maze.playerStart.row,
				maze.playerStart.column) == 'P' &&
			maze.GetCell(
				maze.exit.row,
				maze.exit.column) == 'X' &&
			(maze.exit.row != maze.playerStart.row ||
				maze.exit.column != maze.playerStart.column);
	}

	bool TestReusesProceduralMazeSeed()
	{
		ProceduralMazeConfig config;
		config.passageRowCount = 6;
		config.passageColumnCount = 7;
		config.seed = 5678u;

		const ProceduralMazeResult first = GenerateProceduralMaze(config);
		const ProceduralMazeResult second = GenerateProceduralMaze(config);

		return first.isSuccessful &&
			second.isSuccessful &&
			first.maze.cells == second.maze.cells &&
			first.maze.playerStart.row == second.maze.playerStart.row &&
			first.maze.playerStart.column == second.maze.playerStart.column &&
			first.maze.exit.row == second.maze.exit.row &&
			first.maze.exit.column == second.maze.exit.column;
	}

	bool TestGeneratesReachableProceduralMaze()
	{
		ProceduralMazeConfig config;
		config.passageRowCount = 8;
		config.passageColumnCount = 9;
		config.seed = 9012u;

		const ProceduralMazeResult result = GenerateProceduralMaze(config);

		return result.isSuccessful &&
			IsMazeExitReachable(result.maze);
	}

	bool TestGeneratesClosedProceduralMazeBoundary()
	{
		ProceduralMazeConfig config;
		config.passageRowCount = 5;
		config.passageColumnCount = 6;
		config.seed = 3456u;

		const ProceduralMazeResult result = GenerateProceduralMaze(config);

		if (!result.isSuccessful)
		{
			return false;
		}

		const MazeDefinition& maze = result.maze;
		const int lastRow = maze.GetHeight() - 1;
		const int lastColumn = maze.GetWidth() - 1;

		for (int column = 0; column < maze.GetWidth(); ++column)
		{
			if (maze.GetCell(0, column) != '*' ||
				maze.GetCell(lastRow, column) != '*')
			{
				return false;
			}
		}

		for (int row = 0; row < maze.GetHeight(); ++row)
		{
			if (maze.GetCell(row, 0) != '*' ||
				maze.GetCell(row, lastColumn) != '*')
			{
				return false;
			}
		}

		return true;
	}

	bool TestLoadsMixedLevelCatalog()
	{
		const LevelCatalogLoadResult result =
			LoadLevelCatalogFromText(
				"file|Level01.txt|1|1001\n"
				"procedural|8|10|3001|3|3002\n");

		if (!result.isSuccessful)
		{
			std::cerr << result.errorMessage << '\n';
			return false;
		}

		if (result.levels.size() != 2)
		{
			return false;
		}

		const LevelCatalogEntry& fileLevel = result.levels[0];
		const LevelCatalogEntry& proceduralLevel = result.levels[1];

		return fileLevel.mazeSourceType == MazeSourceType::File &&
			fileLevel.path == "Level01.txt" &&
			fileLevel.enemyCount == 1 &&
			fileLevel.enemySpawnSeed == 1001u &&
			proceduralLevel.mazeSourceType == MazeSourceType::Procedural &&
			proceduralLevel.passageRowCount == 8 &&
			proceduralLevel.passageColumnCount == 10 &&
			proceduralLevel.mazeSeed == 3001u &&
			proceduralLevel.enemyCount == 3 &&
			proceduralLevel.enemySpawnSeed == 3002u;
	}

	bool TestRejectsDuplicateProceduralMaze()
	{
		const LevelCatalogLoadResult result =
			LoadLevelCatalogFromText(
				"procedural|8|10|3001|2|4001\n"
				"procedural|8|10|3001|3|4002\n");

		return !result.isSuccessful &&
			Contains(
				result.errorMessage,
				"Duplicate procedural maze") &&
			Contains(result.errorMessage, "line 2") &&
			Contains(result.errorMessage, "rows 8") &&
			Contains(result.errorMessage, "columns 10") &&
			Contains(result.errorMessage, "seed 3001");
	}

	bool TestBuildsMazeDistanceField()
	{
		MazeDefinition maze;
		maze.cells =
		{
			"*****",
			"*...*",
			"***.*",
			"*...*",
			"*****"
		};

		const MazePathfindingResult result =
			BuildMazeDistanceField(
				maze,
				MazeCellPosition{ 1, 1 });

		if (!result.isSuccessful)
		{
			std::cerr << result.errorMessage << '\n';
			return false;
		}

		return
			result.distanceField.GetDistance(1, 1) == 0 &&
			result.distanceField.GetDistance(1, 2) == 1 &&
			result.distanceField.GetDistance(1, 3) == 2 &&
			result.distanceField.GetDistance(2, 3) == 3 &&
			result.distanceField.GetDistance(3, 3) == 4 &&
			result.distanceField.GetDistance(3, 2) == 5 &&
			result.distanceField.GetDistance(3, 1) == 6 &&
			result.distanceField.GetDistance(0, 0) == kUnreachableMazeDistance;
	}

	bool TestSelectsCloserMazeCell()
	{
		MazeDistanceField distanceField;
		distanceField.distances =
		{
			{ -1, -1, -1, -1 },
			{ -1,  2,  1,  0 },
			{ -1,  3, -1, -1 },
			{ -1, -1, -1, -1 }
		};

		MazeCellPosition nextCell{ -1, -1 };

		const bool hasNextCell =
			TryGetNextMazeCell(
				distanceField,
				MazeCellPosition{ 2, 1 },
				nextCell);

		return hasNextCell &&
			nextCell.row == 1 &&
			nextCell.column == 1;
	}

	bool TestRejectsInvalidPathfindingTarget()
	{
		MazeDefinition maze;
		maze.cells =
		{
			"***",
			"*.*",
			"***"
		};

		const MazePathfindingResult wallTargetResult =
			BuildMazeDistanceField(
				maze,
				MazeCellPosition{ 0, 0 });

		const MazePathfindingResult outsideTargetResult =
			BuildMazeDistanceField(
				maze,
				MazeCellPosition{ -1, 1 });

		return !wallTargetResult.isSuccessful &&
			!wallTargetResult.errorMessage.empty() &&
			!outsideTargetResult.isSuccessful &&
			!outsideTargetResult.errorMessage.empty();
	}

	bool TestDoesNotSelectInvalidNextMazeCell()
	{
		MazeDistanceField distanceField;
		distanceField.distances =
		{
			{ -1, -1, -1, -1 },
			{ -1,  2,  1,  0 },
			{ -1,  3, -1, -1 },
			{ -1, -1, -1, -1 }
		};

		MazeCellPosition targetNextCell{ -1, -1 };
		MazeCellPosition unreachableNextCell{ -1, -1 };

		const bool hasTargetNextCell =
			TryGetNextMazeCell(
				distanceField,
				MazeCellPosition{ 1, 3 },
				targetNextCell);

		const bool hasUnreachableNextCell =
			TryGetNextMazeCell(
				distanceField,
				MazeCellPosition{ 0, 0 },
				unreachableNextCell);

		return !hasTargetNextCell &&
			targetNextCell.row == -1 &&
			targetNextCell.column == -1 &&
			!hasUnreachableNextCell &&
			unreachableNextCell.row == -1 &&
			unreachableNextCell.column == -1;
	}

	bool TestPerformanceRecorderWaitsForWarmup()
	{
		PerformanceRecorder recorder;
		recorder.Start(1.0f, 1.0f);

		if (recorder.GetState() != PerformanceCaptureState::WarmingUp)
		{
			return false;
		}

		recorder.Update(0.5f);

		if (recorder.GetState() !=
			PerformanceCaptureState::WarmingUp ||
			recorder.GetSummary().sampleCount != 0)
		{
			return false;
		}

		recorder.Update(0.5f);

		if (recorder.GetState() !=
			PerformanceCaptureState::Capturing ||
			recorder.GetSummary().sampleCount != 0)
		{
			return false;
		}

		recorder.Update(1.0f);

		const PerformanceSummary& summary = recorder.GetSummary();

		return recorder.GetState() ==
			PerformanceCaptureState::Completed &&
			summary.sampleCount == 1 &&
			NearlyEqual(
				summary.capturedDurationSeconds,
				1.0f) &&
			NearlyEqual(
				summary.averageFrameTimeMilliseconds,
				1000.0f) &&
			NearlyEqual(
				summary.percentile95FrameTimeMilliseconds,
				1000.0f) &&
			NearlyEqual(
				summary.maximumFrameTimeMilliseconds,
				1000.0f);
	}

	bool TestPerformanceRecorderCalculatesPercentiles()
	{
		PerformanceRecorder recorder;
		recorder.Start(0.0f, 0.2f);

		for (int frameTimeMilliseconds = 1;
			frameTimeMilliseconds <= 20;
			++frameTimeMilliseconds)
		{
			recorder.Update(
				frameTimeMilliseconds / 1000.0f);
		}

		const PerformanceSummary& summary = recorder.GetSummary();

		return recorder.GetState() ==
			PerformanceCaptureState::Completed &&
			summary.sampleCount == 20 &&
			NearlyEqual(
				summary.capturedDurationSeconds,
				0.21f) &&
			NearlyEqual(
				summary.averageFrameTimeMilliseconds,
				10.5f) &&
			NearlyEqual(
				summary.percentile95FrameTimeMilliseconds,
				19.0f) &&
			NearlyEqual(
				summary.maximumFrameTimeMilliseconds,
				20.0f);
	}

	bool TestPerformanceRecorderRejectsInvalidDurationsAndResets()
	{
		PerformanceRecorder recorder;

		recorder.Start(-1.0f, 1.0f);

		if (recorder.GetState() !=
			PerformanceCaptureState::Idle)
		{
			return false;
		}

		recorder.Start(0.0f, 0.0f);

		if (recorder.GetState() !=
			PerformanceCaptureState::Idle)
		{
			return false;
		}

		recorder.Start(0.0f, 0.01f);
		recorder.Update(0.01f);

		if (recorder.GetState() !=
			PerformanceCaptureState::Completed ||
			recorder.GetSummary().sampleCount != 1)
		{
			return false;
		}

		recorder.Reset();

		const PerformanceSummary& summary =
			recorder.GetSummary();

		return recorder.GetState() ==
			PerformanceCaptureState::Idle &&
			NearlyEqual(
				recorder.GetRemainingSeconds(),
				0.0f) &&
			summary.sampleCount == 0 &&
			NearlyEqual(
				summary.capturedDurationSeconds,
				0.0f) &&
			NearlyEqual(
				summary.averageFrameTimeMilliseconds,
				0.0f) &&
			NearlyEqual(
				summary.percentile95FrameTimeMilliseconds,
				0.0f) &&
			NearlyEqual(
				summary.maximumFrameTimeMilliseconds,
				0.0f);
	}
}

int main()
{
	int failedTestCount = 0;

	failedTestCount +=
		RunTest(
			"TestLoadsValidMaze",
			TestLoadsValidMaze);

	failedTestCount +=
		RunTest(
			"TestRejectsInvalidCharacter",
			TestRejectsInvalidCharacter);

	failedTestCount +=
		RunTest(
			"TestRejectsMismatchedRowWidth",
			TestRejectsMismatchedRowWidth);

	failedTestCount +=
		RunTest("TestRejectsEmptyFile", TestRejectsEmptyFile);

	failedTestCount +=
		RunTest(
			"TestRejectsMissingRequiredMarkers",
			TestRejectsMissingRequiredMarkers);

	failedTestCount +=
		RunTest(
			"TestRejectsDuplicateMarker",
			TestRejectsDuplicateMarker);

	failedTestCount +=
		RunTest(
			"TestRejectsMissingFile",
			TestRejectsMissingFile);

	failedTestCount +=
		RunTest(
			"TestCalculatesMazeCellCenters",
			TestCalculatesMazeCellCenters);

	failedTestCount +=
		RunTest(
			"TestConvertsMazeWorldPositionsToCells",
			TestConvertsMazeWorldPositionsToCells);

	failedTestCount +=
		RunTest(
			"TestLoadsValidLevelCatalog",
			TestLoadsValidLevelCatalog);

	failedTestCount +=
		RunTest(
			"TestRejectsEmptyLevelCatalog",
			TestRejectsEmptyLevelCatalog);

	failedTestCount +=
		RunTest(
			"TestRejectsDuplicateLevelPath",
			TestRejectsDuplicateLevelPath);

	failedTestCount +=
		RunTest(
			"TestSweepsBulletThroughWall",
			TestSweepsBulletThroughWall);

	failedTestCount +=
		RunTest(
			"TestDoesNotHitWallOutsideBulletPath",
			TestDoesNotHitWallOutsideBulletPath);

	failedTestCount +=
		RunTest(
			"TestSweepsBulletIntoOuterWall",
			TestSweepsBulletIntoOuterWall);

	failedTestCount +=
		RunTest(
			"TestDetectsOverlappingSpheres",
			TestDetectsOverlappingSpheres);

	failedTestCount +=
		RunTest(
			"TestRejectsSeparatedSpheres",
			TestRejectsSeparatedSpheres);

	failedTestCount +=
		RunTest(
			"TestSweepsSphereIntoTarget",
			TestSweepsSphereIntoTarget);

	failedTestCount +=
		RunTest(
			"TestDoesNotSweepSphereOutsideTarget",
			TestDoesNotSweepSphereOutsideTarget);

	failedTestCount +=
		RunTest(
			"TestReportsInitialSphereOverlap",
			TestReportsInitialSphereOverlap);

	failedTestCount +=
		RunTest(
			"TestGeneratesValidEnemySpawns",
			TestGeneratesValidEnemySpawns);

	failedTestCount +=
		RunTest(
			"TestReusesEnemySpawnSeed",
			TestReusesEnemySpawnSeed);

	failedTestCount +=
		RunTest(
			"TestRejectsTooManyEnemySpawns",
			TestRejectsTooManyEnemySpawns);

	failedTestCount +=
		RunTest(
			"TestRejectsMalformedLevelCatalogEntry",
			TestRejectsMalformedLevelCatalogEntry);

	failedTestCount +=
		RunTest(
			"TestRejectsNegativeEnemyCount",
			TestRejectsNegativeEnemyCount);

	failedTestCount +=
		RunTest(
			"TestRejectsInvalidEnemySpawnSeed",
			TestRejectsInvalidEnemySpawnSeed);

	failedTestCount +=
		RunTest(
			"TestRejectsInvalidProceduralMazeSize",
			TestRejectsInvalidProceduralMazeSize);

	failedTestCount +=
		RunTest(
			"TestGeneratesProceduralMazeWithExpectedSize",
			TestGeneratesProceduralMazeWithExpectedSize);

	failedTestCount +=
		RunTest(
			"TestReusesProceduralMazeSeed",
			TestReusesProceduralMazeSeed);

	failedTestCount +=
		RunTest(
			"TestGeneratesReachableProceduralMaze",
			TestGeneratesReachableProceduralMaze);

	failedTestCount +=
		RunTest(
			"TestGeneratesClosedProceduralMazeBoundary",
			TestGeneratesClosedProceduralMazeBoundary);

	failedTestCount +=
		RunTest(
			"TestLoadsMixedLevelCatalog",
			TestLoadsMixedLevelCatalog);

	failedTestCount +=
		RunTest(
			"TestRejectsDuplicateProceduralMaze",
			TestRejectsDuplicateProceduralMaze);

	failedTestCount +=
		RunTest(
			"TestBuildsMazeDistanceField",
			TestBuildsMazeDistanceField);

	failedTestCount +=
		RunTest(
			"TestSelectsCloserMazeCell",
			TestSelectsCloserMazeCell);

	failedTestCount +=
		RunTest(
			"TestRejectsInvalidPathfindingTarget",
			TestRejectsInvalidPathfindingTarget);

	failedTestCount +=
		RunTest(
			"TestDoesNotSelectInvalidNextMazeCell",
			TestDoesNotSelectInvalidNextMazeCell);

	failedTestCount +=
		RunTest(
			"TestPerformanceRecorderWaitsForWarmup",
			TestPerformanceRecorderWaitsForWarmup);

	failedTestCount +=
		RunTest(
			"TestPerformanceRecorderCalculatesPercentiles",
			TestPerformanceRecorderCalculatesPercentiles);

	failedTestCount +=
		RunTest(
			"TestPerformanceRecorderRejectsInvalidDurationsAndResets",
			TestPerformanceRecorderRejectsInvalidDurationsAndResets);

	std::cout
		<< "Tests: 40, Failed: "
		<< failedTestCount
		<< '\n';

	return failedTestCount == 0 ? 0 : 1;
}