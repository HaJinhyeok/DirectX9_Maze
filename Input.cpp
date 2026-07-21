#include "Input.h"

VOID InitInput()
{
	for (int i = 1; i < 255; i++)
		nKeyState[i] = KeyState::Idle;
}

VOID UpdateInput()
{
	for (int i = 1; i < 255; i++)
	{
		// 키가 안 눌린 상태에서
		if (nKeyState[i] == KeyState::Idle)
		{
			// 키가 눌리면
			// if ((GetAsyncKeyState(i) & 0x8000) == 0)
			if (GetAsyncKeyState(i) & 0x8000)
				nKeyState[i] = KeyState::Pressed;
			// 안 눌리면
			else
				nKeyState[i] = KeyState::Idle;
		}
		// 키가 눌린 직후에
		else if (nKeyState[i] == KeyState::Pressed)
		{
			// 키가 또 눌리면
			if (GetAsyncKeyState(i) & 0x8000)
				nKeyState[i] = KeyState::Held;
			// 아니면 안 눌린 상태로
			else
				nKeyState[i] = KeyState::Released;
		}
		// 키가 눌리는 중에
		else if (nKeyState[i] == KeyState::Held)
		{
			// 키가 계속 눌리면
			if (GetAsyncKeyState(i) & 0x8000)
				nKeyState[i] = KeyState::Held;
			// 아니면 안 눌린 상태로
			else
				nKeyState[i] = KeyState::Released;
		}
		// 키가 안 눌린 직후에
		else if (nKeyState[i] == KeyState::Released)
		{
			// 키가 눌리면
			if (GetAsyncKeyState(i) & 0x8000)
				nKeyState[i] = KeyState::Pressed;
			// 아니면 계속 안 눌린 상태로
			else
				nKeyState[i] = KeyState::Idle;
		}
	}
}
BOOL GetKeyDown(int keycode)
{
	if (nKeyState[keycode] == KeyState::Pressed)
		return TRUE;
	else
		return FALSE;
}
BOOL GetKeyUp(int keycode)
{
	if (nKeyState[keycode] == KeyState::Released)
		return TRUE;
	else
		return FALSE;
}
BOOL GetKey(int keycode)
{
	if (nKeyState[keycode] == KeyState::Pressed || nKeyState[keycode] == KeyState::Held)
		return TRUE;
	else
		return FALSE;
}