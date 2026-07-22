#pragma once
#include "main.h"

class SkyBox
{
private:
	// 스카이박스 각 면의 텍스처와 정점 버퍼
	LPDIRECT3DTEXTURE9 m_boxTextures[6];
	LPDIRECT3DVERTEXBUFFER9 m_boxVertexBuffer;

	CustomVertex m_boxVertices[24];

public:
	SkyBox();
	~SkyBox();
	VOID LoadTextures(LPDIRECT3DDEVICE9 device);
	VOID CreateVertexBuffer(LPDIRECT3DDEVICE9 device);
	VOID Render(LPDIRECT3DDEVICE9 device);
	
};

