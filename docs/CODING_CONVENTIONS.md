# Coding Conventions

이 문서는 D3D_MyFPS의 C++ 코드에 적용할 프로젝트별 네이밍 기준이다. 전역 C++ 컨벤션보다 이 문서를 우선하며, Win32와 DirectX 9가 정의한 외부 API 이름은 원형을 유지한다.

## 기본 원칙

- 이름은 타입보다 역할과 의도를 설명한다.
- 동일 범주의 이름은 하나의 표기법을 사용한다.
- 기존 동작을 바꾸지 않는 작은 단위로 이름을 이전한다.
- 예약 식별자인 이중 밑줄과 `_` 뒤 대문자로 시작하는 이름은 사용하지 않는다.
- 직접 관리하는 이름에서는 타입 기반 헝가리안 표기를 새로 사용하지 않는다.

## 네이밍 규칙

| 대상 | 규칙 | 예시 |
| --- | --- | --- |
| 클래스, 구조체, 열거형 | 접두사 없는 `PascalCase` | `Player`, `CustomVertex`, `MoveDirection` |
| 함수 | 동사로 시작하는 `PascalCase` | `MoveBullet`, `GenerateMazeWall` |
| 지역 변수, 매개변수 | `camelCase` | `playerPosition`, `cursorPosition` |
| private 멤버 변수 | `m_` + `camelCase` | `m_playerWorld`, `m_isLightOn` |
| bool | `is`, `has`, `can`, `should` 등 의미 접두사 | `isPaused`, `hasMoved` |
| 임시 전역 변수 | `g_` + `camelCase` | `g_player`, `g_skyBox` |
| 컴파일 시간 상수 | `kPascalCase`와 `constexpr` | `kWindowWidth` |
| 매크로 | `ALL_CAPS` | `D3DFVF_CUSTOMVERTEX` |
| 열거형 값 | `PascalCase` | `MoveDirection::Left` |
| 파일 | 대표 타입과 같은 `PascalCase` | `Player.h`, `Player.cpp` |

약어는 일반 단어처럼 취급한다. 직접 정의하는 이름에서는 `FPS`, `UI`, `D3D` 대신 `Fps`, `Ui`, `D3d`를 사용한다.

## 타입과 접두사

- 기존 클래스의 `C` 접두사는 단계적으로 제거한다.
- `CPlayer`, `CNotice` 같은 프로젝트 타입은 최종적으로 `Player`, `Notice`처럼 표현한다.
- Unreal Engine처럼 도구가 요구하는 타입 접두사는 이 프로젝트에 적용하지 않는다.
- Win32와 DirectX가 제공하는 `HWND`, `DWORD`, `D3DXVECTOR3`, `LPDIRECT3DDEVICE9` 등의 외부 타입 이름은 변경하지 않는다.

## 전역 이름

전역 상태 자체는 `P3-03`에서 축소한다. 그전까지 유지해야 하는 전역 객체에는 `g_`만 붙여 범위를 표시하고 타입 정보는 이름에 넣지 않는다.

```cpp
g_player
g_notices
g_mazeExit
g_settingsOverlay
g_fpsCounter
g_tiger
g_skyBox
```

기존 COM 포인터의 `g_p...` 이름은 소유권 구조를 정리하는 단계까지 임시로 유지한다. 새 전역 포인터에는 같은 패턴을 확장하지 않는다.

## 함수와 상태 이름

- 함수는 수행하는 동작을 나타내는 동사로 시작한다.
- 단순 상태 조회는 `Is`, `Has`, `Can`을 사용한다.
- `Get`과 `Set`은 실제 값 조회와 변경에만 사용한다.
- `Init`, `Update`, `Render`, `Release`처럼 이미 널리 사용된 프로젝트 동사는 같은 의미로 통일한다.
- bool 이름에는 `b` 같은 타입 접두사를 붙이지 않는다.

## 상수와 매크로

- 단순 값과 문자열 경로는 가능한 경우 `constexpr` 상수로 전환한다.
- 전처리 또는 DirectX 매크로 조합이 필요한 경우에만 매크로를 유지한다.
- `ALL_CAPS`는 매크로에만 사용하고 타입이나 열거형 이름에는 사용하지 않는다.

## 적용 예외와 순서

- 외부 API, 라이브러리와 콜백이 요구하는 이름과 시그니처는 유지한다.
- 대량 일괄 치환보다 IDE의 심볼 이름 변경과 빌드 검증을 사용한다.
- 한 Roadmap 항목에서 이름 변경과 구조 리팩터링을 함께 수행하지 않는다.
- 각 단계 후 `Debug|x86` 빌드와 이전 이름 검색을 수행한다.

