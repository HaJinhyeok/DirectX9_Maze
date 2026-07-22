#include "Player.h"

namespace
{
	bool IsWall(const char (*map)[kMazeColumnCount + 1], int row, int column)
	{
		if (row < 0 || row >= kMazeRowCount ||
			column < 0 || column >= kMazeColumnCount)
		{
			return false;
		}

		return map[row][column] == '*';
	}
}

Player::Player()
{
	// 일반화된 m by n matrix로 미로 정보를 받게 되면 수정해야함
	// 현재는 고정된 위치 시작
	m_lookAt = kPlayerStartPosition;
	m_lookAt.z += kLookAtDistance;
	// player world matrix initialization
	D3DXMatrixIdentity(&m_worldMatrix);
	D3DXMatrixTranslation(&m_worldMatrix, kPlayerStartPosition.x, kPlayerStartPosition.y, kPlayerStartPosition.z);

	//// light setting
	m_isFlashlightOn = TRUE;
	ZeroMemory(&m_flashlight, sizeof(D3DLIGHT9));
	m_flashlight.Type = D3DLIGHT_SPOT;
	m_flashlight.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_flashlight.Ambient.r = 1.0f;
	m_flashlight.Ambient.g = 1.0f;
	m_flashlight.Ambient.b = 1.0f;
	m_flashlight.Specular.r = 1.0f;
	m_flashlight.Specular.g = 1.0f;
	m_flashlight.Specular.b = 1.0f;
	m_flashlight.Position = kPlayerStartPosition;
	m_flashlight.Direction = D3DXVECTOR3(m_worldMatrix._31, m_worldMatrix._32, m_worldMatrix._33);
	D3DXVec3Normalize((D3DXVECTOR3*)&m_flashlight.Direction, (D3DXVECTOR3*)&m_flashlight.Direction);
	m_flashlight.Range = 1500.0f;
	m_flashlight.Attenuation0 = 1.0f;
	m_flashlight.Attenuation1 = 0.01f;
	m_flashlight.Attenuation2 = 0.0001f;
	m_flashlight.Falloff = 2.0f;
	m_flashlight.Phi = D3DXToRadian(60.0f);
	m_flashlight.Theta = D3DXToRadian(30.0f);
}

