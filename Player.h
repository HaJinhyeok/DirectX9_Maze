#pragma once
#include <d3d9.h>
#include <d3dx9math.h>

#include "main.h"

class Player
{
private:
	D3DXVECTOR3 m_lookAt;
	D3DXMATRIX m_worldMatrix; // x, y, z축 정보 + position 정보
	BOOL m_isFlashlightOn;
	D3DLIGHT9 m_flashlight;
	deque<Bullet> m_bullets;
	const FLOAT m_bulletVelocity = 100.0f;	// 초당 이동 거리
	// 플레이어가 발사한 총알 객체를 하나하나 다 갖고 있는게 아니라,
	// 위치값만 갖고 있다가 매 프레임마다 위치에 총알 그림 그려주면 되지 않을까?
	// 위치벡터만 배열이든 STL 큐나 벡터든 쭉 갖고있으면 될듯
	// + 날아가는 방향 벡터
	// 총알 속도와 RPM(Rounds Per Minute)은 player 객체가 가지고 있어야함
	// 매 프레임마다 방향벡터 활용해서 각 총알의 예상 이동 위치를 구하고, 그 위치가 벽과 접촉하면 총알 객체 사라지게 만들자

public:
	Player();

	VOID SetPosition(D3DXVECTOR3 position)
	{
		m_worldMatrix._41 = position.x;
		m_worldMatrix._42 = position.y;
		m_worldMatrix._43 = position.z;
	}
	VOID SetLookAt(D3DXVECTOR3 lookAt)
	{
		m_lookAt = lookAt;
	}
	VOID SetWorldMatrix(D3DXMATRIX worldMatrix)
	{
		m_worldMatrix = worldMatrix;
	}
	VOID SetFlashlight(BOOL isEnabled)
	{
		m_isFlashlightOn = isEnabled;
	}
	D3DXVECTOR3 GetPosition()
	{
		return D3DXVECTOR3(m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43);
	}
	D3DXVECTOR3 GetLookAt()
	{
		return m_lookAt;
	}
	D3DXMATRIX GetWorldMatrix()
	{
		return m_worldMatrix;
	}
	BOOL IsFlashlightOn()
	{
		return m_isFlashlightOn;
	}
	D3DLIGHT9* GetLight()
	{
		return &m_flashlight;
	}

	BOOL Move(
		MoveDirection direction,
		const char (*map)[kMazeColumnCount + 1],
		BOOL isNoClipEnabled,
		FLOAT deltaTimeSeconds);
	VOID Rotate(BOOL isCounterClockwise, FLOAT deltaTimeSeconds);
	VOID Rotate(BOOL isCounterClockwise, BOOL isVertical, FLOAT angle);
	VOID Jump();
	VOID FireBullet(LPPOINT cursorPosition);
	VOID UpdateBullets(FLOAT deltaTimeSeconds);
	VOID RenderBullets(LPDIRECT3DDEVICE9 device, LPD3DXMESH sphere);
};

