#include "CPlayer.h"
CPlayer::CPlayer()
{
	D3DXMATRIX tmpMatrix;
	// 일반화된 m by n matrix로 미로 정보를 받게 되면 수정해야함
	// 현재는 고정된 위치 시작
	//m_Position = D3DXVECTOR3(55.0f, LENGTH_OF_TILE / 2, -65.0f);
	//m_LookAt = m_Position;
	m_LookAt = v3StartPosition;
	m_LookAt.z += LOOKAT_DISTANCE;
	// player world matrix initialization
	D3DXMatrixIdentity(&m_PlayerWorld);
	D3DXMatrixTranslation(&m_PlayerWorld, v3StartPosition.x, v3StartPosition.y, v3StartPosition.z);

	//// light setting
	m_IsLightOn = TRUE;
	ZeroMemory(&m_FlashLight, sizeof(D3DLIGHT9));
	m_FlashLight.Type = D3DLIGHT_SPOT;
	m_FlashLight.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_FlashLight.Ambient.r = 1.0f;
	m_FlashLight.Ambient.g = 1.0f;
	m_FlashLight.Ambient.b = 1.0f;
	m_FlashLight.Specular.r = 1.0f;
	m_FlashLight.Specular.g = 1.0f;
	m_FlashLight.Specular.b = 1.0f;
	m_FlashLight.Position = v3StartPosition;
	m_FlashLight.Direction = D3DXVECTOR3(m_PlayerWorld._31, m_PlayerWorld._32, m_PlayerWorld._33);
	D3DXVec3Normalize((D3DXVECTOR3*)&m_FlashLight.Direction, (D3DXVECTOR3*)&m_FlashLight.Direction);
	m_FlashLight.Range = 1500.0f;
	m_FlashLight.Attenuation0 = 1.0f;
	m_FlashLight.Attenuation1 = 0.01f;
	m_FlashLight.Attenuation2 = 0.0001f;
	m_FlashLight.Falloff = 2.0f;
	m_FlashLight.Phi = D3DXToRadian(60.0f);
	m_FlashLight.Theta = D3DXToRadian(30.0f);
	//m_FlashLight.Phi = D3DXToRadian(100.0f);
	//m_FlashLight.Theta = D3DXToRadian(100.0f);
	//m_FlashLight.Phi = 1.0f;
	//m_FlashLight.Theta = 0.7f;

	m_CurrentMoveTime = timeGetTime();
	m_CurrentRotateTime = timeGetTime();
}

