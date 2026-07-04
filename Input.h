#pragma once
#include <Windows.h>

#define KEY_UP_REP		0 // 키가 계속 안눌리는 상태
#define KEY_DOWN		1 // 키가 눌린 상태 (떼졌다가 눌림)
#define KEY_DOWN_REP	2 // 키가 계속 눌리는 상태
#define KEY_UP			3 // 키가 안눌린 상태(눌렸다가 떼짐)

static int nKeyState[255];

VOID InitInput();
VOID UpdateInput();
BOOL GetKeyDown(int keycode);
BOOL GetKeyUp(int keycode);
BOOL GetKey(int keycode);