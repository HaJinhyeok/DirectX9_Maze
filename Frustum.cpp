#include "Frustum.h"

Frustum::Frustum()
{
	ZeroMemory(m_vertex, sizeof(D3DXVECTOR3) * 8);
	ZeroMemory(m_plane, sizeof(D3DXPLANE) * 6);
}
VOID Frustum::Update(D3DXMATRIX* pMatViewProj)
{
	int i;
	D3DXMATRIX matInv;
	D3DXMatrixInverse(&matInv, NULL, pMatViewProj);

	m_vertex[0].x = -1.0f;	m_vertex[0].y = -1.0f;	m_vertex[0].z = 0.0f;
	m_vertex[1].x = 1.0f;	m_vertex[1].y = -1.0f;	m_vertex[1].z = 0.0f;
	m_vertex[2].x = 1.0f;	m_vertex[2].y = -1.0f;	m_vertex[2].z = 1.0f;
	m_vertex[3].x = -1.0f;	m_vertex[3].y = -1.0f;	m_vertex[3].z = 1.0f;
	m_vertex[4].x = -1.0f;	m_vertex[4].y = 1.0f;	m_vertex[4].z = 0.0f;
	m_vertex[5].x = 1.0f;	m_vertex[5].y = 1.0f;	m_vertex[5].z = 0.0f;
	m_vertex[6].x = 1.0f;	m_vertex[6].y = 1.0f;	m_vertex[6].z = 1.0f;
	m_vertex[7].x = -1.0f;	m_vertex[7].y = 1.0f;	m_vertex[7].z = 1.0f;

	for (i = 0; i < 8; i++)
	{
		D3DXVec3TransformCoord(&m_vertex[i], &m_vertex[i], &matInv);
	}
	// D3DXPlaneFromPoints는 왼손 좌표계임
	D3DXPlaneFromPoints(&m_plane[0], &m_vertex[7], &m_vertex[3], &m_vertex[0]); // 좌
	D3DXPlaneFromPoints(&m_plane[1], &m_vertex[6], &m_vertex[5], &m_vertex[1]); // 우
	D3DXPlaneFromPoints(&m_plane[2], &m_vertex[4], &m_vertex[5], &m_vertex[6]); // 상
	D3DXPlaneFromPoints(&m_plane[3], &m_vertex[0], &m_vertex[3], &m_vertex[2]); // 하
	D3DXPlaneFromPoints(&m_plane[4], &m_vertex[0], &m_vertex[1], &m_vertex[5]); // 근
	D3DXPlaneFromPoints(&m_plane[5], &m_vertex[2], &m_vertex[7], &m_vertex[6]); // 원
}
BOOL Frustum::IntersectsSphere(D3DXVECTOR3* position, FLOAT distance)
{
	FLOAT fDistance;
	int i;
	for (i = 0; i < 6; i++)
	{
		fDistance = 0.0f;
		fDistance += m_plane[i].a * position->x + m_plane[i].b * position->y + m_plane[i].c * position->z + m_plane[i].d;
		fDistance /= sqrtf(m_plane[i].a * m_plane[i].a + m_plane[i].b * m_plane[i].b + m_plane[i].c * m_plane[i].c);
		if (fDistance < -distance)
			return FALSE;
	}
	return TRUE;
}