// PLAYER MOVE
// 바꾸어야 할 것: 플레이어 위치, 플레이어 LookAt
// 필요한 것: 플레이어가 움직일 방향 벡터, 거리, 플에이어 현 위치, 플레이어 현 LookAt, 맵 정보
BOOL Player::Move(MoveDirection direction, const char(*map)[kMazeColumnCount + 1], BOOL isNoClipEnabled, FLOAT deltaTimeSeconds)
{
	FLOAT movementScale = kPlayerMoveSpeed * deltaTimeSeconds;

	D3DXVECTOR3 movementDirection;
	D3DXVECTOR3 targetPosition; // 벽을 생각하지 않고 이동된 위치. 주변 8개 벽과 이것을 대조해 최종 위치 결정
	D3DXVECTOR3 currentPosition = GetPosition();
	int i, column, row;

	// y축 움직임은 없으므로 y축 정보는 걍 빼고 계산
	// => 자유시점의 경우도 생각해서 y축도 이동
	if (direction == MoveDirection::Left)
	{
		movementDirection.x = -m_worldMatrix._11;
		movementDirection.y = -m_worldMatrix._12;
		movementDirection.z = -m_worldMatrix._13;
		movementScale /= sqrtf(movementDirection.x * movementDirection.x + movementDirection.y * movementDirection.y + movementDirection.z * movementDirection.z);

		targetPosition.x = currentPosition.x + movementDirection.x * movementScale;
		targetPosition.y = currentPosition.y + movementDirection.y * movementScale;
		targetPosition.z = currentPosition.z + movementDirection.z * movementScale;
	}
	else if (direction == MoveDirection::Right)
	{
		movementDirection.x = m_worldMatrix._11;
		movementDirection.y = m_worldMatrix._12;
		movementDirection.z = m_worldMatrix._13;
		movementScale /= sqrtf(movementDirection.x * movementDirection.x + movementDirection.y * movementDirection.y + movementDirection.z * movementDirection.z);

		targetPosition.x = currentPosition.x + movementDirection.x * movementScale;
		targetPosition.y = currentPosition.y + movementDirection.y * movementScale;
		targetPosition.z = currentPosition.z + movementDirection.z * movementScale;
	}
	else if (direction == MoveDirection::Forward)
	{
		movementDirection.x = m_worldMatrix._31;
		movementDirection.y = m_worldMatrix._32;
		movementDirection.z = m_worldMatrix._33;
		movementScale /= sqrtf(movementDirection.x * movementDirection.x + movementDirection.y * movementDirection.y + movementDirection.z * movementDirection.z);

		targetPosition.x = currentPosition.x + movementDirection.x * movementScale;
		targetPosition.y = currentPosition.y + movementDirection.y * movementScale;
		targetPosition.z = currentPosition.z + movementDirection.z * movementScale;
	}
	else if (direction == MoveDirection::Backward)
	{
		movementDirection.x = -m_worldMatrix._31;
		movementDirection.y = -m_worldMatrix._32;
		movementDirection.z = -m_worldMatrix._33;
		movementScale /= sqrtf(movementDirection.x * movementDirection.x + movementDirection.y * movementDirection.y + movementDirection.z * movementDirection.z);

		targetPosition.x = currentPosition.x + movementDirection.x * movementScale;
		targetPosition.y = currentPosition.y + movementDirection.y * movementScale;
		targetPosition.z = currentPosition.z + movementDirection.z * movementScale;

	}
	if (!isNoClipEnabled)
	{
		// 충돌을 검사할 블록의 왼쪽아래(minX, minZ)와 오른쪽위(maxX,maxZ) 두 점
		D3DXVECTOR2 wallBounds[2];
		// 현재 좌표
		column = static_cast<int>(floorf(currentPosition.x / kTileSize)) + kMazeColumnCount / 2;
		row = kMazeRowCount / 2 - static_cast<int>(floorf(currentPosition.z / kTileSize)) - 1;
		// 해결: 외곽 벽에 부딪히는 경우 추가, 몇몇 곳에서 블록 속으로 들어가버리는 버그 수정하기
		// x축 음의 방향으로 이동일 경우
		if (movementDirection.x < 0)
		{
			for (i = 0; i < 3; i++)
			{
				// 현재 위치 기준 왼쪽 3개 블록에 대해 검사
				// 제일 외곽 벽일 경우 따로 검사
				if (column == 0)
				{
					if (targetPosition.x - kPlayerRadius <= -kMazeColumnCount / 2 * kTileSize)
					{
						targetPosition.x = -kMazeColumnCount / 2 * kTileSize + kPlayerRadius;
					}
				}
				else if (IsWall(map, row - 1 + i, column - 1))
				{
					wallBounds[0].x = (column - 1 - kMazeColumnCount / 2) * kTileSize;
					wallBounds[0].y = (kMazeRowCount / 2 - (row - 1 + i) - 1) * kTileSize;
					wallBounds[1].x = (column - kMazeColumnCount / 2) * kTileSize;
					wallBounds[1].y = (kMazeRowCount / 2 - (row - 1 + i)) * kTileSize;

					//충돌 시
					if (wallBounds[0].x <= targetPosition.x + kPlayerRadius && wallBounds[1].x >= targetPosition.x - kPlayerRadius
						&& wallBounds[0].y <= targetPosition.z + kPlayerRadius && wallBounds[1].y >= targetPosition.z - kPlayerRadius)
					{
						if (IsWall(map, row, column - 1))
							targetPosition.x = wallBounds[1].x + kPlayerRadius + 0.1f;
						break;
					}
				}
			}
		}
		// x축 양의 방향으로 이동일 경우
		else if (movementDirection.x > 0)
		{
			for (i = 0; i < 3; i++)
			{
				// 현재 위치 기준 오른쪽 3개 블록에 대해 검사
				// 제일 외곽 벽일 경우 따로 검사
				if (column == kMazeColumnCount - 1)
				{
					if (targetPosition.x + kPlayerRadius >= kMazeColumnCount / 2 * kTileSize)
					{
						targetPosition.x = kMazeColumnCount / 2 * kTileSize - kPlayerRadius;
					}
				}
				else if (IsWall(map, row - 1 + i, column + 1))
				{
					wallBounds[0].x = (column + 1 - kMazeColumnCount / 2) * kTileSize;
					wallBounds[0].y = (kMazeRowCount / 2 - (row - 1 + i) - 1) * kTileSize;
					wallBounds[1].x = (column + 2 - kMazeColumnCount / 2) * kTileSize;
					wallBounds[1].y = (kMazeRowCount / 2 - (row - 1 + i)) * kTileSize;

					//충돌 시
					if (wallBounds[0].x <= targetPosition.x + kPlayerRadius && wallBounds[1].x >= targetPosition.x - kPlayerRadius
						&& wallBounds[0].y <= targetPosition.z + kPlayerRadius && wallBounds[1].y >= targetPosition.z - kPlayerRadius)
					{
						if (IsWall(map, row, column + 1))
							targetPosition.x = wallBounds[0].x - kPlayerRadius - 0.1f;
						break;
					}
				}
			}
		}

		// z축 음의 방향으로 이동일 경우
		if (movementDirection.z < 0)
		{
			for (i = 0; i < 3; i++)
			{
				// 현재 위치 기준 아래쪽 3개 블록에 대해 검사
				// 제일 외곽 벽일 경우 따로 검사
				if (row == kMazeRowCount - 1)
				{
					if (targetPosition.z - kPlayerRadius <= -kMazeRowCount / 2 * kTileSize)
					{
						targetPosition.z = -kMazeRowCount / 2 * kTileSize + kPlayerRadius;
					}
				}
				else if (IsWall(map, row + 1, column - 1 + i))
				{
					wallBounds[0].x = (column - 1 + i - kMazeColumnCount / 2) * kTileSize;
					wallBounds[0].y = (kMazeRowCount / 2 - (row + 1) - 1) * kTileSize;
					wallBounds[1].x = (column + i - kMazeColumnCount / 2) * kTileSize;
					wallBounds[1].y = (kMazeRowCount / 2 - (row + 1)) * kTileSize;

					//충돌 시
					if (wallBounds[0].x <= targetPosition.x + kPlayerRadius && wallBounds[1].x >= targetPosition.x - kPlayerRadius
						&& wallBounds[0].y <= targetPosition.z + kPlayerRadius && wallBounds[1].y >= targetPosition.z - kPlayerRadius)
					{
						targetPosition.z = wallBounds[1].y + kPlayerRadius + 0.1f;
						break;
					}
				}
			}
		}
		// z축 양의 방향으로 이동일 경우
		else if (movementDirection.z > 0)
		{
			for (i = 0; i < 3; i++)
			{
				// 현재 위치 기준 위쪽 3개 블록에 대해 검사
				// 제일 외곽 벽일 경우 따로 검사
				if (row == 0)
				{
					if (targetPosition.z + kPlayerRadius >= kMazeRowCount / 2 * kTileSize)
					{
						targetPosition.z = kMazeRowCount / 2 * kTileSize - kPlayerRadius;
					}
				}
				else if (IsWall(map, row - 1, column - 1 + i))
				{
					wallBounds[0].x = (column - 1 + i - kMazeColumnCount / 2) * kTileSize;
					wallBounds[0].y = (kMazeRowCount / 2 - (row - 1) - 1) * kTileSize;
					wallBounds[1].x = (column + i - kMazeColumnCount / 2) * kTileSize;
					wallBounds[1].y = (kMazeRowCount / 2 - (row - 1)) * kTileSize;

					//충돌 시
					if (wallBounds[0].x <= targetPosition.x + kPlayerRadius && wallBounds[1].x >= targetPosition.x - kPlayerRadius
						&& wallBounds[0].y <= targetPosition.z + kPlayerRadius && wallBounds[1].y >= targetPosition.z - kPlayerRadius)
					{
						targetPosition.z = wallBounds[0].y - kPlayerRadius - 0.1f;
						break;
					}
				}
			}
		}
	}
	// LookAt은 Position이 이동한 만큼만 더하거나 빼주면 됨
	m_lookAt.x += targetPosition.x - currentPosition.x;
	m_lookAt.z += targetPosition.z - currentPosition.z;
	if (isNoClipEnabled)
	{
		m_lookAt.y += targetPosition.y - currentPosition.y;
	}

	if (!isNoClipEnabled)
	{
		targetPosition.y = kTileSize / 2;
	}

	// world matrix도 translate해주기
	D3DXMATRIX translationMatrix;
	D3DXMatrixTranslation(&translationMatrix, targetPosition.x - currentPosition.x, targetPosition.y - currentPosition.y, targetPosition.z - currentPosition.z);
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);

	//SetPosition(targetPosition);

	m_flashlight.Position = GetPosition();
	return TRUE;
}

