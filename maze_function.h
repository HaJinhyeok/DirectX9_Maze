#pragma once
#include "CPlayer.h"
#include "CExit.h"
#include "CSetting.h"

D3DXVECTOR3 CalculateMidPoint(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2);
VOID GenerateMazeWall(int nMapNumber, CUSTOMVERTEX (*Maze)[20], vector<CNotice>* notice, CExit* exit);
VOID MakeWallBlock(CUSTOMVERTEX* Block, D3DXVECTOR3 position);