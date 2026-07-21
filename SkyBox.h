#pragma once
#include "main.h"
extern LPDIRECT3DDEVICE9 g_pd3dDevice;

class SkyBox
{
private:
	// skybox에 입힐 texture와 위치를 담을 vertex buffer
	LPDIRECT3DTEXTURE9 m_boxTextures[6];
	LPDIRECT3DVERTEXBUFFER9 m_boxVertexBuffer;

	CustomVertex m_boxVertices[24];

public:
	SkyBox();
	~SkyBox();
	VOID LoadTextures();
	VOID CreateVertexBuffer();
	VOID Render();
	
};

