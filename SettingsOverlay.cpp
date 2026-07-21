#include "SettingsOverlay.h"

VOID SettingsOverlay::Render(LPDIRECT3DDEVICE9 device)
{
	if (FAILED(device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_settingUiVertices, sizeof(UiVertex))))
		OutputDebugString("Setting Draw Failed!!!\n");
}