#pragma once
#include "Player.h"
#include "Exit.h"
#include "SettingsOverlay.h"

D3DXVECTOR3 CalculateMidPoint(D3DXVECTOR3 firstPoint, D3DXVECTOR3 secondPoint);
VOID GenerateMazeWalls(int mapNumber, CustomVertex (*mazeVertices)[20], vector<Notice>* notices, Exit* exit);
VOID GenerateWallBlock(CustomVertex* blockVertices, D3DXVECTOR3 position);