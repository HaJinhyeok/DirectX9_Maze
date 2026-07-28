#include "../MazeLoader.h"
#include "../MazeCoordinates.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
	constexpr char kTestMazePath[] = "MazeLoaderTestInput.txt";

	bool WriteTestMazeFile(const std::string& contents)
	{
		std::ofstream outputFile(kTestMazePath, std::ios::trunc);

		if (!outputFile.is_open())
			return false;

		outputFile << contents;

		return outputFile.good();
	}

	MazeLoadResult LoadMazeFromText(const std::string& contents)
	{
		if (!WriteTestMazeFile(contents))
		{
			MazeLoadResult result;
			result.errorMessage = "Failed to create test maze file.";

			return result;
		}

		MazeLoadResult result = LoadMazeFromFile(kTestMazePath);

		std::remove(kTestMazePath);

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

	std::cout
		<< "Tests: 8, Failed: "
		<< failedTestCount
		<< '\n';

	return failedTestCount == 0 ? 0 : 1;
}