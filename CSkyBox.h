#pragma once
#include "main.h"
extern LPDIRECT3DDEVICE9 g_pd3dDevice;

class CSkyBox
{
private:
	// skybox에 입힐 texture와 위치를 담을 vertex buffer
	LPDIRECT3DTEXTURE9 m_BoxTextures[6];
	LPDIRECT3DVERTEXBUFFER9 m_BoxVertexBuffer;

	CUSTOMVERTEX m_BoxVertices[24];

public:
	CSkyBox();
	~CSkyBox();
	VOID LoadTextures();
	VOID CreateVertexBuffer();
	VOID Render();
	
};