VOID Player::Rotate(BOOL isCounterClockwise, FLOAT deltaTimeSeconds)
{
	FLOAT rotationAmount = kPlayerRotationSpeed * deltaTimeSeconds;

	// isCounterClockwise로 q인지 e인지 구분하고, angle만큼 회전을 하며, LookAt과 Position 사이 간격은 distance
	// 이 함수는 좌우 회전만 하므로, CalculateAngle 불필요
	D3DXMATRIX rotationMatrix, translationMatrix;
	D3DXVECTOR3 currentPosition = GetPosition();
	FLOAT lookDirectionScale = kLookAtDistance;

	D3DXMatrixTranslation(&translationMatrix, -currentPosition.x, -currentPosition.y, -currentPosition.z);
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);

	if (isCounterClockwise == TRUE)
	{
		D3DXMatrixRotationY(&rotationMatrix, -rotationAmount);
	}
	else
	{
		D3DXMatrixRotationY(&rotationMatrix, rotationAmount);
	}
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &rotationMatrix);

	D3DXMatrixTranslation(&translationMatrix, currentPosition.x, currentPosition.y, currentPosition.z);
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);

	lookDirectionScale /= sqrtf(m_worldMatrix._31 * m_worldMatrix._31 + m_worldMatrix._32 * m_worldMatrix._32 + m_worldMatrix._33 * m_worldMatrix._33);
	m_lookAt.x = currentPosition.x + m_worldMatrix._31 * lookDirectionScale;
	m_lookAt.y = currentPosition.y + m_worldMatrix._32 * lookDirectionScale;
	m_lookAt.z = currentPosition.z + m_worldMatrix._33 * lookDirectionScale;

	m_flashlight.Direction = D3DXVECTOR3(m_worldMatrix._31, m_worldMatrix._32, m_worldMatrix._33);
}

