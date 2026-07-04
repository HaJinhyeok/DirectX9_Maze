#pragma once
#pragma warning(disable:4996)
#include <d3dx9.h>
#include <time.h>
#include "main.h"

#define TRANSLATION_DISTANCE_TIGER 0.2f
#define ROTATION_AMOUNT_TIGER 3
#define SCALE_AMOUNT_TIGER 7.0f

class CXFileUtil
{
private:
	// x 파일 로드용 변수
	LPD3DXMESH g_pMesh; // 메쉬 객체
	D3DMATERIAL9* g_pMeshMaterials; // 메쉬에 대한 재질
	LPDIRECT3DTEXTURE9* g_pMeshTextures; // 메쉬에 대한 텍스쳐
	DWORD g_dwNumMaterials; // 재질의 수

	// x 파일 객체의 위치와 바라보는 방향
	D3DXVECTOR3 m_Position;
	D3DXVECTOR3 m_LookAt;
	D3DXMATRIX m_TigerWorld;
	// x 파일 객체(호랑이)는 일단 맵에 한 마리만 존재하도록 설정
	// 그 한 마리 호랑이가 살아있는지 아닌지 판별
	// 총알로 호랑이 맞춰서 잡으면, 그 시점으로부터 일정 시간 후 호랑이 다시 랜덤 위치에서 리젠되도록 설정
	BOOL m_IsLive;
	BOOL m_IsRotating; // 현재 호랑이가 방향전환 중인지 확인
	BOOL m_IsClockwise;
	BOOL m_IsWallOpen[4]; // 현재 호랑이 위치 기준 앞, 뒤, 좌, 우로 진행 가능한지 확인
	DWORD m_CurrentTime;
	DWORD m_RotationAmount, m_RotationCount;

	random_device m_Random; // 시드값을 얻기 위한 random_device

public:
	int XFileDisplay(LPDIRECT3DDEVICE9 pD3DDevice);
	int XFileLoad(LPDIRECT3DDEVICE9 pD3DDevice, char* xFileName);
	VOID Move(const char(*map)[NUM_OF_COLUMN + 1]);
	VOID Rotate(BOOL clockwise);

	VOID SetPosition(D3DXVECTOR3 position)
	{
		m_Position = position;
	}
	VOID SetLookAt(D3DXVECTOR3 look)
	{
		m_LookAt = look;
	}
	VOID SetLive(BOOL isLive)
	{
		m_IsLive = isLive;
	}
	D3DXVECTOR3 GetPosition()
	{
		return m_Position;
	}
	D3DXVECTOR3 GetLookAt()
	{
		return m_LookAt;
	}
	BOOL GetLive()
	{
		return m_IsLive;
	}
	D3DXMATRIX& GetTigerWorld()
	{
		return m_TigerWorld;
	}

	CXFileUtil(D3DXVECTOR3 position);
	virtual ~CXFileUtil();
};

