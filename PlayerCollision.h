#pragma once

#include "main.h"

D3DXVECTOR3 ResolvePlayerMazeCollision(
	const char (*map)[kMazeColumnCount + 1],
	const D3DXVECTOR3& currentPosition,
	D3DXVECTOR3 targetPosition,
	const D3DXVECTOR3& movementDirection);