VOID Player::Rotate(BOOL isCounterClockwise, BOOL isVertical, FLOAT angle)
{
	// 위아래일때 ccw면 위, !ccw면 아래
	D3DXMATRIX rotationMatrix, translationMatrix;
	D3DXVECTOR3 rotationAxis;
	D3DXVECTOR3 currentPosition = GetPosition();
	FLOAT lookDirectionScale = kLookAtDistance;

	D3DXMatrixTranslation(&translationMatrix, -currentPosition.x, -currentPosition.y, -currentPosition.z);
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);

	// 좌우 회전
	if (isVertical == FALSE)
	{
		rotationAxis = D3DXVECTOR3(m_worldMatrix._21, m_worldMatrix._22, m_worldMatrix._23);
		if (isCounterClockwise == TRUE)
		{
			D3DXMatrixRotationY(&rotationMatrix, -angle);
		}
		else
		{
			D3DXMatrixRotationY(&rotationMatrix, angle);
		}
	}
	// 상하 회전
	else
	{
		// up vector와 player가 바라보는 방향 vector 사이의 각도 구하기
		FLOAT lookAngle = CalculateAngle(kWorldUp, D3DXVECTOR3(m_worldMatrix._31, m_worldMatrix._32, m_worldMatrix._33));

		rotationAxis = D3DXVECTOR3(m_worldMatrix._11, 0, m_worldMatrix._13);
		// 아래
		if (isCounterClockwise == TRUE)
		{
			// 밑으로 너무 숙이면 회전 안시킴
			if (lookAngle + angle > D3DXToRadian(175.0f))
			{
				D3DXMatrixTranslation(&translationMatrix, currentPosition.x, currentPosition.y, currentPosition.z);
				D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);
				return;
			}
			D3DXMatrixRotationAxis(&rotationMatrix, &rotationAxis, angle);
		}
		// 위
		else
		{
			// 위로 너무 올라가면 회전 안시킴
			if (lookAngle - angle < D3DXToRadian(5.0f))
			{
				D3DXMatrixTranslation(&translationMatrix, currentPosition.x, currentPosition.y, currentPosition.z);
				D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);
				return;
			}
			D3DXMatrixRotationAxis(&rotationMatrix, &rotationAxis, -angle);
		}
	}
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &rotationMatrix);

	D3DXMatrixTranslation(&translationMatrix, currentPosition.x, currentPosition.y, currentPosition.z);
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);

	lookDirectionScale /= sqrtf(m_worldMatrix._31 * m_worldMatrix._31 + m_worldMatrix._32 * m_worldMatrix._32 + m_worldMatrix._33 * m_worldMatrix._33);
	m_lookAt.x = currentPosition.x + m_worldMatrix._31 * lookDirectionScale;
	m_lookAt.y = currentPosition.y + m_worldMatrix._32 * lookDirectionScale;
	m_lookAt.z = currentPosition.z + m_worldMatrix._33 * lookDirectionScale;

	m_flashlight.Direction = D3DXVECTOR3(m_worldMatrix._31, m_worldMatrix._32, m_worldMatrix._33);
}