// PLAYER MOVE
// 바꾸어야 할 것: 플레이어 위치, 플레이어 LookAt
// 필요한 것: 플레이어가 움직일 방향 벡터, 거리, 플에이어 현 위치, 플레이어 현 LookAt, 맵 정보
BOOL CPlayer::Move(MOVE_DIRECTION direction, const char(*map)[NUM_OF_COLUMN + 1], BOOL NoClip)
{
	DWORD currentTime = timeGetTime();
	if (currentTime - m_CurrentMoveTime < 10) return FALSE;
	m_CurrentMoveTime = currentTime;

	D3DXVECTOR3 vecDirection, tmpPosition; // 벽을 생각하지 않고 이동된 위치. 주변 8개 벽과 이것을 대조해 최종 위치 결정
	D3DXVECTOR3 currentPosition = GetPosition();
	FLOAT fCoefficient = TRANSLATION_DISTANCE;
	int i, nCoX, nCoZ;

	// y축 움직임은 없으므로 y축 정보는 걍 빼고 계산
	// => 자유시점의 경우도 생각해서 y축도 이동
	if (direction == MOVE_DIRECTION::left)
	{
		vecDirection.x = -m_PlayerWorld._11;
		vecDirection.y = -m_PlayerWorld._12;
		vecDirection.z = -m_PlayerWorld._13;
		fCoefficient /= sqrtf(vecDirection.x * vecDirection.x + vecDirection.y * vecDirection.y + vecDirection.z * vecDirection.z);

		tmpPosition.x = currentPosition.x + vecDirection.x * fCoefficient;
		tmpPosition.y = currentPosition.y + vecDirection.y * fCoefficient;
		tmpPosition.z = currentPosition.z + vecDirection.z * fCoefficient;
	}
	else if (direction == MOVE_DIRECTION::right)
	{
		vecDirection.x = m_PlayerWorld._11;
		vecDirection.y = m_PlayerWorld._12;
		vecDirection.z = m_PlayerWorld._13;
		fCoefficient /= sqrtf(vecDirection.x * vecDirection.x + vecDirection.y * vecDirection.y + vecDirection.z * vecDirection.z);

		tmpPosition.x = currentPosition.x + vecDirection.x * fCoefficient;
		tmpPosition.y = currentPosition.y + vecDirection.y * fCoefficient;
		tmpPosition.z = currentPosition.z + vecDirection.z * fCoefficient;
	}
	else if (direction == MOVE_DIRECTION::front)
	{
		vecDirection.x = m_PlayerWorld._31;
		vecDirection.y = m_PlayerWorld._32;
		vecDirection.z = m_PlayerWorld._33;
		fCoefficient /= sqrtf(vecDirection.x * vecDirection.x + vecDirection.y * vecDirection.y + vecDirection.z * vecDirection.z);

		tmpPosition.x = currentPosition.x + vecDirection.x * fCoefficient;
		tmpPosition.y = currentPosition.y + vecDirection.y * fCoefficient;
		tmpPosition.z = currentPosition.z + vecDirection.z * fCoefficient;
	}
	else if (direction == MOVE_DIRECTION::back)
	{
		vecDirection.x = -m_PlayerWorld._31;
		vecDirection.y = -m_PlayerWorld._32;
		vecDirection.z = -m_PlayerWorld._33;
		fCoefficient /= sqrtf(vecDirection.x * vecDirection.x + vecDirection.y * vecDirection.y + vecDirection.z * vecDirection.z);

		tmpPosition.x = currentPosition.x + vecDirection.x * fCoefficient;
		tmpPosition.y = currentPosition.y + vecDirection.y * fCoefficient;
		tmpPosition.z = currentPosition.z + vecDirection.z * fCoefficient;

	}
	if (!NoClip)
	{
		// 충돌을 검사할 블록의 왼쪽아래(minX, minZ)와 오른쪽위(maxX,maxZ) 두 점
		D3DXVECTOR2 WallPoint[2];
		// 현재 좌표
		nCoX = static_cast<int>(floorf(currentPosition.x / LENGTH_OF_TILE)) + NUM_OF_COLUMN / 2;
		nCoZ = NUM_OF_ROW / 2 - static_cast<int>(floorf(currentPosition.z / LENGTH_OF_TILE)) - 1;
		// 해결: 외곽 벽에 부딪히는 경우 추가, 몇몇 곳에서 블록 속으로 들어가버리는 버그 수정하기
		// x축 음의 방향으로 이동일 경우
		if (vecDirection.x < 0)
		{
			for (i = 0; i < 3; i++)
			{
				// 현재 위치 기준 왼쪽 3개 블록에 대해 검사
				// 제일 외곽 벽일 경우 따로 검사
				if (nCoX == 0)
				{
					if (tmpPosition.x - PLAYER_RADIUS <= -NUM_OF_COLUMN / 2 * LENGTH_OF_TILE)
					{
						tmpPosition.x = -NUM_OF_COLUMN / 2 * LENGTH_OF_TILE + PLAYER_RADIUS;
					}
				}
				else if (map[nCoZ - 1 + i][nCoX - 1] == '*')
				{
					WallPoint[0].x = (nCoX - 1 - NUM_OF_COLUMN / 2) * LENGTH_OF_TILE;
					WallPoint[0].y = (NUM_OF_ROW / 2 - (nCoZ - 1 + i) - 1) * LENGTH_OF_TILE;
					WallPoint[1].x = (nCoX - NUM_OF_COLUMN / 2) * LENGTH_OF_TILE;
					WallPoint[1].y = (NUM_OF_ROW / 2 - (nCoZ - 1 + i)) * LENGTH_OF_TILE;

					//충돌 시
					if (WallPoint[0].x <= tmpPosition.x + PLAYER_RADIUS && WallPoint[1].x >= tmpPosition.x - PLAYER_RADIUS
						&& WallPoint[0].y <= tmpPosition.z + PLAYER_RADIUS && WallPoint[1].y >= tmpPosition.z - PLAYER_RADIUS)
					{
						if (map[nCoZ][nCoX - 1] == '*')
							tmpPosition.x = WallPoint[1].x + PLAYER_RADIUS + 0.1f;
						break;
					}
				}
			}
		}
		// x축 양의 방향으로 이동일 경우
		else if (vecDirection.x > 0)
		{
			for (i = 0; i < 3; i++)
			{
				// 현재 위치 기준 오른쪽 3개 블록에 대해 검사
				// 제일 외곽 벽일 경우 따로 검사
				if (nCoX == NUM_OF_COLUMN - 1)
				{
					if (tmpPosition.x + PLAYER_RADIUS >= NUM_OF_COLUMN / 2 * LENGTH_OF_TILE)
					{
						tmpPosition.x = NUM_OF_COLUMN / 2 * LENGTH_OF_TILE - PLAYER_RADIUS;
					}
				}
				else if (map[nCoZ - 1 + i][nCoX + 1] == '*')
				{
					WallPoint[0].x = (nCoX + 1 - NUM_OF_COLUMN / 2) * LENGTH_OF_TILE;
					WallPoint[0].y = (NUM_OF_ROW / 2 - (nCoZ - 1 + i) - 1) * LENGTH_OF_TILE;
					WallPoint[1].x = (nCoX + 2 - NUM_OF_COLUMN / 2) * LENGTH_OF_TILE;
					WallPoint[1].y = (NUM_OF_ROW / 2 - (nCoZ - 1 + i)) * LENGTH_OF_TILE;

					//충돌 시
					if (WallPoint[0].x <= tmpPosition.x + PLAYER_RADIUS && WallPoint[1].x >= tmpPosition.x - PLAYER_RADIUS
						&& WallPoint[0].y <= tmpPosition.z + PLAYER_RADIUS && WallPoint[1].y >= tmpPosition.z - PLAYER_RADIUS)
					{
						if (map[nCoZ][nCoX + 1] == '*')
							tmpPosition.x = WallPoint[0].x - PLAYER_RADIUS - 0.1f;
						break;
					}
				}
			}
		}

		// z축 음의 방향으로 이동일 경우
		if (vecDirection.z < 0)
		{
			for (i = 0; i < 3; i++)
			{
				// 현재 위치 기준 아래쪽 3개 블록에 대해 검사
				// 제일 외곽 벽일 경우 따로 검사
				if (nCoZ == NUM_OF_ROW - 1)
				{
					if (tmpPosition.z - PLAYER_RADIUS <= -NUM_OF_ROW / 2 * LENGTH_OF_TILE)
					{
						tmpPosition.z = -NUM_OF_ROW / 2 * LENGTH_OF_TILE + PLAYER_RADIUS;
					}
				}
				else if (map[nCoZ + 1][nCoX - 1 + i] == '*')
				{
					WallPoint[0].x = (nCoX - 1 + i - NUM_OF_COLUMN / 2) * LENGTH_OF_TILE;
					WallPoint[0].y = (NUM_OF_ROW / 2 - (nCoZ + 1) - 1) * LENGTH_OF_TILE;
					WallPoint[1].x = (nCoX + i - NUM_OF_COLUMN / 2) * LENGTH_OF_TILE;
					WallPoint[1].y = (NUM_OF_ROW / 2 - (nCoZ + 1)) * LENGTH_OF_TILE;

					//충돌 시
					if (WallPoint[0].x <= tmpPosition.x + PLAYER_RADIUS && WallPoint[1].x >= tmpPosition.x - PLAYER_RADIUS
						&& WallPoint[0].y <= tmpPosition.z + PLAYER_RADIUS && WallPoint[1].y >= tmpPosition.z - PLAYER_RADIUS)
					{
						tmpPosition.z = WallPoint[1].y + PLAYER_RADIUS + 0.1f;
						break;
					}
				}
			}
		}
		// z축 양의 방향으로 이동일 경우
		else if (vecDirection.z > 0)
		{
			for (i = 0; i < 3; i++)
			{
				// 현재 위치 기준 위쪽 3개 블록에 대해 검사
				// 제일 외곽 벽일 경우 따로 검사
				if (nCoZ == 0)
				{
					if (tmpPosition.z + PLAYER_RADIUS >= NUM_OF_ROW / 2 * LENGTH_OF_TILE)
					{
						tmpPosition.z = NUM_OF_ROW / 2 * LENGTH_OF_TILE - PLAYER_RADIUS;
					}
				}
				else if (map[nCoZ - 1][nCoX - 1 + i] == '*')
				{
					WallPoint[0].x = (nCoX - 1 + i - NUM_OF_COLUMN / 2) * LENGTH_OF_TILE;
					WallPoint[0].y = (NUM_OF_ROW / 2 - (nCoZ - 1) - 1) * LENGTH_OF_TILE;
					WallPoint[1].x = (nCoX + i - NUM_OF_COLUMN / 2) * LENGTH_OF_TILE;
					WallPoint[1].y = (NUM_OF_ROW / 2 - (nCoZ - 1)) * LENGTH_OF_TILE;

					//충돌 시
					if (WallPoint[0].x <= tmpPosition.x + PLAYER_RADIUS && WallPoint[1].x >= tmpPosition.x - PLAYER_RADIUS
						&& WallPoint[0].y <= tmpPosition.z + PLAYER_RADIUS && WallPoint[1].y >= tmpPosition.z - PLAYER_RADIUS)
					{
						tmpPosition.z = WallPoint[0].y - PLAYER_RADIUS - 0.1f;
						break;
					}
				}
			}
		}
	}
	// LookAt은 Position이 이동한 만큼만 더하거나 빼주면 됨
	m_LookAt.x += tmpPosition.x - currentPosition.x;
	m_LookAt.z += tmpPosition.z - currentPosition.z;
	if (NoClip)
	{
		m_LookAt.y += tmpPosition.y - currentPosition.y;
	}

	if (!NoClip)
	{
		tmpPosition.y = LENGTH_OF_TILE / 2;
	}

	// world matrix도 translate해주기
	D3DXMATRIX tmpTranslation;
	D3DXMatrixTranslation(&tmpTranslation, tmpPosition.x - currentPosition.x, tmpPosition.y - currentPosition.y, tmpPosition.z - currentPosition.z);
	D3DXMatrixMultiply(&m_PlayerWorld, &m_PlayerWorld, &tmpTranslation);

	//SetPosition(tmpPosition);

	m_FlashLight.Position = GetPosition();
	return TRUE;
}

