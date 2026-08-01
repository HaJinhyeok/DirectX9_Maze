#include "Tiger.h"
#include "MazeCoordinates.h"

#include <cmath>

Tiger::Tiger(D3DXVECTOR3 position)
{
	ResetForLevel(position);
}

Tiger::~Tiger() = default;

VOID Tiger::ReleaseResources()
{
	m_model.ReleaseResources();
}

HRESULT Tiger::Load(LPDIRECT3DDEVICE9 device, char* xFilePath)
{
	return m_model.Load(device, xFilePath);
}

int Tiger::Render(LPDIRECT3DDEVICE9 device)
{
	if (!IsAlive())
	{
		return 0;
	}

	return m_model.Render(device);
}

VOID Tiger::Move(
	const MazeDefinition& maze,
	const MazeDistanceField& distanceField,
	FLOAT deltaTimeSeconds)
{
	if (!IsAlive() || deltaTimeSeconds <= 0.0f)
	{
		return;
	}

	const D3DXVECTOR3 currentPosition = GetPosition();

	const MazeCellPosition currentCell =
		CalculateMazeCellPosition(
			maze,
			MazeWorldPosition
			{
				currentPosition.x,
				currentPosition.y,
				currentPosition.z
			},
			kTileSize);

	if (m_moveTargetCell.row < 0 ||
		m_moveTargetCell.column < 0)
	{
		if (!TryGetNextMazeCell(
			distanceField,
			currentCell,
			m_moveTargetCell))
		{
			return;
		}
	}

	const MazeWorldPosition targetWorldPosition =
		CalculateMazeCellCenterPosition(
			maze,
			m_moveTargetCell.row,
			m_moveTargetCell.column,
			kTileSize);

	const D3DXVECTOR3 targetPosition(
		targetWorldPosition.x,
		currentPosition.y,
		targetWorldPosition.z);

	D3DXVECTOR3 movementDirection = targetPosition - currentPosition;

	const FLOAT remainingDistance = D3DXVec3Length(&movementDirection);

	if (remainingDistance <= kEpsilon)
	{
		SetPosition(targetPosition);
		m_moveTargetCell = MazeCellPosition{ -1, -1 };
		return;
	}

	D3DXVec3Normalize(&movementDirection, &movementDirection);

	const FLOAT movementDistance = kTigerMoveSpeed * deltaTimeSeconds;

	D3DXVECTOR3 nextPosition;

	if (movementDistance >= remainingDistance)
	{
		nextPosition = targetPosition;
		m_moveTargetCell = MazeCellPosition{ -1, -1 };
	}
	else
	{
		nextPosition =
			currentPosition +
			movementDirection * movementDistance;
	}

	const FLOAT yaw = static_cast<FLOAT>(
		std::atan2(
			-movementDirection.x,
			-movementDirection.z));

	D3DXMATRIX scalingMatrix;
	D3DXMATRIX rotationMatrix;
	D3DXMATRIX translationMatrix;
	D3DXMATRIX scaledRotationMatrix;

	D3DXMatrixScaling(
		&scalingMatrix,
		kTigerScale,
		kTigerScale,
		kTigerScale * 2.0f / 3.0f);

	D3DXMatrixRotationY(
		&rotationMatrix,
		yaw);

	D3DXMatrixTranslation(
		&translationMatrix,
		nextPosition.x,
		nextPosition.y,
		nextPosition.z);

	D3DXMatrixMultiply(
		&scaledRotationMatrix,
		&scalingMatrix,
		&rotationMatrix);

	D3DXMatrixMultiply(
		&m_worldMatrix,
		&scaledRotationMatrix,
		&translationMatrix);
}

VOID Tiger::ResetForLevel(const D3DXVECTOR3& position)
{
	// scale 먼저 하고 translation
	D3DXMatrixScaling(
		&m_worldMatrix,
		kTigerScale,
		kTigerScale,
		kTigerScale * 2.0f / 3.0f);

	D3DXMATRIX translationMatrix;
	D3DXMatrixTranslation(
		&translationMatrix,
		position.x,
		position.y,
		position.z);

	D3DXMatrixMultiply(
		&m_worldMatrix,
		&m_worldMatrix,
		&translationMatrix);

	m_health = kTigerMaxHealth;
	m_moveTargetCell = MazeCellPosition{ -1, -1 };
}

void Tiger::TakeDamage(int damage) noexcept
{
	if (damage <= 0 || !IsAlive())
	{
		return;
	}

	m_health -= damage;

	if (m_health < 0)
	{
		m_health = 0;
	}
}

int Tiger::GetHealth() const noexcept
{
	return m_health;
}

bool Tiger::IsAlive() const noexcept
{
	return m_health > 0;
}

CollisionSphere Tiger::GetCollisionSphere() const noexcept
{
	return
	{
		{
			m_worldMatrix._41,
			m_worldMatrix._42,
			m_worldMatrix._43
		},
		kTigerCollisionRadius
	};
}