#pragma once
#include "Notice.h"

class Exit :
    public Notice
{
private:
    BOOL m_isPressed = FALSE;
    UiVertex m_exitButtonVertices[4] =
    {
        D3DXVECTOR3(300.0f, 450.0f, 0.0f), 1.0f, kButtonDefaultColor, D3DXVECTOR2(0.0f, 0.0f),
        D3DXVECTOR3(400.0f, 450.0f, 0.0f), 1.0f, kButtonDefaultColor, D3DXVECTOR2(1.0f, 0.0f),
        D3DXVECTOR3(400.0f, 500.0f, 0.0f), 1.0f, kButtonDefaultColor, D3DXVECTOR2(1.0f, 1.0f),
        D3DXVECTOR3(300.0f, 500.0f, 0.0f), 1.0f, kButtonDefaultColor, D3DXVECTOR2(0.0f, 1.0f),

    };

public:
    VOID Initialize(D3DXVECTOR3 position);
    VOID RenderButton(LPDIRECT3DDEVICE9 device);
    VOID PressButton();
    VOID ReleaseButton();
    BOOL IsButtonPressed();
    VOID SetButtonBounds(const RECT& bounds);
};

