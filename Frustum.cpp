#include "Frustum.h"

Frustum::Frustum()
{
	ZeroMemory(m_vertices, sizeof(D3DXVECTOR3) * 8);
	ZeroMemory(m_planes, sizeof(D3DXPLANE) * 6);
}

VOID Frustum::Update(D3DXMATRIX* viewProjectionMatrix)
{
	int i;
	D3DXMATRIX inverseMatrix;
	D3DXMatrixInverse(&inverseMatrix, NULL, viewProjectionMatrix);

	m_vertices[0].x = -1.0f;	m_vertices[0].y = -1.0f;	m_vertices[0].z = 0.0f;
	m_vertices[1].x = 1.0f;	m_vertices[1].y = -1.0f;	m_vertices[1].z = 0.0f;
	m_vertices[2].x = 1.0f;	m_vertices[2].y = -1.0f;	m_vertices[2].z = 1.0f;
	m_vertices[3].x = -1.0f;	m_vertices[3].y = -1.0f;	m_vertices[3].z = 1.0f;
	m_vertices[4].x = -1.0f;	m_vertices[4].y = 1.0f;	m_vertices[4].z = 0.0f;
	m_vertices[5].x = 1.0f;	m_vertices[5].y = 1.0f;	m_vertices[5].z = 0.0f;
	m_vertices[6].x = 1.0f;	m_vertices[6].y = 1.0f;	m_vertices[6].z = 1.0f;
	m_vertices[7].x = -1.0f;	m_vertices[7].y = 1.0f;	m_vertices[7].z = 1.0f;

	for (i = 0; i < 8; i++)
	{
		D3DXVec3TransformCoord(&m_vertices[i], &m_vertices[i], &inverseMatrix);
	}
	// D3DXPlaneFromPoints는 왼손 좌표계임
	D3DXPlaneFromPoints(&m_planes[0], &m_vertices[7], &m_vertices[3], &m_vertices[0]); // 좌
	D3DXPlaneFromPoints(&m_planes[1], &m_vertices[6], &m_vertices[5], &m_vertices[1]); // 우
	D3DXPlaneFromPoints(&m_planes[2], &m_vertices[4], &m_vertices[5], &m_vertices[6]); // 상
	D3DXPlaneFromPoints(&m_planes[3], &m_vertices[0], &m_vertices[3], &m_vertices[2]); // 하
	D3DXPlaneFromPoints(&m_planes[4], &m_vertices[0], &m_vertices[1], &m_vertices[5]); // 근
	D3DXPlaneFromPoints(&m_planes[5], &m_vertices[2], &m_vertices[7], &m_vertices[6]); // 원

	for (int i = 0; i < 6; i++)
	{
		D3DXPlaneNormalize(&m_planes[i], &m_planes[i]);
	}
}

BOOL Frustum::IntersectsAabb(const D3DXVECTOR3* center, const D3DXVECTOR3* halfExtents) const
{
	if (center == nullptr || halfExtents == nullptr)
		return FALSE;

	for (int i = 0; i < 6; i++)
	{
		const FLOAT centerDistance = D3DXPlaneDotCoord(&m_planes[i], center);

		const FLOAT projectedRadius =
			fabsf(m_planes[i].a) * halfExtents->x +
			fabsf(m_planes[i].b) * halfExtents->y +
			fabsf(m_planes[i].c) * halfExtents->z;

		if (centerDistance < -projectedRadius)
			return FALSE;
	}

	return TRUE;
}