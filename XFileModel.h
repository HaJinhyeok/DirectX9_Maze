#pragma once

#include <d3dx9.h>

class XFileModel
{
private:
    LPD3DXMESH m_mesh;
    D3DMATERIAL9* m_materials;
    LPDIRECT3DTEXTURE9* m_textures;
    DWORD m_materialCount;

public:
    XFileModel();
    ~XFileModel();

    XFileModel(const XFileModel&) = delete;
    XFileModel& operator=(const XFileModel&) = delete;

    int Load(LPDIRECT3DDEVICE9 device, char* xFilePath);
    int Render(LPDIRECT3DDEVICE9 device);
};