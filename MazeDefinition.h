#pragma once

#include <string>
#include <vector>

struct MazeCellPosition
{
	int row = 0;
	int column = 0;
};

struct MazeDefinition
{
	std::vector<std::string> cells;
	MazeCellPosition playerStart;
	MazeCellPosition tigerStart;
	MazeCellPosition exit;
	std::vector<MazeCellPosition> notices;

	int GetWidth() const noexcept
	{
		return cells.empty()
			? 0
			: static_cast<int>(cells.front().size());
	}

	int GetHeight() const noexcept
	{
		return static_cast<int>(cells.size());
	}

	bool IsInside(int row, int column) const noexcept
	{
		return row >= 0 &&
			row < GetHeight() &&
			column >= 0 &&
			column < static_cast<int>(cells[row].size());
	}

	char GetCell(int row, int column) const noexcept
	{
		return IsInside(row, column)
			? cells[row][column]
			: '*';
	}
};