#include "Input.h"

VOID InitInput()
{
	for (int i = 1; i < 255; i++)
		nKeyState[i] = KEY_UP_REP;
}

VOID UpdateInput()
{
	for (int i = 1; i < 255; i++)
	{
		// 키가 안 눌린 상태에서
		if (nKeyState[i] == KEY_UP_REP)
		{
			// 키가 눌리면
			// if ((GetAsyncKeyState(i) & 0x8000) == 0)
			if (GetAsyncKeyState(i) & 0x8000)
				nKeyState[i] = KEY_DOWN;
			// 안 눌리면
			else
				nKeyState[i] = KEY_UP_REP;
		}
		// 키가 눌린 직후에
		else if (nKeyState[i] == KEY_DOWN)
		{
			// 키가 또 눌리면
			if (GetAsyncKeyState(i) & 0x8000)
				nKeyState[i] = KEY_DOWN_REP;
			// 아니면 안 눌린 상태로
			else
				nKeyState[i] = KEY_UP;
		}
		// 키가 눌리는 중에
		else if (nKeyState[i] == KEY_DOWN_REP)
		{
			// 키가 계속 눌리면
			if (GetAsyncKeyState(i) & 0x8000)
				nKeyState[i] = KEY_DOWN_REP;
			// 아니면 안 눌린 상태로
			else
				nKeyState[i] = KEY_UP;
		}
		// 키가 안 눌린 직후에
		else if (nKeyState[i] == KEY_UP)
		{
			// 키가 눌리면
			if (GetAsyncKeyState(i) & 0x8000)
				nKeyState[i] = KEY_DOWN;
			// 아니면 계속 안 눌린 상태로
			else
				nKeyState[i] = KEY_UP_REP;
		}
	}
}
BOOL GetKeyDown(int keycode)
{
	if (nKeyState[keycode] == KEY_DOWN)
		return TRUE;
	else
		return FALSE;
}
BOOL GetKeyUp(int keycode)
{
	if (nKeyState[keycode] == KEY_UP)
		return TRUE;
	else
		return FALSE;
}
BOOL GetKey(int keycode)
{
	if (nKeyState[keycode] == KEY_DOWN || nKeyState[keycode] == KEY_DOWN_REP)
		return TRUE;
	else
		return FALSE;
}