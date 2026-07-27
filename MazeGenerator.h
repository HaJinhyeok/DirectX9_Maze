#pragma once

#include <array>
#include <vector>

#include "Player.h"
#include "Exit.h"
#include "SettingsOverlay.h"
#include "MazeDefinition.h"

constexpr int kVerticesPerWallFace = 4;
constexpr int kWallBlockFaceCount = 5;
constexpr int kWallBlockVertexCount = kVerticesPerWallFace * kWallBlockFaceCount;

using MazeWallBlockVertices = std::array<CustomVertex, kWallBlockVertexCount>;

D3DXVECTOR3 CalculateMidPoint(D3DXVECTOR3 firstPoint, D3DXVECTOR3 secondPoint);
D3DXVECTOR3 CalculateAabbHalfExtents(D3DXVECTOR3 firstPoint, D3DXVECTOR3 secondPoint);
D3DXVECTOR3 CalculateMazeCellCenter(const MazeDefinition& maze, int row, int column);

std::vector<MazeWallBlockVertices> GenerateMazeWalls(const MazeDefinition& maze);

VOID InitializeMazeEntities(
	const MazeDefinition& maze,
	vector<Notice>* notices,
	Exit* exit);

VOID GenerateWallBlock(CustomVertex* blockVertices, D3DXVECTOR3 position);