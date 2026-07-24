#include "SettingsOverlay.h"

VOID SettingsOverlay::SetBounds(const RECT& bounds)
{
	const FLOAT left = static_cast<FLOAT>(bounds.left);
	const FLOAT top = static_cast<FLOAT>(bounds.top);
	const FLOAT right = static_cast<FLOAT>(bounds.right);
	const FLOAT bottom = static_cast<FLOAT>(bounds.bottom);

	m_settingUiVertices[0].position.x = left;
	m_settingUiVertices[0].position.y = top;

	m_settingUiVertices[1].position.x = right;
	m_settingUiVertices[1].position.y = top;

	m_settingUiVertices[2].position.x = right;
	m_settingUiVertices[2].position.y = bottom;

	m_settingUiVertices[3].position.x = left;
	m_settingUiVertices[3].position.y = bottom;
}

VOID SettingsOverlay::Render(LPDIRECT3DDEVICE9 device)
{
	if (FAILED(device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_settingUiVertices, sizeof(UiVertex))))
		OutputDebugString("Setting Draw Failed!!!\n");
}