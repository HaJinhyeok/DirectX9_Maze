#pragma once
#include "CNotice.h"

class CExit :
    public CNotice
{
private:
    BOOL bIsPressed = FALSE;
    UI_VERTEX m_ExitButtonVertices[4] =
    {
        D3DXVECTOR3(300.0f, 450.0f, 0.0f), 1.0f, kButtonDefaultColor, D3DXVECTOR2(0.0f, 0.0f),
        D3DXVECTOR3(400.0f, 450.0f, 0.0f), 1.0f, kButtonDefaultColor, D3DXVECTOR2(1.0f, 0.0f),
        D3DXVECTOR3(400.0f, 500.0f, 0.0f), 1.0f, kButtonDefaultColor, D3DXVECTOR2(1.0f, 1.0f),
        D3DXVECTOR3(300.0f, 500.0f, 0.0f), 1.0f, kButtonDefaultColor, D3DXVECTOR2(0.0f, 1.0f),

    };

public:
    VOID MakeExit(D3DXVECTOR3 position);
    VOID DrawExitButton(LPDIRECT3DDEVICE9 device);
    VOID ButtonPressed();
    VOID ButtonUnpressed();
    BOOL IsButtonPressed();
};

