#include "CExit.h"

VOID CExit::Initialize(D3DXVECTOR3 position)
{
	CNotice::Initialize(position);
	// Exit는 notice count하지 않는다.
	CNotice::m_NoticeCount--;
	CNotice::m_bIsNotice = FALSE;
}
VOID CExit::RenderButton(LPDIRECT3DDEVICE9 device)
{
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_ExitButtonVertices, sizeof(UI_VERTEX));
}
VOID CExit::PressButton()
{
	bIsPressed = TRUE;
	for (int i = 0; i < 4; i++)
	{
		m_ExitButtonVertices[i].VerColor = kButtonPressedColor;
	}
}
VOID CExit::ReleaseButton()
{
	bIsPressed = FALSE;
	for (int i = 0; i < 4; i++)
	{
		m_ExitButtonVertices[i].VerColor = kButtonDefaultColor;
	}
}
BOOL CExit::IsButtonPressed()
{
	return bIsPressed;
}