VOID CPlayer::Rotate(BOOL bIsCCW)
{
	DWORD currentTime = timeGetTime();
	if (currentTime - m_CurrentRotateTime < 10) return;
	m_CurrentRotateTime = currentTime;

	// bIsCCW로 q인지 e인지 구분하고, angle만큼 회전을 하며, LookAt과 Position 사이 간격은 distance
	// 이 함수는 좌우 회전만 하므로, CalculateAngle 불필요
	D3DXMATRIX mtRotation, mtTranslation;
	D3DXVECTOR3 currentPosition = GetPosition();
	FLOAT fCoefficient = LOOKAT_DISTANCE;
	D3DXVECTOR3 v3Axis = D3DXVECTOR3(m_PlayerWorld._21, m_PlayerWorld._22, m_PlayerWorld._23);

	D3DXMatrixTranslation(&mtTranslation, -currentPosition.x, -currentPosition.y, -currentPosition.z);
	D3DXMatrixMultiply(&m_PlayerWorld, &m_PlayerWorld, &mtTranslation);

	if (bIsCCW == TRUE)
	{
		D3DXMatrixRotationY(&mtRotation, -ROTATION_AMOUNT);
	}
	else
	{
		D3DXMatrixRotationY(&mtRotation, ROTATION_AMOUNT);
	}
	D3DXMatrixMultiply(&m_PlayerWorld, &m_PlayerWorld, &mtRotation);

	D3DXMatrixTranslation(&mtTranslation, currentPosition.x, currentPosition.y, currentPosition.z);
	D3DXMatrixMultiply(&m_PlayerWorld, &m_PlayerWorld, &mtTranslation);

	fCoefficient /= sqrtf(m_PlayerWorld._31 * m_PlayerWorld._31 + m_PlayerWorld._32 * m_PlayerWorld._32 + m_PlayerWorld._33 * m_PlayerWorld._33);
	m_LookAt.x = currentPosition.x + m_PlayerWorld._31 * fCoefficient;
	m_LookAt.y = currentPosition.y + m_PlayerWorld._32 * fCoefficient;
	m_LookAt.z = currentPosition.z + m_PlayerWorld._33 * fCoefficient;

	m_FlashLight.Direction = D3DXVECTOR3(m_PlayerWorld._31, m_PlayerWorld._32, m_PlayerWorld._33);
}
VOID CPlayer::Rotate(BOOL bIsCCW, BOOL bIsUpDown, FLOAT angle)
{
	DWORD currentTime = timeGetTime();
	//if (currentTime - m_CurrentRotateTime < 10) return;
	// 현재 로직 상 x축 회전 후 y축 회전시키므로, y축 회전 하고 시간 체크해야 온전히 회전됨
	/*if (bIsUpDown)
		m_CurrentRotateTime = currentTime;*/

		// 위아래일때 ccw면 위, !ccw면 아래
	D3DXMATRIX mtRotation, mtTranslation;
	D3DXVECTOR3 v3RotationAxis;
	D3DXVECTOR3 currentPosition = GetPosition();
	D3DXQUATERNION vQuart;
	FLOAT fCoefficient = LOOKAT_DISTANCE;

	D3DXMatrixTranslation(&mtTranslation, -currentPosition.x, -currentPosition.y, -currentPosition.z);
	D3DXMatrixMultiply(&m_PlayerWorld, &m_PlayerWorld, &mtTranslation);

	// 좌우 회전
	if (bIsUpDown == FALSE)
	{
		v3RotationAxis = D3DXVECTOR3(m_PlayerWorld._21, m_PlayerWorld._22, m_PlayerWorld._23);
		if (bIsCCW == TRUE)
		{
			D3DXMatrixRotationY(&mtRotation, -angle);
		}
		else
		{
			D3DXMatrixRotationY(&mtRotation, angle);
		}
	}
	// 상하 회전
	else
	{
		// up vector와 player가 바라보는 방향 vector 사이의 각도 구하기
		FLOAT fAngle = CalculateAngle(v3Up, D3DXVECTOR3(m_PlayerWorld._31, m_PlayerWorld._32, m_PlayerWorld._33));

		v3RotationAxis = D3DXVECTOR3(m_PlayerWorld._11, 0, m_PlayerWorld._13);
		// 아래
		if (bIsCCW == TRUE)
		{
			// 밑으로 너무 숙이면 회전 안시킴
			if (fAngle + ROTATION_AMOUNT > D3DXToRadian(175.0f))
			{
				D3DXMatrixTranslation(&mtTranslation, currentPosition.x, currentPosition.y, currentPosition.z);
				D3DXMatrixMultiply(&m_PlayerWorld, &m_PlayerWorld, &mtTranslation);
				return;
			}
			D3DXMatrixRotationAxis(&mtRotation, &v3RotationAxis, angle);
		}
		// 위
		else
		{
			// 위로 너무 올라가면 회전 안시킴
			if (fAngle - ROTATION_AMOUNT < D3DXToRadian(5.0f))
			{
				D3DXMatrixTranslation(&mtTranslation, currentPosition.x, currentPosition.y, currentPosition.z);
				D3DXMatrixMultiply(&m_PlayerWorld, &m_PlayerWorld, &mtTranslation);
				return;
			}
			D3DXMatrixRotationAxis(&mtRotation, &v3RotationAxis, -angle);
		}
	}
	D3DXMatrixMultiply(&m_PlayerWorld, &m_PlayerWorld, &mtRotation);

	D3DXMatrixTranslation(&mtTranslation, currentPosition.x, currentPosition.y, currentPosition.z);
	D3DXMatrixMultiply(&m_PlayerWorld, &m_PlayerWorld, &mtTranslation);

	fCoefficient /= sqrtf(m_PlayerWorld._31 * m_PlayerWorld._31 + m_PlayerWorld._32 * m_PlayerWorld._32 + m_PlayerWorld._33 * m_PlayerWorld._33);
	m_LookAt.x = currentPosition.x + m_PlayerWorld._31 * fCoefficient;
	m_LookAt.y = currentPosition.y + m_PlayerWorld._32 * fCoefficient;
	m_LookAt.z = currentPosition.z + m_PlayerWorld._33 * fCoefficient;

	m_FlashLight.Direction = D3DXVECTOR3(m_PlayerWorld._31, m_PlayerWorld._32, m_PlayerWorld._33);
}

