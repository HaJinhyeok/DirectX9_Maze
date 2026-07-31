#pragma once

#include <d3dx9.h>
#include <memory>
#include <vector>

#include "main.h"
#include "XFileModel.h"
#include "MazeDefinition.h"
#include "CombatCollision.h"

constexpr float kTigerMoveDistance = 0.2f;
constexpr float kTigerScale = 7.0f;
constexpr float kTigerUpdateIntervalSeconds = 0.01f;
constexpr int kTigerMaxHealth = 3;
constexpr float kTigerCollisionRadius = 3.5f;

class Tiger
{
private:
	XFileModel m_model;

	D3DXVECTOR3 m_lookAt;
	D3DXMATRIX m_worldMatrix;
	int m_health;
	BOOL m_isRotating; // 현재 호랑이가 방향전환 중인지 확인
	BOOL m_isClockwise;
	BOOL m_isWallOpen[4]; // 현재 호랑이 위치 기준 앞, 뒤, 좌, 우로 진행 가능한지 확인
	DWORD m_rotationAmount, m_rotationCount;
	FLOAT m_accumulatedTimeSeconds;

	random_device m_randomDevice; // 시드값을 얻기 위한 random_device

public:
	int Render(LPDIRECT3DDEVICE9 device);
	HRESULT Load(LPDIRECT3DDEVICE9 device, char* xFilePath);
	VOID Move(const MazeDefinition& maze, FLOAT deltaTimeSeconds);
	VOID Rotate(BOOL clockwise);

	VOID SetPosition(const D3DXVECTOR3& position)
	{
		m_worldMatrix._41 = position.x;
		m_worldMatrix._42 = position.y;
		m_worldMatrix._43 = position.z;
	}

	VOID SetLookAt(D3DXVECTOR3 lookAt)
	{
		m_lookAt = lookAt;
	}

	D3DXVECTOR3 GetPosition() const
	{
		return D3DXVECTOR3(
			m_worldMatrix._41,
			m_worldMatrix._42,
			m_worldMatrix._43);
	}

	D3DXVECTOR3 GetLookAt()
	{
		return m_lookAt;
	}

	D3DXMATRIX& GetWorldMatrix()
	{
		return m_worldMatrix;
	}

	Tiger(D3DXVECTOR3 position);
	virtual ~Tiger();
	VOID ReleaseResources();
	VOID ResetForLevel(const D3DXVECTOR3& position, const D3DXVECTOR3& lookAt);

	void TakeDamage(int damage) noexcept;
	int GetHealth() const noexcept;
	bool IsAlive() const noexcept;
	CollisionSphere GetCollisionSphere() const noexcept;
};

using TigerCollection =
	std::vector<std::unique_ptr<Tiger>>;

