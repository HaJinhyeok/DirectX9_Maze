#include "SkyBox.h"
#include "ComUtils.h"

SkyBox::SkyBox()
{
	for (int i = 0; i < 6; i++)
		m_BoxTextures[i] = NULL;
	m_BoxVertexBuffer = NULL;
	// vertex 정보 입력
	{
		// 앞면
		m_BoxVertices[0].v3VerPos = D3DXVECTOR3(-kSkyBoxSize / 2, kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_BoxVertices[1].v3VerPos = D3DXVECTOR3(kSkyBoxSize / 2, kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_BoxVertices[2].v3VerPos = D3DXVECTOR3(kSkyBoxSize / 2, -kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_BoxVertices[3].v3VerPos = D3DXVECTOR3(-kSkyBoxSize / 2, -kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_BoxVertices[0].v3VerNormal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		m_BoxVertices[1].v3VerNormal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		m_BoxVertices[2].v3VerNormal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		m_BoxVertices[3].v3VerNormal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		// 뒷면
		m_BoxVertices[4].v3VerPos = D3DXVECTOR3(kSkyBoxSize / 2, kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_BoxVertices[5].v3VerPos = D3DXVECTOR3(-kSkyBoxSize / 2, kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_BoxVertices[6].v3VerPos = D3DXVECTOR3(-kSkyBoxSize / 2, -kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_BoxVertices[7].v3VerPos = D3DXVECTOR3(kSkyBoxSize / 2, -kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_BoxVertices[4].v3VerNormal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		m_BoxVertices[5].v3VerNormal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		m_BoxVertices[6].v3VerNormal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		m_BoxVertices[7].v3VerNormal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		// 왼쪽면
		m_BoxVertices[8].v3VerPos = D3DXVECTOR3(-kSkyBoxSize / 2, kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_BoxVertices[9].v3VerPos = D3DXVECTOR3(-kSkyBoxSize / 2, kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_BoxVertices[10].v3VerPos = D3DXVECTOR3(-kSkyBoxSize / 2, -kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_BoxVertices[11].v3VerPos = D3DXVECTOR3(-kSkyBoxSize / 2, -kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_BoxVertices[8].v3VerNormal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		m_BoxVertices[9].v3VerNormal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		m_BoxVertices[10].v3VerNormal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		m_BoxVertices[11].v3VerNormal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		// 오른쪽면
		m_BoxVertices[12].v3VerPos = D3DXVECTOR3(kSkyBoxSize / 2, kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_BoxVertices[13].v3VerPos = D3DXVECTOR3(kSkyBoxSize / 2, kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_BoxVertices[14].v3VerPos = D3DXVECTOR3(kSkyBoxSize / 2, -kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_BoxVertices[15].v3VerPos = D3DXVECTOR3(kSkyBoxSize / 2, -kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_BoxVertices[12].v3VerNormal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		m_BoxVertices[13].v3VerNormal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		m_BoxVertices[14].v3VerNormal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		m_BoxVertices[15].v3VerNormal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		// 윗면
		m_BoxVertices[16].v3VerPos = D3DXVECTOR3(-kSkyBoxSize / 2, kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_BoxVertices[17].v3VerPos = D3DXVECTOR3(kSkyBoxSize / 2, kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_BoxVertices[18].v3VerPos = D3DXVECTOR3(kSkyBoxSize / 2, kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_BoxVertices[19].v3VerPos = D3DXVECTOR3(-kSkyBoxSize / 2, kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_BoxVertices[16].v3VerNormal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		m_BoxVertices[17].v3VerNormal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		m_BoxVertices[18].v3VerNormal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		m_BoxVertices[19].v3VerNormal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		// 아랫면
		m_BoxVertices[20].v3VerPos = D3DXVECTOR3(-kSkyBoxSize / 2, -kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_BoxVertices[21].v3VerPos = D3DXVECTOR3(kSkyBoxSize / 2, -kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_BoxVertices[22].v3VerPos = D3DXVECTOR3(kSkyBoxSize / 2, -kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_BoxVertices[23].v3VerPos = D3DXVECTOR3(-kSkyBoxSize / 2, -kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_BoxVertices[20].v3VerNormal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		m_BoxVertices[21].v3VerNormal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		m_BoxVertices[22].v3VerNormal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		m_BoxVertices[23].v3VerNormal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		// tex 좌표 입력
		for (int i = 0; i < 6; i++)
		{
			m_BoxVertices[i * 4].v2VerTex = D3DXVECTOR2(0.0f, 0.0f);
			m_BoxVertices[i * 4 + 1].v2VerTex = D3DXVECTOR2(1.0f, 0.0f);
			m_BoxVertices[i * 4 + 2].v2VerTex = D3DXVECTOR2(1.0f, 1.0f);
			m_BoxVertices[i * 4 + 3].v2VerTex = D3DXVECTOR2(0.0f, 1.0f);
		}
	}
}
SkyBox::~SkyBox()
{
	// texture release
	for (int i = 0; i < 6; i++)
	{
		SafeRelease(m_BoxTextures[i]);
	}
	// vertex buffer release
	SafeRelease(m_BoxVertexBuffer);
}
VOID SkyBox::LoadTextures()
{
	// 앞 - 뒤 - 좌 - 우 - 상 - 하
	D3DXCreateTextureFromFile(g_pd3dDevice, "Daylight Box_Pieces/Daylight Box_Front.bmp", &m_BoxTextures[0]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Daylight Box_Pieces/Daylight Box_Back.bmp", &m_BoxTextures[1]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Daylight Box_Pieces/Daylight Box_Left.bmp", &m_BoxTextures[2]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Daylight Box_Pieces/Daylight Box_Right.bmp", &m_BoxTextures[3]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Daylight Box_Pieces/Daylight Box_Top.bmp", &m_BoxTextures[4]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Daylight Box_Pieces/Daylight Box_Bottom.bmp", &m_BoxTextures[5]);
}
VOID SkyBox::CreateVertexBuffer()
{
	// vertex buffer 생성
	g_pd3dDevice->CreateVertexBuffer(sizeof(m_BoxVertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_BoxVertexBuffer, NULL);
	VOID** SkyBoxVertices;
	m_BoxVertexBuffer->Lock(0, sizeof(m_BoxVertices), (void**)&SkyBoxVertices, 0);
	memcpy(SkyBoxVertices, m_BoxVertices, sizeof(m_BoxVertices));
	m_BoxVertexBuffer->Unlock();
}
VOID SkyBox::Render()
{
	g_pd3dDevice->SetStreamSource(0, m_BoxVertexBuffer, 0, sizeof(CustomVertex));
	for (int i = 0; i < 6; i++)
	{
		g_pd3dDevice->SetTexture(0, m_BoxTextures[i]);
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * 4, 2);
	}
}
