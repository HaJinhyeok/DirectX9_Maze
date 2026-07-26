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
		// 각 면에 동일한 텍스처 좌표 적용
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
	ReleaseResources();
}

VOID SkyBox::ReleaseResources()
{
	for (int i = 0; i < 6; i++)
	{
		SafeRelease(m_boxTextures[i]);
	}

	SafeRelease(m_boxVertexBuffer);
}

HRESULT SkyBox::LoadTextures(LPDIRECT3DDEVICE9 device)
{
	const char* const texturePaths[] =
	{
		"Daylight Box_Pieces/Daylight Box_Front.bmp",
		"Daylight Box_Pieces/Daylight Box_Back.bmp",
		"Daylight Box_Pieces/Daylight Box_Left.bmp",
		"Daylight Box_Pieces/Daylight Box_Right.bmp",
		"Daylight Box_Pieces/Daylight Box_Top.bmp",
		"Daylight Box_Pieces/Daylight Box_Bottom.bmp"
	};

	for (int i = 0; i < 6; i++)
	{
		SafeRelease(m_boxTextures[i]);

		const HRESULT textureResult = D3DXCreateTextureFromFile(device, texturePaths[i], &m_boxTextures[i]);

		if (FAILED(textureResult))
		{
			for (int releaseIndex = 0; releaseIndex < 6; releaseIndex++)
			{
				SafeRelease(m_boxTextures[releaseIndex]);
			}

			return textureResult;
		}
	}

	return S_OK;
}

HRESULT SkyBox::CreateVertexBuffer(LPDIRECT3DDEVICE9 device)
{
	return CreateManagedVertexBuffer(
		device,
		m_boxVertices,
		sizeof(m_boxVertices),
		D3DFVF_CUSTOMVERTEX,
		&m_boxVertexBuffer);
}

VOID SkyBox::Render(LPDIRECT3DDEVICE9 device)
{
	device->SetStreamSource(0, m_boxVertexBuffer, 0, sizeof(CustomVertex));
	for (int i = 0; i < 6; i++)
	{
		device->SetTexture(0, m_boxTextures[i]);
		device->DrawPrimitive(D3DPT_TRIANGLEFAN, i * 4, 2);
	}
}
