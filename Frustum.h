#pragma once
#include "main.h"

class Frustum
{
private:
	D3DXVECTOR3 m_vertices[8];
	D3DXPLANE m_planes[6];

public:
	Frustum();
	VOID Update(D3DXMATRIX* viewProjectionMatrix);
	BOOL IntersectsSphere(D3DXVECTOR3* center, FLOAT radius);
};

