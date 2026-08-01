#pragma once

#include <d3dx9.h>
#include <memory>
#include <vector>

#include "main.h"
#include "XFileModel.h"
#include "MazeDefinition.h"
#include "CombatCollision.h"
#include "MazePathfinding.h"

constexpr float kTigerScale = 7.0f;
constexpr int kTigerMaxHealth = 3;
constexpr float kTigerCollisionRadius = 3.5f;
constexpr float kTigerMoveSpeed = 20.0f;

class Tiger
{
private:
	XFileModel m_model;

	D3DXMATRIX m_worldMatrix;
	int m_health;

	MazeCellPosition m_moveTargetCell{ -1, -1 };

public:
	int Render(LPDIRECT3DDEVICE9 device);
	HRESULT Load(LPDIRECT3DDEVICE9 device, char* xFilePath);
	VOID Move(
		const MazeDefinition& maze,
		const MazeDistanceField& distanceField,
		FLOAT deltaTimeSeconds);

	VOID SetPosition(const D3DXVECTOR3& position)
	{
		m_worldMatrix._41 = position.x;
		m_worldMatrix._42 = position.y;
		m_worldMatrix._43 = position.z;
	}

	D3DXVECTOR3 GetPosition() const
	{
		return D3DXVECTOR3(
			m_worldMatrix._41,
			m_worldMatrix._42,
			m_worldMatrix._43);
	}

	D3DXMATRIX& GetWorldMatrix()
	{
		return m_worldMatrix;
	}

	Tiger(D3DXVECTOR3 position);
	virtual ~Tiger();
	VOID ReleaseResources();
	VOID ResetForLevel(const D3DXVECTOR3& position);

	void TakeDamage(int damage) noexcept;
	int GetHealth() const noexcept;
	bool IsAlive() const noexcept;
	CollisionSphere GetCollisionSphere() const noexcept;
};

using TigerCollection =
	std::vector<std::unique_ptr<Tiger>>;

