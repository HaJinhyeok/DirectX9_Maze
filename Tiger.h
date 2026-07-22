#pragma once
#include <d3dx9.h>
#include "main.h"
#include "XFileModel.h"

constexpr float kTigerMoveDistance = 0.2f;
constexpr float kTigerScale = 7.0f;
constexpr float kTigerUpdateIntervalSeconds = 0.01f;

class Tiger
{
private:
	XFileModel m_model;

	// x 파일 객체의 위치와 바라보는 방향
	D3DXVECTOR3 m_position;
	D3DXVECTOR3 m_lookAt;
	D3DXMATRIX m_worldMatrix;
	// x 파일 객체(호랑이)는 일단 맵에 한 마리만 존재하도록 설정
	// 그 한 마리 호랑이가 살아있는지 아닌지 판별
	// 총알로 호랑이 맞춰서 잡으면, 그 시점으로부터 일정 시간 후 호랑이 다시 랜덤 위치에서 리젠되도록 설정
	BOOL m_isAlive;
	BOOL m_isRotating; // 현재 호랑이가 방향전환 중인지 확인
	BOOL m_isClockwise;
	BOOL m_isWallOpen[4]; // 현재 호랑이 위치 기준 앞, 뒤, 좌, 우로 진행 가능한지 확인
	DWORD m_rotationAmount, m_rotationCount;
	FLOAT m_accumulatedTimeSeconds;

	random_device m_randomDevice; // 시드값을 얻기 위한 random_device

public:
	int Render(LPDIRECT3DDEVICE9 device);
	int Load(LPDIRECT3DDEVICE9 device, char* xFilePath);
	VOID Move(const char(*map)[kMazeColumnCount + 1], FLOAT deltaTimeSeconds);
	VOID Rotate(BOOL clockwise);

	VOID SetPosition(D3DXVECTOR3 position)
	{
		m_position = position;
	}
	VOID SetLookAt(D3DXVECTOR3 lookAt)
	{
		m_lookAt = lookAt;
	}
	VOID SetAlive(BOOL isAlive)
	{
		m_isAlive = isAlive;
	}
	D3DXVECTOR3 GetPosition()
	{
		return m_position;
	}
	D3DXVECTOR3 GetLookAt()
	{
		return m_lookAt;
	}
	BOOL IsAlive()
	{
		return m_isAlive;
	}
	D3DXMATRIX& GetWorldMatrix()
	{
		return m_worldMatrix;
	}

	Tiger(D3DXVECTOR3 position);
	virtual ~Tiger();
};

