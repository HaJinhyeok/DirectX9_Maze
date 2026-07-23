#pragma once
#pragma comment (lib,"winmm.lib")
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9math.h>
#include <d3dx9shape.h>
#include <wchar.h>
#include <cmath>
#include <vector>
#include <deque>
#include <iterator>
#include <random>

using namespace std;

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define D3DFVF_UI_VERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

constexpr D3DCOLOR kTextureColorKey = D3DCOLOR_ARGB(0xff, 0x00, 0x00, 0x00);
constexpr D3DCOLOR kButtonDefaultColor = D3DCOLOR_XRGB(128, 128, 128);
constexpr D3DCOLOR kButtonPressedColor = D3DCOLOR_XRGB(64, 64, 64);

constexpr char kProgramName[] = "DirectX9_Maze";

// texture file names
constexpr char kTileTexturePath[] = "tex_tile.bmp";
constexpr char kGrassTexturePath[] = "tex_grass.jpg";
constexpr char kWallTexturePath[] = "tex_wall.jpg";
constexpr char kNoticeTexturePath[] = "tex_question.png";
constexpr char kExitTexturePath[] = "tex_exit.png";
constexpr char kSkyBoxTexturePath[] = "SkyBox1.dds";

// constant values
constexpr int kWindowWidth = 700;
constexpr int kWindowHeight = 700;
constexpr float kEpsilon = 0.001f;
constexpr float kLookAtDistance = 5.0f;
constexpr float kPlayerRadius = 2.0f;
constexpr float kBulletRadius = 0.4f;
constexpr float kSqrt2 = 1.41421356237f;
constexpr float kPlayerMoveSpeed = 30.0f;                       // 초당 이동 거리
constexpr float kPlayerRotationSpeed = D3DX_PI / 2.0f;          // 초당 90도 회전
constexpr float kMouseHorizontalRotationSensitivity = 0.001f;   // 좌우 1pixel 당 회전각
constexpr float kMouseVerticalRotationSensitivity = 0.001f;     // 상하 1pixel 당 회전각
constexpr int kMazeColumnCount = 12;                            // 미로의 가로 칸 수
constexpr int kMazeRowCount = 14;                               // 미로의 세로 칸 수
constexpr float kTileSize = 10.0f;
constexpr float kSkyBoxSize = 500.0f;
constexpr float kMaxDeltaTimeSeconds = 0.1f;

// Calculate the norm of 3-D vector
inline FLOAT CalculateLength(D3DXVECTOR3 vectorValue)
{
    return sqrtf(vectorValue.x * vectorValue.x + vectorValue.y * vectorValue.y + vectorValue.z * vectorValue.z);
}

// Calculate the angle between two 3-D vectors
inline FLOAT CalculateAngle(D3DXVECTOR3 firstVector, D3DXVECTOR3 secondVector)
{
    FLOAT angle, firstLength = CalculateLength(firstVector), secondLength = CalculateLength(secondVector), cosine;
    // Calculate angle using dot product
    cosine = D3DXVec3Dot(&firstVector, &secondVector) / (firstLength * secondLength);
    // -1 <= cosine <= 1
    cosine = min(1.0f, max(-1.0f, cosine));
    angle = acosf(cosine);
    return angle;
}

struct CustomVertex
{
    D3DXVECTOR3 position;               // 정점의 x, y, z 좌표
    D3DXVECTOR3 normal;                 // 정점의 수직 벡터
    D3DXVECTOR2 textureCoordinate;      // 정점의 텍스처 좌표
};

struct UiVertex
{
    D3DXVECTOR3 position;
    FLOAT rhw;
    D3DCOLOR color;
    D3DXVECTOR2 textureCoordinate;
};

struct Bullet
{
    D3DXVECTOR3 position;
    D3DXVECTOR3 direction;
};

enum class MoveDirection : WORD
{
    Left = 1,
    Right,
    Forward,
    Backward
};

extern const D3DXVECTOR3 kWorldUp;
extern const D3DXVECTOR3 kPlayerStartPosition;

// tile culling 수정: 정사각형 중심으로부터 거리가 변의 길이의 절반 이하(d <= kTileSize / 2) culling 해주어야 함.
// 추가할 기능: 플레이어 시점이 qe가 아닌 마우스 움직임에 따라 변하면 좋을 듯? ==> 창모드에서는 뭔가뭔가임 창 밖에서 마우스 움직임 제어는 어케

//// 미로 찾기 게임
// 맵은 한 개, 많아야 두 개(만들기 번거로울 듯?)
// 손전등 기능?(플레이어 위치 기준, 바라보는 방향으로 spot light)

//// 필요한 객체
// <player>
// 플레이어. 위치, 바라보는 방향, 손전등 on/off
// 
// <안내문>
// 미로 막다른 골목에서 상호작용(G) 가능한 객체.
// 상호작용 시, 카메라 시점을 하늘에서 내려다보게 바꾸어 맵을 볼 수 있게 해줌.
// 안내문이 바라보고 있는 방향을 벡터값으로 갖고 있는다.(v3LookAt)
// (플레이어 위치) - (안내문 위치) 벡터(v3Distance) 활용
// ( |v3Distance| <= (특정 거리) && v3LookAt X v3Distance > 0 ) 이면 상호작용 활성화
// 
// <출구> - <안내문>의 child class
// 미로의 끝에서 상호작용(G) 가능한 객체.
// 상호작용 시, 게임 종료?
