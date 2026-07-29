#include "../MazeLoader.h"
#include "../MazeCoordinates.h"
#include "../LevelCatalog.h"
#include "../BulletCollision.h"

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
			"X*T\n"
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
			result.maze.tigerStart.row == 0 &&
			result.maze.tigerStart.column == 2 &&
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
				"  Level01.txt  \n"
				"Level02.txt\n");

		if (!result.isSuccessful)
		{
			std::cerr << result.errorMessage << '\n';
			return false;
		}

		return result.levelPaths.size() == 2 &&
			result.levelPaths[0] == "Level01.txt" &&
			result.levelPaths[1] == "Level02.txt";
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
				"Level01.txt\n"
				"Level02.txt\n"
				"Level01.txt\n");

		return !result.isSuccessful &&
			Contains(result.errorMessage, "Duplicate level path") &&
			Contains(result.errorMessage, "line 3") &&
			Contains(result.errorMessage, "Level01.txt");
	}

	bool TestRejectsInvalidCharacter()
	{
		const MazeLoadResult result = LoadMazeFromText(
			"X?T\n"
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
			"X*T\n"
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
				"X*T\n"
				"...\n"
				"...\n");

		const MazeLoadResult missingTiger =
			LoadMazeFromText(
				"X*.\n"
				"...\n"
				"..P\n");

		const MazeLoadResult missingExit =
			LoadMazeFromText(
				"..T\n"
				"...\n"
				"..P\n");

		return !missingPlayer.isSuccessful &&
			Contains(missingPlayer.errorMessage, "'P'") &&
			!missingTiger.isSuccessful &&
			Contains(missingTiger.errorMessage, "'T'") &&
			!missingExit.isSuccessful &&
			Contains(missingExit.errorMessage, "'X'");
	}

	bool TestRejectsDuplicateMarker()
	{
		const MazeLoadResult result =
			LoadMazeFromText(
				"X*T\n"
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

	std::cout
		<< "Tests: 14, Failed: "
		<< failedTestCount
		<< '\n';

	return failedTestCount == 0 ? 0 : 1;
}