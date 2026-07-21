#include "Exit.h"

VOID Exit::Initialize(D3DXVECTOR3 position)
{
	Notice::Initialize(position);
	// Exit는 notice count하지 않는다.
	Notice::m_NoticeCount--;
	Notice::m_bIsNotice = FALSE;
}
VOID Exit::RenderButton(LPDIRECT3DDEVICE9 device)
{
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_ExitButtonVertices, sizeof(UiVertex));
}
VOID Exit::PressButton()
{
	bIsPressed = TRUE;
	for (int i = 0; i < 4; i++)
	{
		m_ExitButtonVertices[i].VerColor = kButtonPressedColor;
	}
}
VOID Exit::ReleaseButton()
{
	bIsPressed = FALSE;
	for (int i = 0; i < 4; i++)
	{
		m_ExitButtonVertices[i].VerColor = kButtonDefaultColor;
	}
}
BOOL Exit::IsButtonPressed()
{
	return bIsPressed;
}