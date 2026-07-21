#pragma once
#include "main.h"

class Notice
{
	// notice의 position, player와 충돌 상태인지 확인하는 변수
private:
	D3DXVECTOR3 m_position;
	D3DXVECTOR3 m_lookAt;
	D3DXMATRIX m_worldMatrix;
	CustomVertex m_vertices[4];
	LPDIRECT3DVERTEXBUFFER9 m_noticeVertexBuffer = NULL;

public:
	BOOL m_isNotice = TRUE;
	static WORD s_noticeCount;

	VOID Initialize(D3DXVECTOR3 position);
	VOID CreateVertexBuffer(LPDIRECT3DDEVICE9 device);
	VOID UpdateFacing(D3DXVECTOR3 playerPosition);
	VOID Render(LPDIRECT3DDEVICE9 device);
	VOID ReleaseVertexBuffer();

	BOOL CanInteract(D3DXVECTOR3 playerPosition);
	BOOL IsNotice()
	{
		return m_isNotice;
	}
	WORD GetNoticeCount()
	{
		return s_noticeCount;
	}
	D3DXMATRIX GetWorldMatrix()
	{
		return m_worldMatrix;
	}
};

// billboard 형식
// 일단 현재 map1에서 필요한 notice는 5개, exit는 1개이다.
// 미로 생성 시 notice 객체 5개 및 exit 객체 1개(exit는 모든 맵에서 1개로 고정 생성) 생성
// 화면 상에 띄우기
// player가 일정 거리 내에 접근했을 때 상호작용 버튼 활성화