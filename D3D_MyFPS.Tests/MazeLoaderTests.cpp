#include "../MazeLoader.h"
#include "../MazeCoordinates.h"
#include "../LevelCatalog.h"
#include "../BulletCollision.h"
#include "../CombatCollision.h"
#include "../EnemySpawn.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

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

	std::cout
		<< "Tests: 25, Failed: "
		<< failedTestCount
		<< '\n';

	return failedTestCount == 0 ? 0 : 1;
}