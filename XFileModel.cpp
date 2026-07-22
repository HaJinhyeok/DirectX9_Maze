#pragma warning(push)
#pragma warning(disable:4996)

#include "XFileModel.h"
#include "ComUtils.h"

XFileModel::XFileModel() : m_mesh(nullptr), m_materials(nullptr), m_textures(nullptr), m_materialCount(0)
{
}

XFileModel::~XFileModel()
{
	SafeRelease(m_mesh);

	if (m_textures != nullptr)
	{
		for (DWORD i = 0; i < m_materialCount; i++)
		{
			SafeRelease(m_textures[i]);
		}

		delete[] m_textures;
		m_textures = nullptr;
	}

	delete[] m_materials;
	m_materials = nullptr;
}

int XFileModel::Load(LPDIRECT3DDEVICE9 device, char* xFilePath)
{
	LPD3DXBUFFER materialBuffer;
	if (FAILED(D3DXLoadMeshFromX(xFilePath, D3DXMESH_SYSTEMMEM, device, NULL,
		&materialBuffer, NULL, &m_materialCount, &m_mesh)))
	{
		MessageBox(NULL, "X파일 로드 실패", "메쉬로드 실패", MB_OK);
		return E_FAIL;
	}
	// 텍스쳐 파일이 다른 폴더에 있을 경우를 위하여 텍스쳐 패스 위치 닫기
	char texturePath[256];
	// 현재 폴더의 경우
	if (strchr(xFilePath, '\\') == NULL)
		wsprintf(texturePath, "..\\");
	else
	{
		// 기타 폴더의 경우
		char reversedPath[256], * pathSeparator;
		strcpy(reversedPath, xFilePath);
		_strrev(reversedPath);
		pathSeparator = strchr(reversedPath, '\\');
		strcpy(texturePath, pathSeparator);
		_strrev(texturePath);
	}

	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)materialBuffer->GetBufferPointer();
	m_materials = new D3DMATERIAL9[m_materialCount];
	m_textures = new LPDIRECT3DTEXTURE9[m_materialCount];
	for (DWORD i = 0; i < m_materialCount; i++)
	{
		// 재질 복사
		m_materials[i] = d3dxMaterials[i].MatD3D;
		// D3DX가 Ambient를 설정하지 않으므로 Diffuse 색상으로 보완
		m_materials[i].Ambient = m_materials[i].Diffuse;
		m_textures[i] = NULL;
		// 텍스쳐 파일이 존재하는 경우
		if (d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlen(d3dxMaterials[i].pTextureFilename) > 0)
		{
			// 텍스쳐 생성
			if (FAILED(D3DXCreateTextureFromFile(device, d3dxMaterials[i].pTextureFilename, &m_textures[i])))
			{
				// 경로 + 텍스쳐 파일 이름 만들기
				char fallbackTexturePath[256];
				wsprintf(fallbackTexturePath, "%s%s", texturePath, d3dxMaterials[i].pTextureFilename);
				m_textures[i] = NULL;
				MessageBox(NULL, "Could not find texture map", "D3D_TEST.exe", MB_OK);
			}
		}
	}
	// 재질 버퍼 사용끝 & 해제
	SafeRelease(materialBuffer);
	return S_OK;
}

int XFileModel::Render(LPDIRECT3DDEVICE9 device)
{
	// 각 재질 서브셋에 대응하는 재질과 텍스처를 설정해 렌더링
	for (DWORD i = 0; i < m_materialCount; i++)
	{
		device->SetMaterial(&m_materials[i]);
		device->SetTexture(0, m_textures[i]);
		m_mesh->DrawSubset(i);
	}
	device->SetTexture(0, NULL);

	return 0;
}

#pragma warning(pop)