#pragma once
#include "main.h"

class SettingsOverlay
{
private:
	UiVertex m_settingUiVertices[4] =
	{
		D3DXVECTOR3(100.0f, 100.0f, 0.0f), 1.0f, D3DCOLOR_RGBA(200,200,200,50), D3DXVECTOR2(0.0f, 0.0f),
		D3DXVECTOR3(600.0f, 100.0f, 0.0f), 1.0f, D3DCOLOR_RGBA(200,200,200,50), D3DXVECTOR2(1.0f, 0.0f),
		D3DXVECTOR3(600.0f, 600.0f, 0.0f), 1.0f, D3DCOLOR_RGBA(200,200,200,50), D3DXVECTOR2(1.0f, 1.0f),
		D3DXVECTOR3(100.0f, 600.0f, 0.0f), 1.0f, D3DCOLOR_RGBA(200,200,200,50), D3DXVECTOR2(0.0f, 1.0f)
	};

public:
	VOID SetBounds(const RECT& bounds);
	VOID Render(LPDIRECT3DDEVICE9 device);
};

