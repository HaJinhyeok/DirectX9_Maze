#include "Tiger.h"

Tiger::Tiger(D3DXVECTOR3 position)
{
	// scale 먼저 하고 translation
	D3DXMatrixScaling(&m_worldMatrix, kTigerScale, kTigerScale, kTigerScale * 2.0f / 3.0f);
	D3DXMATRIX translationMatrix;
	D3DXMatrixTranslation(&translationMatrix, position.x, position.y, position.z);
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);
	m_isAlive = TRUE;
	m_isRotating = FALSE;
	m_isClockwise = TRUE;
	for (int i = 0; i < 4; i++)
	{
		m_isWallOpen[i] = TRUE;
	}
	m_rotationAmount = 0;
	m_rotationCount = 0;
	m_accumulatedTimeSeconds = 0.0f;
}

Tiger::~Tiger() = default;

int Tiger::Load(LPDIRECT3DDEVICE9 device, char* xFilePath)
{
	return m_model.Load(device, xFilePath);
}

int Tiger::Render(LPDIRECT3DDEVICE9 device)
{
	return m_model.Render(device);
}

VOID Tiger::Move(const char(*map)[kMazeColumnCount + 1], FLOAT deltaTimeSeconds)
{
	m_accumulatedTimeSeconds += deltaTimeSeconds;

	while (m_accumulatedTimeSeconds >= kTigerUpdateIntervalSeconds)
	{
		// 호랑이가 통로를 따라 움직이게 한다.
		// 호랑이가 현재 바라보는 방향 정보와 맵 정보를 이용해서 현재 방향으로 더 전진해도 되는지 여부 판단
		// 현재 바라보는 방향이 벽으로 막혀있을 경우, 시계방향으로 90도씩 회전시켜 다시 전진 여부 판단
		// 현재 바라보는 방향이 막혀있지 않더라도, 여러 갈래 길일 경우 랜덤하게 진행방향 바꾸도록 해볼까
		// => 랜덤(50%?)으로 방향 전환할 지 안 할 지 결정하는
		D3DXVECTOR3 currentLookAt = D3DXVECTOR3(-m_worldMatrix._31, -m_worldMatrix._32, -m_worldMatrix._33);
		D3DXVec3Normalize(&currentLookAt, &currentLookAt);
		D3DXVECTOR3 currentPosition = D3DXVECTOR3(m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43);
		FLOAT tileOffsetX = currentPosition.x / kTileSize, tileOffsetZ = currentPosition.z / kTileSize;
		tileOffsetX -= floorf(tileOffsetX);
		tileOffsetZ -= floorf(tileOffsetZ);
		// 우선, 현재 호랑이가 블록 정가운데 위치해있는지 확인
		if (tileOffsetX <= 0.5f + kEpsilon && tileOffsetX >= 0.5f - kEpsilon)
		{
			if (tileOffsetZ <= 0.50f + kEpsilon && tileOffsetZ >= 0.5f - kEpsilon)
			{
				//OutputDebugString("center of block\n");
				if (m_isRotating == TRUE)
				{
					this->Rotate(m_isClockwise);
				}
				else if (m_isRotating == FALSE && m_rotationCount == 1)
				{
					m_rotationCount--;
				}
				else
				{
					// 현재 좌표
					int column = static_cast<int>(floorf(currentPosition.x / kTileSize)) + kMazeColumnCount / 2;
					int row = kMazeRowCount / 2 - static_cast<int>(floorf(currentPosition.z / kTileSize)) - 1;
					// random_device로 시드값 생성해 메르센 트위스트 알고리즘으로 난수 생성
					mt19937 randomEngine(m_randomDevice());

					// 상하좌우 칸으로 전진 가능한지 판별
					{
						// 0 - 상
						if (row == 0)
						{
							m_isWallOpen[0] = FALSE;
						}
						else if (map[row - 1][column] == '*')
						{
							m_isWallOpen[0] = FALSE;
						}
						else
						{
							m_isWallOpen[0] = TRUE;
						}
						// 1 - 하
						if (row == kMazeRowCount - 1)
						{
							m_isWallOpen[1] = FALSE;
						}
						else if (map[row + 1][column] == '*')
						{
							m_isWallOpen[1] = FALSE;
						}
						else
						{
							m_isWallOpen[1] = TRUE;
						}
						// 2 - 좌
						if (column == 0)
						{
							m_isWallOpen[2] = FALSE;
						}
						else if (map[row][column - 1] == '*')
						{
							m_isWallOpen[2] = FALSE;
						}

						else
						{
							m_isWallOpen[2] = TRUE;
						}
						// 3 - 우
						if (column == kMazeColumnCount - 1)
						{
							m_isWallOpen[3] = FALSE;
						}
						else if (map[row][column + 1] == '*')
						{
							m_isWallOpen[3] = FALSE;
						}
						else
						{
							m_isWallOpen[3] = TRUE;
						}
					}

					// 진행 방향이 막혀있을 경우
					// 진행 방향이 열려있고, 왼쪽과 오른쪽이 막혀있을 경우 == 일단은 그냥 직진
					// 진행 방향이 열려있고, 왼쪽 또는 오른쪽도 열려있을 경우 == 일단은 회전, 몇 갈래인지 카운트해서 랜덤하게 회전시켜 진행
					if (currentLookAt.x > 0 && fabsf(currentLookAt.z) <= kEpsilon)
					{
						// +x direction
						if (m_isWallOpen[3] == FALSE)
						{
							// 전진 막혔으니 일단 무조건 회전은 함
							m_isRotating = TRUE;
							m_rotationCount++;
							if (!m_isWallOpen[0] && m_isWallOpen[1])
							{
								m_isClockwise = TRUE;
							}
							else if (m_isWallOpen[0] && !m_isWallOpen[1])
							{
								m_isClockwise = FALSE;
							}
							else if (m_isWallOpen[0] && m_isWallOpen[1])
							{
								uniform_int_distribution<INT> distribution(0, 1);
								if (distribution(randomEngine) == 0)
								{
									m_isClockwise = TRUE;
								}
								else
								{
									m_isClockwise = FALSE;
								}
							}
							else
							{
								// 양쪽 다 막혔으므로 회전 2회
								m_rotationCount++;
							}
						}
						else
						{
							if (!m_isWallOpen[0] && !m_isWallOpen[1])
							{
								m_isRotating = FALSE;
							}
							else if (!m_isWallOpen[0] && m_isWallOpen[1])
							{
								uniform_int_distribution<INT> distribution(0, 1);
								if (distribution(randomEngine) == 0)
								{
									m_isRotating = FALSE;
								}
								else
								{
									m_isRotating = TRUE;
									m_isClockwise = TRUE;
									m_rotationCount++;
								}
							}
							else if (m_isWallOpen[0] && !m_isWallOpen[1])
							{
								uniform_int_distribution<INT> distribution(0, 1);
								if (distribution(randomEngine) == 0)
								{
									m_isRotating = FALSE;
								}
								else
								{
									m_isRotating = TRUE;
									m_isClockwise = FALSE;
									m_rotationCount++;
								}
							}
							else
							{
								uniform_int_distribution<INT> distribution(0, 2);
								if (distribution(randomEngine) == 0)
								{
									m_isRotating = FALSE;
								}
								else if (distribution(randomEngine) == 1)
								{
									m_isRotating = TRUE;
									m_isClockwise = TRUE;
									m_rotationCount++;
								}
								else
								{
									m_isRotating = TRUE;
									m_isClockwise = FALSE;
									m_rotationCount++;
								}
							}
						}
					}
					else if (currentLookAt.x < 0 && fabsf(currentLookAt.z) <= kEpsilon)
					{
						// -x direction
						if (m_isWallOpen[2] == FALSE)
						{
							// 전진 막혔으니 일단 무조건 회전은 함
							m_isRotating = TRUE;
							m_rotationCount++;
							if (!m_isWallOpen[1] && m_isWallOpen[0])
							{
								m_isClockwise = TRUE;
							}
							else if (m_isWallOpen[1] && !m_isWallOpen[0])
							{
								m_isClockwise = FALSE;
							}
							else if (m_isWallOpen[1] && m_isWallOpen[0])
							{
								uniform_int_distribution<INT> distribution(0, 1);
								if (distribution(randomEngine) == 0)
								{
									m_isClockwise = TRUE;
								}
								else
								{
									m_isClockwise = FALSE;
								}
							}
							else
							{
								m_rotationCount++;
							}
						}
						else
						{
							if (!m_isWallOpen[1] && !m_isWallOpen[0])
							{
								m_isRotating = FALSE;
							}
							else if (!m_isWallOpen[1] && m_isWallOpen[0])
							{
								uniform_int_distribution<INT> distribution(0, 1);
								if (distribution(randomEngine) == 0)
								{
									m_isRotating = FALSE;
								}
								else
								{
									m_isRotating = TRUE;
									m_isClockwise = TRUE;
									m_rotationCount++;
								}
							}
							else if (m_isWallOpen[1] && !m_isWallOpen[0])
							{
								uniform_int_distribution<INT> distribution(0, 1);
								if (distribution(randomEngine) == 0)
								{
									m_isRotating = FALSE;
								}
								else
								{
									m_isRotating = TRUE;
									m_isClockwise = FALSE;
									m_rotationCount++;
								}
							}
							else
							{
								uniform_int_distribution<INT> distribution(0, 2);
								if (distribution(randomEngine) == 0)
								{
									m_isRotating = FALSE;
								}
								else if (distribution(randomEngine) == 1)
								{
									m_isRotating = TRUE;
									m_isClockwise = TRUE;
									m_rotationCount++;
								}
								else
								{
									m_isRotating = TRUE;
									m_isClockwise = FALSE;
									m_rotationCount++;
								}
							}
						}
					}
					else if (currentLookAt.z > 0 && fabsf(currentLookAt.x) <= kEpsilon)
					{
						// +z direction
						if (m_isWallOpen[0] == FALSE)
						{
							// 전진 막혔으니 일단 무조건 회전은 함
							m_isRotating = TRUE;
							m_rotationCount++;
							if (!m_isWallOpen[2] && m_isWallOpen[3])
							{
								m_isClockwise = TRUE;
							}
							else if (m_isWallOpen[2] && !m_isWallOpen[3])
							{
								m_isClockwise = FALSE;
							}
							else if (m_isWallOpen[2] && m_isWallOpen[3])
							{
								uniform_int_distribution<INT> distribution(0, 1);
								if (distribution(randomEngine) == 0)
								{
									m_isClockwise = TRUE;
								}
								else
								{
									m_isClockwise = FALSE;
								}
							}
							else
							{
								// 양쪽 다 막혔으므로 회전 2회
								m_rotationCount++;
							}
						}
						else
						{
							if (!m_isWallOpen[2] && !m_isWallOpen[3])
							{
								m_isRotating = FALSE;
							}
							else if (!m_isWallOpen[2] && m_isWallOpen[3])
							{
								uniform_int_distribution<INT> distribution(0, 1);
								if (distribution(randomEngine) == 0)
								{
									m_isRotating = FALSE;
								}
								else
								{
									m_isRotating = TRUE;
									m_isClockwise = TRUE;
									m_rotationCount++;
								}
							}
							else if (m_isWallOpen[2] && !m_isWallOpen[3])
							{
								uniform_int_distribution<INT> distribution(0, 1);
								if (distribution(randomEngine) == 0)
								{
									m_isRotating = FALSE;
								}
								else
								{
									m_isRotating = TRUE;
									m_isClockwise = FALSE;
									m_rotationCount++;
								}
							}
							else
							{
								uniform_int_distribution<INT> distribution(0, 2);
								if (distribution(randomEngine) == 0)
								{
									m_isRotating = FALSE;
								}
								else if (distribution(randomEngine) == 1)
								{
									m_isRotating = TRUE;
									m_isClockwise = TRUE;
									m_rotationCount++;
								}
								else
								{
									m_isRotating = TRUE;
									m_isClockwise = FALSE;
									m_rotationCount++;
								}
							}
						}
					}
					else if (currentLookAt.z < 0 && fabsf(currentLookAt.x) <= kEpsilon)
					{
						// -z direction
						if (m_isWallOpen[1] == FALSE)
						{
							// 전진 막혔으니 일단 무조건 회전은 함
							m_isRotating = TRUE;
							m_rotationCount++;
							if (!m_isWallOpen[3] && m_isWallOpen[2])
							{
								m_isClockwise = TRUE;
							}
							else if (m_isWallOpen[3] && !m_isWallOpen[2])
							{
								m_isClockwise = FALSE;
							}
							else if (m_isWallOpen[3] && m_isWallOpen[2])
							{
								uniform_int_distribution<INT> distribution(0, 1);
								if (distribution(randomEngine) == 0)
								{
									m_isClockwise = TRUE;
								}
								else
								{
									m_isClockwise = FALSE;
								}
							}
							else
							{
								// 양쪽 다 막혔으므로 회전 2회
								m_rotationCount++;
							}
						}
						else
						{
							// 전진이 불가능하진 않으므로 회전은 하더라도 1회만
							if (!m_isWallOpen[3] && !m_isWallOpen[2])
							{
								// 양쪽 막히면 그냥 직진
								m_isRotating = FALSE;
							}
							else if (!m_isWallOpen[3] && m_isWallOpen[2])
							{
								uniform_int_distribution<INT> distribution(0, 1);
								if (distribution(randomEngine) == 0)
								{
									m_isRotating = FALSE;
								}
								else
								{
									m_isRotating = TRUE;
									m_isClockwise = TRUE;
									m_rotationCount++;
								}
							}
							else if (m_isWallOpen[3] && !m_isWallOpen[2])
							{
								uniform_int_distribution<INT> distribution(0, 1);
								if (distribution(randomEngine) == 0)
								{
									m_isRotating = FALSE;
								}
								else
								{
									m_isRotating = TRUE;
									m_isClockwise = FALSE;
									m_rotationCount++;
								}
							}
							else
							{
								uniform_int_distribution<INT> distribution(0, 2);
								if (distribution(randomEngine) == 0)
								{
									m_isRotating = FALSE;
								}
								else if (distribution(randomEngine) == 1)
								{
									m_isRotating = TRUE;
									m_isClockwise = TRUE;
									m_rotationCount++;
								}
								else
								{
									m_isRotating = TRUE;
									m_isClockwise = FALSE;
									m_rotationCount++;
								}
							}
						}
					}
				}

			}
		}
		if (m_rotationAmount == 90)
		{
			if (m_rotationCount == 2)
			{
				m_rotationCount--;
			}
			else
			{
				m_isRotating = FALSE;
			}
			// 90도 회전 후 벡터값 조정?
			m_rotationAmount = 0;
		}
		else if (m_isRotating == FALSE)
		{
			D3DXMATRIX translationMatrix;
			FLOAT movementScale = kTigerMoveDistance;
			movementScale /= sqrtf(currentLookAt.x * currentLookAt.x + currentLookAt.y * currentLookAt.y + currentLookAt.z * currentLookAt.z);
			D3DXMatrixTranslation(&translationMatrix, currentLookAt.x * movementScale, currentLookAt.y * movementScale, currentLookAt.z * movementScale);
			D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);
		}

		m_accumulatedTimeSeconds -= kTigerUpdateIntervalSeconds;
	}
}

// 90도 회전시키고 나서 벡터값 조정이 필요할 듯
VOID Tiger::Rotate(BOOL clockwise)
{
	D3DXMATRIX rotationMatrix, translationMatrix;
	D3DXVECTOR3 currentPosition = D3DXVECTOR3(m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43);
	// 원점으로 이동
	D3DXMatrixTranslation(&translationMatrix, -currentPosition.x, -currentPosition.y, -currentPosition.z);
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);
	// 3도 회전
	if (clockwise == TRUE)
	{
		D3DXMatrixRotationY(&rotationMatrix, D3DXToRadian(3.0f));
	}
	else
	{
		D3DXMatrixRotationY(&rotationMatrix, D3DXToRadian(-3.0f));
	}
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &rotationMatrix);
	m_rotationAmount += 3;
	// 다시 제자리로 이동
	D3DXMatrixTranslation(&translationMatrix, currentPosition.x, currentPosition.y, currentPosition.z);
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &translationMatrix);
}
