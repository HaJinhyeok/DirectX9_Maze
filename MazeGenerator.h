#pragma once
#include "Player.h"
#include "Exit.h"
#include "SettingsOverlay.h"

D3DXVECTOR3 CalculateMidPoint(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2);
VOID GenerateMazeWalls(int nMapNumber, CustomVertex (*Maze)[20], vector<Notice>* notice, Exit* exit);
VOID GenerateWallBlock(CustomVertex* Block, D3DXVECTOR3 position);