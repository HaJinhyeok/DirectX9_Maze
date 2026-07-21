#pragma once
#include "main.h"
class CFrustum
{
private:
	D3DXVECTOR3 m_vertex[8];
	D3DXPLANE m_plane[6];

public:
	CFrustum();
	VOID Update(D3DXMATRIX* pMatViewProj);
	BOOL IntersectsSphere(D3DXVECTOR3* position, FLOAT distance);
};

