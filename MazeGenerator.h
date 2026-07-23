#pragma once
#include "Player.h"
#include "Exit.h"
#include "SettingsOverlay.h"

constexpr int kVerticesPerWallFace = 4;
constexpr int kWallBlockFaceCount = 5;
constexpr int kWallBlockVertexCount = kVerticesPerWallFace * kWallBlockFaceCount;

D3DXVECTOR3 CalculateMidPoint(D3DXVECTOR3 firstPoint, D3DXVECTOR3 secondPoint);
int GenerateMazeWalls(
	const char (*map)[kMazeColumnCount + 1],
	CustomVertex(*mazeVertices)[kWallBlockVertexCount]);
VOID InitializeMazeEntities(
	const char (*map)[kMazeColumnCount + 1],
	vector<Notice>* notices,
	Exit* exit);
VOID GenerateWallBlock(CustomVertex* blockVertices, D3DXVECTOR3 position);