VOID Player::Jump()
{
	// TODO: 점프 혹은 스페이스바 기능 추가
}

VOID Player::FireBullet(LPPOINT cursorPosition)
{
	// 마우스 클릭 시, 클릭한 방향으로 벡터를 설정하고 일직선으로 날아가는 투사체 발사
	// world 상에서 플레이어 위치를 (0,0,0)으로 생각하고, 마우스가 클릭되는 평면을
	// 총알 크기, 속도, 연사(마우스 꾹 누르고 있을 때), 벽이나 오브젝트에 닿으면 사라지도록
	Bullet bullet;
	bullet.position = this->GetPosition();
	bullet.direction = this->GetLookAt() - this->GetPosition();
	m_bullets.push_back(bullet);
}

VOID Player::UpdateBullets(FLOAT deltaTimeSeconds)
{
	// 매 프레임마다 호출해서 총알의 이동 및 충돌 후 제거 연산 수행 (x)
	// 프레임마다 호출하면 프레임 낮은 똥컴에서는 총알 속도가 느려지는 말도 안 되는 상황 발생한다.
	// 프레임이 아닌, 실제 시간을 기준으로 이동시켜야함
	// 발사된 총알이 없을 시 건너뜀
	if (m_bullets.empty())
		return;

	for (auto iter = m_bullets.begin(); iter != m_bullets.end();)
	{
		FLOAT travelDistance = m_bulletVelocity * deltaTimeSeconds;

		travelDistance /= CalculateLength(iter->direction);
		iter->position += travelDistance * iter->direction;

		// 벽 또는 장애물과 충돌 검사
		// 일단은 플레이어로부터 100만큼 떨어지면 제거되게
		if (CalculateLength(iter->position - GetPosition()) >= 100.0f)
		{
			iter = m_bullets.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

VOID Player::RenderBullets(LPDIRECT3DDEVICE9 device, LPD3DXMESH sphere)
{
	D3DXMATRIX worldMatrix;
	for (size_t i = 0; i < m_bullets.size(); i++)
	{
		D3DXMatrixIdentity(&worldMatrix);
		D3DXMatrixTranslation(&worldMatrix, m_bullets[i].position.x, m_bullets[i].position.y, m_bullets[i].position.z);
		device->SetTransform(D3DTS_WORLD, &worldMatrix);
		sphere->DrawSubset(0);
	}
}
