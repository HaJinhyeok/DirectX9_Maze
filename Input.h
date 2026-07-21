#pragma once
#include <Windows.h>

enum class KeyState
{
    Idle,       // 계속 눌리지 않은 상태
    Pressed,    // 이번 프레임에 눌림
    Held,       // 계속 눌린 상태
    Released    // 이번 프레임에 떼어짐
};

static KeyState nKeyState[255];

VOID InitializeInput();
VOID UpdateInput();
BOOL IsKeyPressed(int keyCode);
BOOL IsKeyReleased(int keyCode);
BOOL IsKeyDown(int keyCode);