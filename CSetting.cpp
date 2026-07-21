#include "CSetting.h"

VOID CSetting::Render(LPDIRECT3DDEVICE9 device)
{
	if (FAILED(device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_Setting_UI_Vertex, sizeof(UI_VERTEX))))
		OutputDebugString("Setting Draw Failed!!!\n");
}