#include "CNotice.h"

WORD CNotice::m_NoticeCount = 0;

VOID CNotice::MakeNotice(D3DXVECTOR3 position)
{
	D3DXMatrixIdentity(&m_World);
	m_Position = position;
	for (int i = 0; i < 4; i++)
	{
		m_Vertex[i].v3VerNormal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	}
	m_Vertex[0].v2VerTex = D3DXVECTOR2(0.0f, 0.0f);
	m_Vertex[1].v2VerTex = D3DXVECTOR2(1.0f, 0.0f);
	m_Vertex[2].v2VerTex = D3DXVECTOR2(1.0f, 1.0f);
	m_Vertex[3].v2VerTex = D3DXVECTOR2(0.0f, 1.0f);

	m_Vertex[0].v3VerPos = D3DXVECTOR3(m_Position.x - LENGTH_OF_TILE / 4, m_Position.y + LENGTH_OF_TILE / 4, m_Position.z);
	m_Vertex[1].v3VerPos = D3DXVECTOR3(m_Position.x + LENGTH_OF_TILE / 4, m_Position.y + LENGTH_OF_TILE / 4, m_Position.z);
	m_Vertex[2].v3VerPos = D3DXVECTOR3(m_Position.x + LENGTH_OF_TILE / 4, m_Position.y - LENGTH_OF_TILE / 4, m_Position.z);
	m_Vertex[3].v3VerPos = D3DXVECTOR3(m_Position.x - LENGTH_OF_TILE / 4, m_Position.y - LENGTH_OF_TILE / 4, m_Position.z);

	m_LookAt = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	m_NoticeCount++;
}
VOID CNotice::MakeNoticeVB(LPDIRECT3DDEVICE9 device)
{
	device->CreateVertexBuffer(sizeof(CUSTOMVERTEX) * 4, 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pNoticeVB, NULL);
	VOID** NoticeVertices;
	m_pNoticeVB->Lock(0, sizeof(CUSTOMVERTEX) * 4, (void**)&NoticeVertices, 0);
	memcpy(NoticeVertices, m_Vertex, sizeof(CUSTOMVERTEX) * 4);
	m_pNoticeVB->Unlock();
}
VOID CNotice::RotateNotice(D3DXVECTOR3 player_position)
{
	D3DXVECTOR3 v3Vertices[4], v3Cross, v3Dst = player_position - m_Position;
	D3DXMATRIX mtRotation, mtTranslation;
	v3Dst.y = 0.0f;
	// player가 움직이지 않았으면 회전x
	if (m_LookAt == v3Dst) return;

	FLOAT angle, cos, fDst, fCurrent;
	// 원점으로 옮기기
	D3DXMatrixTranslation(&mtTranslation, -m_Position.x, -m_Position.y, -m_Position.z);
	D3DXMatrixMultiply(&m_World, &m_World, &mtTranslation);
	// Calculate angle between current LookAt and goal LookAt, using dot product
	fDst = sqrtf(v3Dst.x * v3Dst.x + v3Dst.z * v3Dst.z);
	fCurrent = sqrtf(m_LookAt.x * m_LookAt.x + m_LookAt.z * m_LookAt.z);
	cos = D3DXVec3Dot(&v3Dst, &m_LookAt) / (fDst * fCurrent);
	cos = min(1.0f, max(-1.0f, cos));
	angle = acosf(cos);
	D3DXVec3Cross(&v3Cross, &m_LookAt, &v3Dst);
	// Update LookAt vector
	m_LookAt = v3Dst;
	// If the result of cross product heads to +y, +angle
	if (v3Cross.y > 0)
	{
		D3DXMatrixRotationY(&mtRotation, angle);
	}
	// If the result of cross product heads to -y, -angle
	else
	{
		D3DXMatrixRotationY(&mtRotation, -angle);
	}
	D3DXMatrixMultiply(&m_World, &m_World, &mtRotation);
	// 제자리로 복귀
	D3DXMatrixTranslation(&mtTranslation, m_Position.x, m_Position.y, m_Position.z);
	D3DXMatrixMultiply(&m_World, &m_World, &mtTranslation);
}
VOID CNotice::DrawNotice(LPDIRECT3DDEVICE9 device)
{
	device->SetStreamSource(0, m_pNoticeVB, 0, sizeof(CUSTOMVERTEX));
	device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
}
VOID CNotice::ReleaseNoticeVB()
{
	if (m_pNoticeVB != NULL)
		m_pNoticeVB->Release();
}
BOOL CNotice::IsPossibleInteraction(D3DXVECTOR3 playerPosition)
{
	if (bIsNoClipOn)
		return FALSE;
	// 충돌을 검사할 블록의 왼쪽아래(minX, minZ)와 오른쪽위(maxX,maxZ) 두 점
	D3DXVECTOR2 NoticePoint[2];
	NoticePoint[0].x = m_Position.x - LENGTH_OF_TILE / 2;
	NoticePoint[0].y = m_Position.z - LENGTH_OF_TILE / 2;
	NoticePoint[1].x = m_Position.x + LENGTH_OF_TILE / 2;
	NoticePoint[1].y = m_Position.z + LENGTH_OF_TILE / 2;

	//충돌 시
	if (NoticePoint[0].x <= playerPosition.x + PLAYER_RADIUS && NoticePoint[1].x >= playerPosition.x - PLAYER_RADIUS
		&& NoticePoint[0].y <= playerPosition.z + PLAYER_RADIUS && NoticePoint[1].y >= playerPosition.z - PLAYER_RADIUS)
	{
		return TRUE;
	}
	// 충돌을 벗어나면 다시 원래 시점으로 복구
	else
	{
		return FALSE;
	}
}
