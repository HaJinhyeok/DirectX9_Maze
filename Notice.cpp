#include "Notice.h"
#include "ComUtils.h"

WORD Notice::s_noticeCount = 0;

VOID Notice::Initialize(D3DXVECTOR3 position)
{
	D3DXMatrixIdentity(&m_worldMatrix);
	m_position = position;
	for (int i = 0; i < 4; i++)
	{
		m_vertices[i].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	}
	m_vertices[0].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
	m_vertices[1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
	m_vertices[2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
	m_vertices[3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);

	m_vertices[0].position = D3DXVECTOR3(m_position.x - kTileSize / 4, m_position.y + kTileSize / 4, m_position.z);
	m_vertices[1].position = D3DXVECTOR3(m_position.x + kTileSize / 4, m_position.y + kTileSize / 4, m_position.z);
	m_vertices[2].position = D3DXVECTOR3(m_position.x + kTileSize / 4, m_position.y - kTileSize / 4, m_position.z);
	m_vertices[3].position = D3DXVECTOR3(m_position.x - kTileSize / 4, m_position.y - kTileSize / 4, m_position.z);

	m_lookAt = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	s_noticeCount++;
}

VOID Notice::CreateVertexBuffer(LPDIRECT3DDEVICE9 device)
{
	device->CreateVertexBuffer(sizeof(CustomVertex) * 4, 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_noticeVertexBuffer, NULL);
	VOID** noticeVertices;
	m_noticeVertexBuffer->Lock(0, sizeof(CustomVertex) * 4, (void**)&noticeVertices, 0);
	memcpy(noticeVertices, m_vertices, sizeof(CustomVertex) * 4);
	m_noticeVertexBuffer->Unlock();
}

VOID Notice::UpdateFacing(D3DXVECTOR3 playerPosition)
{
	D3DXVECTOR3 crossProduct, targetDirection = playerPosition - m_position;
	D3DXMATRIX rotationMatrix, translationMatrix;
	targetDirection.y = 0.0f;
	// player가 움직이지 않았으면 회전x
	if (m_lookAt == targetDirection) return;

	FLOAT angle, cosine, targetLength, currentLength;
	// 원점으로 옮기기
	D3DXMatrixTranslation(&translationMatrix, -m_position.x, -m_position.y, -m_position.z);
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);
	// Calculate angle between current LookAt and goal LookAt, using dot product
	targetLength = sqrtf(targetDirection.x * targetDirection.x + targetDirection.z * targetDirection.z);
	currentLength = sqrtf(m_lookAt.x * m_lookAt.x + m_lookAt.z * m_lookAt.z);
	cosine = D3DXVec3Dot(&targetDirection, &m_lookAt) / (targetLength * currentLength);
	cosine = min(1.0f, max(-1.0f, cosine));
	angle = acosf(cosine);
	D3DXVec3Cross(&crossProduct, &m_lookAt, &targetDirection);
	// Update LookAt vector
	m_lookAt = targetDirection;
	// If the result of cross product heads to +y, +angle
	if (crossProduct.y > 0)
	{
		D3DXMatrixRotationY(&rotationMatrix, angle);
	}
	// If the result of cross product heads to -y, -angle
	else
	{
		D3DXMatrixRotationY(&rotationMatrix, -angle);
	}
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &rotationMatrix);
	// 제자리로 복귀
	D3DXMatrixTranslation(&translationMatrix, m_position.x, m_position.y, m_position.z);
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);
}

VOID Notice::Render(LPDIRECT3DDEVICE9 device)
{
	device->SetStreamSource(0, m_noticeVertexBuffer, 0, sizeof(CustomVertex));
	device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
}

VOID Notice::ReleaseVertexBuffer()
{
	SafeRelease(m_noticeVertexBuffer);
}

BOOL Notice::CanInteract(D3DXVECTOR3 playerPosition, BOOL isNoClipEnabled)
{
	if (isNoClipEnabled)
		return FALSE;
	// 충돌을 검사할 블록의 왼쪽아래(minX, minZ)와 오른쪽위(maxX,maxZ) 두 점
	D3DXVECTOR2 noticeBounds[2];
	noticeBounds[0].x = m_position.x - kTileSize / 2;
	noticeBounds[0].y = m_position.z - kTileSize / 2;
	noticeBounds[1].x = m_position.x + kTileSize / 2;
	noticeBounds[1].y = m_position.z + kTileSize / 2;

	//충돌 시
	if (noticeBounds[0].x <= playerPosition.x + kPlayerRadius && noticeBounds[1].x >= playerPosition.x - kPlayerRadius
		&& noticeBounds[0].y <= playerPosition.z + kPlayerRadius && noticeBounds[1].y >= playerPosition.z - kPlayerRadius)
	{
		return TRUE;
	}
	// 충돌을 벗어나면 다시 원래 시점으로 복구
	else
	{
		return FALSE;
	}
}
