#include "Tiger.h"
#include "ComUtils.h"

Tiger::Tiger(D3DXVECTOR3 position)
{
	g_pMesh = NULL; // 메쉬 객체
	g_pMeshMaterials = NULL; // 메쉬에 대한 재질
	g_pMeshTextures = NULL; // 메쉬에 대한 텍스쳐
	g_dwNumMaterials = 0L; // 메쉬 재질의 개수

	// scale 먼저 하고 translation
	D3DXMatrixScaling(&m_TigerWorld, kTigerScale, kTigerScale, kTigerScale * 2.0f / 3.0f);
	D3DXMATRIX tmpMat;
	D3DXMatrixTranslation(&tmpMat, position.x, position.y, position.z);
	D3DXMatrixMultiply(&m_TigerWorld, &m_TigerWorld, &tmpMat);
	m_IsLive = TRUE;
	m_IsRotating = FALSE;
	m_IsClockwise = TRUE;
	for (int i = 0; i < 4; i++)
	{
		m_IsWallOpen[i] = TRUE;
	}
	m_CurrentTime = timeGetTime();
	m_RotationAmount = 0;
	m_RotationCount = 0;
}
Tiger::~Tiger()
{
	SafeRelease(g_pMesh);
	if (g_pMeshTextures != NULL)
	{
		for (DWORD i = 0; i < g_dwNumMaterials; i++)
		{
			SafeRelease(g_pMeshTextures[i]);
		}
		delete[] g_pMeshTextures;
		g_pMeshTextures = nullptr;
	}
	delete[] g_pMeshMaterials;
	g_pMeshMaterials = nullptr;
}

//---------------------------------------------------------------------------------
// 이름 : Load()
// 기능 : x 파일을 로드 한다.
//---------------------------------------------------------------------------------
int Tiger::Load(LPDIRECT3DDEVICE9 pD3DDevice, char* xFileName)
{
	LPD3DXBUFFER pD3DXMtrlBuffer;
	// x 파일을 로딩한다.
	if (FAILED(D3DXLoadMeshFromX(xFileName, D3DXMESH_SYSTEMMEM, pD3DDevice, NULL,
		&pD3DXMtrlBuffer, NULL, &g_dwNumMaterials, &g_pMesh)))
	{
		MessageBox(NULL, "X파일 로드 실패", "메쉬로드 실패", MB_OK);
		return E_FAIL;
	}
	// 텍스쳐 파일이 다른 폴더에 있을 경우를 위하여 텍스쳐 패스 위치 닫기
	char texturePath[256];
	// 현재 폴더의 경우
	if (strchr(xFileName, '\\') == NULL)
		wsprintf(texturePath, "..\\");
	else
	{
		// 기타 폴더의 경우
		char temp[256], * pChar;
		strcpy(temp, xFileName);
		_strrev(temp);
		pChar = strchr(temp, '\\');
		strcpy(texturePath, pChar);
		_strrev(texturePath);
	}
	// x 파일 로딩 코드
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
	g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
	for (DWORD i = 0; i < g_dwNumMaterials; i++)
	{
		// 재질 복사
		g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;
		// 재질에 대한 앰비언트 색상 설정(D3DX 가 해주지 않으므로)
		g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;
		g_pMeshTextures[i] = NULL;
		// 텍스쳐 파일이 존재하는 경우
		if (d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlen(d3dxMaterials[i].pTextureFilename) > 0)
		{
			// 텍스쳐 생성
			if (FAILED(D3DXCreateTextureFromFile(pD3DDevice, d3dxMaterials[i].pTextureFilename, &g_pMeshTextures[i])))
			{
				// 경로 + 텍스쳐 파일 이름 만들기
				char tempFile[256];
				wsprintf(tempFile, "%s%s", texturePath, d3dxMaterials[i].pTextureFilename);
				g_pMeshTextures[i] = NULL;
				MessageBox(NULL, "Could not find texture map", "D3D_TEST.exe", MB_OK);
			}
		}
	}
	// 재질 버퍼 사용끝 & 해제
	SafeRelease(pD3DXMtrlBuffer);
	return S_OK;
}

