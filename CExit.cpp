#include "CExit.h"

VOID CExit::MakeExit(D3DXVECTOR3 position)
{
	MakeNotice(position);
	// Exit는 notice count하지 않는다.
	CNotice::m_NoticeCount--;
	CNotice::m_bIsNotice = FALSE;
}
VOID CExit::DrawExitButton(LPDIRECT3DDEVICE9 device)
{
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_ExitButtonVertices, sizeof(UI_VERTEX));
}
VOID CExit::ButtonPressed()
{
	bIsPressed = TRUE;
	for (int i = 0; i < 4; i++)
	{
		m_ExitButtonVertices[i].VerColor = BUTTON_PRESSED;
	}
}
VOID CExit::ButtonUnpressed()
{
	bIsPressed = FALSE;
	for (int i = 0; i < 4; i++)
	{
		m_ExitButtonVertices[i].VerColor = BUTTON_DEFAULT;
	}
}
BOOL CExit::IsButtonPressed()
{
	return bIsPressed;
}