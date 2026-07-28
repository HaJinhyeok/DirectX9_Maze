#include "Notice.h"
#include "ComUtils.h"

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
}

HRESULT Notice::CreateVertexBuffer(LPDIRECT3DDEVICE9 device)
{
	return CreateManagedVertexBuffer(
		device,
		m_vertices,
		sizeof(m_vertices),
		D3DFVF_CUSTOMVERTEX,
		&m_noticeVertexBuffer
	);
}

VOID Notice::UpdateFacing(D3DXVECTOR3 playerPosition)
{
	const D3DXVECTOR3 baseFacing(0.0f, 0.0f, -1.0f);
	D3DXVECTOR3 targetDirection = playerPosition - m_position;
	targetDirection.y = 0.0f;

	const FLOAT targetLength = sqrtf(
		targetDirection.x * targetDirection.x +
		targetDirection.z * targetDirection.z);

	if (targetLength <= kEpsilon)
		return;

	if (m_lookAt == targetDirection)
		return;

	FLOAT cosine = D3DXVec3Dot(&targetDirection, &baseFacing) / targetLength;
	cosine = min(1.0f, max(-1.0f, cosine));

	const FLOAT angle = acosf(cosine);

	D3DXVECTOR3 crossProduct;
	D3DXVec3Cross(
		&crossProduct,
		&baseFacing,
		&targetDirection);

	D3DXMATRIX translationMatrix;
	D3DXMATRIX rotationMatrix;

	D3DXMatrixIdentity(&m_worldMatrix);

	D3DXMatrixTranslation(
		&translationMatrix,
		-m_position.x,
		-m_position.y,
		-m_position.z);

	D3DXMatrixMultiply(
		&m_worldMatrix,
		&m_worldMatrix,
		&translationMatrix);

	D3DXMatrixRotationY(
		&rotationMatrix,
		crossProduct.y > 0.0f ? angle : -angle);

	D3DXMatrixMultiply(
		&m_worldMatrix,
		&m_worldMatrix,
		&rotationMatrix);

	D3DXMatrixTranslation(
		&translationMatrix,
		m_position.x,
		m_position.y,
		m_position.z);

	D3DXMatrixMultiply(
		&m_worldMatrix,
		&m_worldMatrix,
		&translationMatrix);

	m_lookAt = targetDirection;
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