//---------------------------------------------------------------------------------
// 이름 : Render()
// 기능 : x 파일을 출력 해준다.
//---------------------------------------------------------------------------------
int Tiger::Render(LPDIRECT3DDEVICE9 pD3DDevice)
{
	// 메쉬 출력
	for (DWORD i = 0; i < g_dwNumMaterials; i++)
	{
		pD3DDevice->SetMaterial(&g_pMeshMaterials[i]);
		// 현재 sub Set에 대한 재질 설정
		pD3DDevice->SetTexture(0, g_pMeshTextures[i]);
		// Draw the mesh subset
		g_pMesh->DrawSubset(i);
	}
	pD3DDevice->SetTexture(0, NULL);

	return 0;
}

VOID Tiger::Move(const char(*map)[kMazeColumnCount + 1])
{
	DWORD currentTime = timeGetTime();
	if (currentTime - m_CurrentTime >= 10)
	{
		// 호랑이가 통로를 따라 움직이게 한다.
		// 호랑이가 현재 바라보는 방향 정보와 맵 정보를 이용해서 현재 방향으로 더 전진해도 되는지 여부 판단
		// 현재 바라보는 방향이 벽으로 막혀있을 경우, 시계방향으로 90도씩 회전시켜 다시 전진 여부 판단
		// 현재 바라보는 방향이 막혀있지 않더라도, 여러 갈래 길일 경우 랜덤하게 진행방향 바꾸도록 해볼까
		// => 랜덤(50%?)으로 방향 전환할 지 안 할 지 결정하는
		D3DXVECTOR3 currentLookAt = D3DXVECTOR3(-m_TigerWorld._31, -m_TigerWorld._32, -m_TigerWorld._33);
		D3DXVec3Normalize(&currentLookAt, &currentLookAt);
		D3DXVECTOR3 currentPosition = D3DXVECTOR3(m_TigerWorld._41, m_TigerWorld._42, m_TigerWorld._43);
		FLOAT tmpX = currentPosition.x / kTileSize, tmpZ = currentPosition.z / kTileSize;
		tmpX -= floorf(tmpX);
		tmpZ -= floorf(tmpZ);
		// 우선, 현재 호랑이가 블록 정가운데 위치해있는지 확인
		if (tmpX <= 0.5f + kEpsilon && tmpX >= 0.5f - kEpsilon)
		{
			if (tmpZ <= 0.50f + kEpsilon && tmpZ >= 0.5f - kEpsilon)
			{
				//OutputDebugString("center of block\n");
				if (m_IsRotating == TRUE)
				{
					this->Rotate(m_IsClockwise);
				}
				else if (m_IsRotating == FALSE && m_RotationCount == 1)
				{
					m_RotationCount--;
				}
				else
				{
					// 현재 좌표
					int nCoX = static_cast<int>(floorf(currentPosition.x / kTileSize)) + kMazeColumnCount / 2;
					int nCoZ = kMazeRowCount / 2 - static_cast<int>(floorf(currentPosition.z / kTileSize)) - 1;
					// random_device로 시드값 생성해 메르센 트위스트 알고리즘으로 난수 생성
					mt19937 m_Engine(m_Random());

					// 상하좌우 칸으로 전진 가능한지 판별
					{
						// 0 - 상
						if (nCoZ == 0)
						{
							m_IsWallOpen[0] = FALSE;
						}
						else if (map[nCoZ - 1][nCoX] == '*')
						{
							m_IsWallOpen[0] = FALSE;
						}
						else
						{
							m_IsWallOpen[0] = TRUE;
						}
						// 1 - 하
						if (nCoZ == kMazeRowCount - 1)
						{
							m_IsWallOpen[1] = FALSE;
						}
						else if (map[nCoZ + 1][nCoX] == '*')
						{
							m_IsWallOpen[1] = FALSE;
						}
						else
						{
							m_IsWallOpen[1] = TRUE;
						}
						// 2 - 좌
						if (nCoX == 0)
						{
							m_IsWallOpen[2] = FALSE;
						}
						else if (map[nCoZ][nCoX - 1] == '*')
						{
							m_IsWallOpen[2] = FALSE;
						}

						else
						{
							m_IsWallOpen[2] = TRUE;
						}
						// 3 - 우
						if (nCoX == kMazeColumnCount - 1)
						{
							m_IsWallOpen[3] = FALSE;
						}
						else if (map[nCoZ][nCoX + 1] == '*')
						{
							m_IsWallOpen[3] = FALSE;
						}
						else
						{
							m_IsWallOpen[3] = TRUE;
						}
					}

					// 진행 방향이 막혀있을 경우
					// 진행 방향이 열려있고, 왼쪽과 오른쪽이 막혀있을 경우 == 일단은 그냥 직진
					// 진행 방향이 열려있고, 왼쪽 또는 오른쪽도 열려있을 경우 == 일단은 회전, 몇 갈래인지 카운트해서 랜덤하게 회전시켜 진행
					if (currentLookAt.x > 0 && fabsf(currentLookAt.z) <= kEpsilon)
					{
						// +x direction
						if (m_IsWallOpen[3] == FALSE)
						{
							// 전진 막혔으니 일단 무조건 회전은 함
							m_IsRotating = TRUE;
							m_RotationCount++;
							if (!m_IsWallOpen[0] && m_IsWallOpen[1])
							{
								m_IsClockwise = TRUE;
							}
							else if (m_IsWallOpen[0] && !m_IsWallOpen[1])
							{
								m_IsClockwise = FALSE;
							}
							else if (m_IsWallOpen[0] && m_IsWallOpen[1])
							{
								uniform_int_distribution<INT> dis(0, 1);
								if (dis(m_Engine) == 0)
								{
									m_IsClockwise = TRUE;
								}
								else
								{
									m_IsClockwise = FALSE;
								}
							}
							else
							{
								// 양쪽 다 막혔으므로 회전 2회
								m_RotationCount++;
							}
						}
						else
						{
							if (!m_IsWallOpen[0] && !m_IsWallOpen[1])
							{
								m_IsRotating = FALSE;
							}
							else if (!m_IsWallOpen[0] && m_IsWallOpen[1])
							{
								uniform_int_distribution<INT> dis(0, 1);
								if (dis(m_Engine) == 0)
								{
									m_IsRotating = FALSE;
								}
								else
								{
									m_IsRotating = TRUE;
									m_IsClockwise = TRUE;
									m_RotationCount++;
								}
							}
							else if (m_IsWallOpen[0] && !m_IsWallOpen[1])
							{
								uniform_int_distribution<INT> dis(0, 1);
								if (dis(m_Engine) == 0)
								{
									m_IsRotating = FALSE;
								}
								else
								{
									m_IsRotating = TRUE;
									m_IsClockwise = FALSE;
									m_RotationCount++;
								}
							}
							else
							{
								uniform_int_distribution<INT> dis(0, 2);
								if (dis(m_Engine) == 0)
								{
									m_IsRotating = FALSE;
								}
								else if (dis(m_Engine) == 1)
								{
									m_IsRotating = TRUE;
									m_IsClockwise = TRUE;
									m_RotationCount++;
								}
								else
								{
									m_IsRotating = TRUE;
									m_IsClockwise = FALSE;
									m_RotationCount++;
								}
							}
						}
					}
					else if (currentLookAt.x < 0 && fabsf(currentLookAt.z) <= kEpsilon)
					{
						// -x direction
						if (m_IsWallOpen[2] == FALSE)
						{
							// 전진 막혔으니 일단 무조건 회전은 함
							m_IsRotating = TRUE;
							m_RotationCount++;
							if (!m_IsWallOpen[1] && m_IsWallOpen[0])
							{
								m_IsClockwise = TRUE;
							}
							else if (m_IsWallOpen[1] && !m_IsWallOpen[0])
							{
								m_IsClockwise = FALSE;
							}
							else if (m_IsWallOpen[1] && m_IsWallOpen[0])
							{
								uniform_int_distribution<INT> dis(0, 1);
								if (dis(m_Engine) == 0)
								{
									m_IsClockwise = TRUE;
								}
								else
								{
									m_IsClockwise = FALSE;
								}
							}
							else
							{
								m_RotationCount++;
							}
						}
						else
						{
							if (!m_IsWallOpen[1] && !m_IsWallOpen[0])
							{
								m_IsRotating = FALSE;
							}
							else if (!m_IsWallOpen[1] && m_IsWallOpen[0])
							{
								uniform_int_distribution<INT> dis(0, 1);
								if (dis(m_Engine) == 0)
								{
									m_IsRotating = FALSE;
								}
								else
								{
									m_IsRotating = TRUE;
									m_IsClockwise = TRUE;
									m_RotationCount++;
								}
							}
							else if (m_IsWallOpen[1] && !m_IsWallOpen[0])
							{
								uniform_int_distribution<INT> dis(0, 1);
								if (dis(m_Engine) == 0)
								{
									m_IsRotating = FALSE;
								}
								else
								{
									m_IsRotating = TRUE;
									m_IsClockwise = FALSE;
									m_RotationCount++;
								}
							}
							else
							{
								uniform_int_distribution<INT> dis(0, 2);
								if (dis(m_Engine) == 0)
								{
									m_IsRotating = FALSE;
								}
								else if (dis(m_Engine) == 1)
								{
									m_IsRotating = TRUE;
									m_IsClockwise = TRUE;
									m_RotationCount++;
								}
								else
								{
									m_IsRotating = TRUE;
									m_IsClockwise = FALSE;
									m_RotationCount++;
								}
							}
						}
					}
					else if (currentLookAt.z > 0 && fabsf(currentLookAt.x) <= kEpsilon)
					{
						// +z direction
						if (m_IsWallOpen[0] == FALSE)
						{
							// 전진 막혔으니 일단 무조건 회전은 함
							m_IsRotating = TRUE;
							m_RotationCount++;
							if (!m_IsWallOpen[2] && m_IsWallOpen[3])
							{
								m_IsClockwise = TRUE;
							}
							else if (m_IsWallOpen[2] && !m_IsWallOpen[3])
							{
								m_IsClockwise = FALSE;
							}
							else if (m_IsWallOpen[2] && m_IsWallOpen[3])
							{
								uniform_int_distribution<INT> dis(0, 1);
								if (dis(m_Engine) == 0)
								{
									m_IsClockwise = TRUE;
								}
								else
								{
									m_IsClockwise = FALSE;
								}
							}
							else
							{
								// 양쪽 다 막혔으므로 회전 2회
								m_RotationCount++;
							}
						}
						else
						{
							if (!m_IsWallOpen[2] && !m_IsWallOpen[3])
							{
								m_IsRotating = FALSE;
							}
							else if (!m_IsWallOpen[2] && m_IsWallOpen[3])
							{
								uniform_int_distribution<INT> dis(0, 1);
								if (dis(m_Engine) == 0)
								{
									m_IsRotating = FALSE;
								}
								else
								{
									m_IsRotating = TRUE;
									m_IsClockwise = TRUE;
									m_RotationCount++;
								}
							}
							else if (m_IsWallOpen[2] && !m_IsWallOpen[3])
							{
								uniform_int_distribution<INT> dis(0, 1);
								if (dis(m_Engine) == 0)
								{
									m_IsRotating = FALSE;
								}
								else
								{
									m_IsRotating = TRUE;
									m_IsClockwise = FALSE;
									m_RotationCount++;
								}
							}
							else
							{
								uniform_int_distribution<INT> dis(0, 2);
								if (dis(m_Engine) == 0)
								{
									m_IsRotating = FALSE;
								}
								else if (dis(m_Engine) == 1)
								{
									m_IsRotating = TRUE;
									m_IsClockwise = TRUE;
									m_RotationCount++;
								}
								else
								{
									m_IsRotating = TRUE;
									m_IsClockwise = FALSE;
									m_RotationCount++;
								}
							}
						}
					}
					else if (currentLookAt.z < 0 && fabsf(currentLookAt.x) <= kEpsilon)
					{
						// -z direction
						if (m_IsWallOpen[1] == FALSE)
						{
							// 전진 막혔으니 일단 무조건 회전은 함
							m_IsRotating = TRUE;
							m_RotationCount++;
							if (!m_IsWallOpen[3] && m_IsWallOpen[2])
							{
								m_IsClockwise = TRUE;
							}
							else if (m_IsWallOpen[3] && !m_IsWallOpen[2])
							{
								m_IsClockwise = FALSE;
							}
							else if (m_IsWallOpen[3] && m_IsWallOpen[2])
							{
								uniform_int_distribution<INT> dis(0, 1);
								if (dis(m_Engine) == 0)
								{
									m_IsClockwise = TRUE;
								}
								else
								{
									m_IsClockwise = FALSE;
								}
							}
							else
							{
								// 양쪽 다 막혔으므로 회전 2회
								m_RotationCount++;
							}
						}
						else
						{
							// 전진이 불가능하진 않으므로 회전은 하더라도 1회만
							if (!m_IsWallOpen[3] && !m_IsWallOpen[2])
							{
								// 양쪽 막히면 그냥 직진
								m_IsRotating = FALSE;
							}
							else if (!m_IsWallOpen[3] && m_IsWallOpen[2])
							{
								uniform_int_distribution<INT> dis(0, 1);
								if (dis(m_Engine) == 0)
								{
									m_IsRotating = FALSE;
								}
								else
								{
									m_IsRotating = TRUE;
									m_IsClockwise = TRUE;
									m_RotationCount++;
								}
							}
							else if (m_IsWallOpen[3] && !m_IsWallOpen[2])
							{
								uniform_int_distribution<INT> dis(0, 1);
								if (dis(m_Engine) == 0)
								{
									m_IsRotating = FALSE;
								}
								else
								{
									m_IsRotating = TRUE;
									m_IsClockwise = FALSE;
									m_RotationCount++;
								}
							}
							else
							{
								uniform_int_distribution<INT> dis(0, 2);
								if (dis(m_Engine) == 0)
								{
									m_IsRotating = FALSE;
								}
								else if (dis(m_Engine) == 1)
								{
									m_IsRotating = TRUE;
									m_IsClockwise = TRUE;
									m_RotationCount++;
								}
								else
								{
									m_IsRotating = TRUE;
									m_IsClockwise = FALSE;
									m_RotationCount++;
								}
							}
						}
					}
				}

			}
		}
		if (m_RotationAmount == 90)
		{
			if (m_RotationCount == 2)
			{
				m_RotationCount--;
			}
			else
			{
				m_IsRotating = FALSE;
			}
			// 90도 회전 후 벡터값 조정?
			m_RotationAmount = 0;
		}
		else if (m_IsRotating == FALSE)
		{
			D3DXMATRIX tmpTranslation;
			FLOAT fCoefficient = kTigerMoveDistance;
			fCoefficient /= sqrtf(currentLookAt.x * currentLookAt.x + currentLookAt.y * currentLookAt.y + currentLookAt.z * currentLookAt.z);
			D3DXMatrixTranslation(&tmpTranslation, currentLookAt.x * fCoefficient, currentLookAt.y * fCoefficient, currentLookAt.z * fCoefficient);
			D3DXMatrixMultiply(&m_TigerWorld, &m_TigerWorld, &tmpTranslation);
			m_CurrentTime = currentTime;
		}
	}
}
// 90도 회전시키고 나서 벡터값 조정이 필요할 듯
// 
VOID Tiger::Rotate(BOOL clockwise)
{
	D3DXMATRIX tmpRotation, tmpTranslation;
	D3DXVECTOR3 tmpPosition = D3DXVECTOR3(m_TigerWorld._41, m_TigerWorld._42, m_TigerWorld._43);
	// 원점으로 이동
	D3DXMatrixTranslation(&tmpTranslation, -tmpPosition.x, -tmpPosition.y, -tmpPosition.z);
	D3DXMatrixMultiply(&m_TigerWorld, &m_TigerWorld, &tmpTranslation);
	// 3도 회전
	if (clockwise == TRUE)
	{
		D3DXMatrixRotationY(&tmpRotation, D3DXToRadian(3.0f));
	}
	else
	{
		D3DXMatrixRotationY(&tmpRotation, D3DXToRadian(-3.0f));
	}
	D3DXMatrixMultiply(&m_TigerWorld, &m_TigerWorld, &tmpRotation);
	m_RotationAmount += 3;
	// 다시 제자리로 이동
	D3DXMatrixTranslation(&tmpTranslation, tmpPosition.x, tmpPosition.y, tmpPosition.z);
	D3DXMatrixMultiply(&m_TigerWorld, &m_TigerWorld, &tmpTranslation);
}
