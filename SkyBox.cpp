#include "SkyBox.h"
#include "ComUtils.h"

SkyBox::SkyBox()
{
	for (int i = 0; i < 6; i++)
		m_boxTextures[i] = NULL;
	m_boxVertexBuffer = NULL;
	// vertex 정보 입력
	{
		// 앞면
		m_boxVertices[0].position = D3DXVECTOR3(-kSkyBoxSize / 2, kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_boxVertices[1].position = D3DXVECTOR3(kSkyBoxSize / 2, kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_boxVertices[2].position = D3DXVECTOR3(kSkyBoxSize / 2, -kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_boxVertices[3].position = D3DXVECTOR3(-kSkyBoxSize / 2, -kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_boxVertices[0].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		m_boxVertices[1].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		m_boxVertices[2].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		m_boxVertices[3].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		// 뒷면
		m_boxVertices[4].position = D3DXVECTOR3(kSkyBoxSize / 2, kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_boxVertices[5].position = D3DXVECTOR3(-kSkyBoxSize / 2, kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_boxVertices[6].position = D3DXVECTOR3(-kSkyBoxSize / 2, -kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_boxVertices[7].position = D3DXVECTOR3(kSkyBoxSize / 2, -kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_boxVertices[4].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		m_boxVertices[5].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		m_boxVertices[6].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		m_boxVertices[7].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		// 왼쪽면
		m_boxVertices[8].position = D3DXVECTOR3(-kSkyBoxSize / 2, kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_boxVertices[9].position = D3DXVECTOR3(-kSkyBoxSize / 2, kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_boxVertices[10].position = D3DXVECTOR3(-kSkyBoxSize / 2, -kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_boxVertices[11].position = D3DXVECTOR3(-kSkyBoxSize / 2, -kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_boxVertices[8].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		m_boxVertices[9].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		m_boxVertices[10].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		m_boxVertices[11].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		// 오른쪽면
		m_boxVertices[12].position = D3DXVECTOR3(kSkyBoxSize / 2, kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_boxVertices[13].position = D3DXVECTOR3(kSkyBoxSize / 2, kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_boxVertices[14].position = D3DXVECTOR3(kSkyBoxSize / 2, -kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_boxVertices[15].position = D3DXVECTOR3(kSkyBoxSize / 2, -kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_boxVertices[12].normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		m_boxVertices[13].normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		m_boxVertices[14].normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		m_boxVertices[15].normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		// 윗면
		m_boxVertices[16].position = D3DXVECTOR3(-kSkyBoxSize / 2, kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_boxVertices[17].position = D3DXVECTOR3(kSkyBoxSize / 2, kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_boxVertices[18].position = D3DXVECTOR3(kSkyBoxSize / 2, kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_boxVertices[19].position = D3DXVECTOR3(-kSkyBoxSize / 2, kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_boxVertices[16].normal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		m_boxVertices[17].normal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		m_boxVertices[18].normal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		m_boxVertices[19].normal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		// 아랫면
		m_boxVertices[20].position = D3DXVECTOR3(-kSkyBoxSize / 2, -kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_boxVertices[21].position = D3DXVECTOR3(kSkyBoxSize / 2, -kSkyBoxSize / 2, kSkyBoxSize / 2);
		m_boxVertices[22].position = D3DXVECTOR3(kSkyBoxSize / 2, -kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_boxVertices[23].position = D3DXVECTOR3(-kSkyBoxSize / 2, -kSkyBoxSize / 2, -kSkyBoxSize / 2);
		m_boxVertices[20].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		m_boxVertices[21].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		m_boxVertices[22].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		m_boxVertices[23].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		// tex 좌표 입력
		for (int i = 0; i < 6; i++)
		{
			m_boxVertices[i * 4].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
			m_boxVertices[i * 4 + 1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
			m_boxVertices[i * 4 + 2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
			m_boxVertices[i * 4 + 3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);
		}
	}
}
SkyBox::~SkyBox()
{
	// texture release
	for (int i = 0; i < 6; i++)
	{
		SafeRelease(m_boxTextures[i]);
	}
	// vertex buffer release
	SafeRelease(m_boxVertexBuffer);
}
VOID SkyBox::LoadTextures()
{
	// 앞 - 뒤 - 좌 - 우 - 상 - 하
	D3DXCreateTextureFromFile(g_pd3dDevice, "Daylight Box_Pieces/Daylight Box_Front.bmp", &m_boxTextures[0]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Daylight Box_Pieces/Daylight Box_Back.bmp", &m_boxTextures[1]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Daylight Box_Pieces/Daylight Box_Left.bmp", &m_boxTextures[2]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Daylight Box_Pieces/Daylight Box_Right.bmp", &m_boxTextures[3]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Daylight Box_Pieces/Daylight Box_Top.bmp", &m_boxTextures[4]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Daylight Box_Pieces/Daylight Box_Bottom.bmp", &m_boxTextures[5]);
}
VOID SkyBox::CreateVertexBuffer()
{
	// vertex buffer 생성
	g_pd3dDevice->CreateVertexBuffer(sizeof(m_boxVertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_boxVertexBuffer, NULL);
	VOID** vertexData;
	m_boxVertexBuffer->Lock(0, sizeof(m_boxVertices), (void**)&vertexData, 0);
	memcpy(vertexData, m_boxVertices, sizeof(m_boxVertices));
	m_boxVertexBuffer->Unlock();
}
VOID SkyBox::Render()
{
	g_pd3dDevice->SetStreamSource(0, m_boxVertexBuffer, 0, sizeof(CustomVertex));
	for (int i = 0; i < 6; i++)
	{
		g_pd3dDevice->SetTexture(0, m_boxTextures[i]);
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * 4, 2);
	}
}
