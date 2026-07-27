#pragma once

#include "main.h"
#include "MazeDefinition.h"

D3DXVECTOR3 ResolvePlayerMazeCollision(
	const MazeDefinition& maze,
	const D3DXVECTOR3& currentPosition,
	const D3DXVECTOR3& targetPosition);