VOID CPlayer::Attack(LPPOINT CursorPosition)
{
	// 마우스 클릭 시, 클릭한 방향으로 벡터를 설정하고 일직선으로 날아가는 투사체 발사
	// world 상에서 플레이어 위치를 (0,0,0)으로 생각하고, 마우스가 클릭되는 평면을
	// 총알 크기, 속도, 연사(마우스 꾹 누르고 있을 때), 벽이나 오브젝트에 닿으면 사라지도록
	Bullet tmpBullet;
	tmpBullet.v3Position = this->GetPosition();
	tmpBullet.v3Direction = this->GetLookAt() - this->GetPosition();
	tmpBullet.Time = timeGetTime();
	m_Bullet.push_back(tmpBullet);
}
VOID CPlayer::MoveBullet()
{
	// 매 프레임마다 호출해서 총알의 이동 및 충돌 후 제거 연산 수행 (x)
	// 프레임마다 호출하면 프레임 낮은 똥컴에서는 총알 속도가 느려지는 말도 안 되는 상황 발생한다.
	// 프레임이 아닌, 실제 시간을 기준으로 이동시켜야함
	// 발사된 총알이 없을 시 건너뜀
	DWORD currentTime = timeGetTime();
	if (m_Bullet.size() == 0)
		return;
	else
	{
		FLOAT fCoefficient;
		for (auto iter = m_Bullet.begin(); iter != m_Bullet.end();)
		{
			fCoefficient = this->m_BulletVelocity * (currentTime - iter->Time);
			iter->Time = currentTime;
			// 우선 이동
			fCoefficient /= sqrtf(iter->v3Direction.x * iter->v3Direction.x + iter->v3Direction.y * iter->v3Direction.y + iter->v3Direction.z * iter->v3Direction.z);
			iter->v3Position.x += fCoefficient * iter->v3Direction.x;
			iter->v3Position.y += fCoefficient * iter->v3Direction.y;
			iter->v3Position.z += fCoefficient * iter->v3Direction.z;
			// 벽 또는 장애물과 충돌 검사
			// 일단은 플레이어로부터 100만큼 떨어지면 제거되게
			if (Length(iter->v3Position - this->GetPosition()) >= 100.0f)
			{
				iter = m_Bullet.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}
}
VOID CPlayer::DrawBullet(LPDIRECT3DDEVICE9 device, LPD3DXMESH sphere)
{
	D3DXMATRIX world;
	for (size_t i = 0; i < m_Bullet.size(); i++)
	{
		D3DXMatrixIdentity(&world);
		D3DXMatrixTranslation(&world, m_Bullet[i].v3Position.x, m_Bullet[i].v3Position.y, m_Bullet[i].v3Position.z);
		device->SetTransform(D3DTS_WORLD, &world);
		sphere->DrawSubset(0);
	}
}
