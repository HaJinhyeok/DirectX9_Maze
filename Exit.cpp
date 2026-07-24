#include "Exit.h"

VOID Exit::Initialize(D3DXVECTOR3 position)
{
	Notice::Initialize(position);
	// Exit는 notice count하지 않는다.
	Notice::s_noticeCount--;
	Notice::m_isNotice = FALSE;
}

VOID Exit::RenderButton(LPDIRECT3DDEVICE9 device)
{
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_exitButtonVertices, sizeof(UiVertex));
}

VOID Exit::PressButton()
{
	m_isPressed = TRUE;
	for (int i = 0; i < 4; i++)
	{
		m_exitButtonVertices[i].color = kButtonPressedColor;
	}
}

VOID Exit::ReleaseButton()
{
	m_isPressed = FALSE;
	for (int i = 0; i < 4; i++)
	{
		m_exitButtonVertices[i].color = kButtonDefaultColor;
	}
}

BOOL Exit::IsButtonPressed()
{
	return m_isPressed;
}

VOID Exit::SetButtonBounds(const RECT& bounds)
{
	const FLOAT left = static_cast<FLOAT>(bounds.left);
	const FLOAT top = static_cast<FLOAT>(bounds.top);
	const FLOAT right = static_cast<FLOAT>(bounds.right);
	const FLOAT bottom = static_cast<FLOAT>(bounds.bottom);

	m_exitButtonVertices[0].position.x = left;
	m_exitButtonVertices[0].position.y = top;

	m_exitButtonVertices[1].position.x = right;
	m_exitButtonVertices[1].position.y = top;

	m_exitButtonVertices[2].position.x = right;
	m_exitButtonVertices[2].position.y = bottom;

	m_exitButtonVertices[3].position.x = left;
	m_exitButtonVertices[3].position.y = bottom;
}