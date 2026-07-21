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
constexpr float kPlayerMoveDistance = 0.3f;   // 플레이어 이동거리
constexpr float kLookAtDistance = 5.0f;
constexpr float kPlayerRadius = 2.0f;
constexpr float kBulletRadius = 0.4f;
constexpr float kSqrt2 = 1.41421356237f;
constexpr float kRotationAmount = D3DX_PI / 200.0f; // 플레이어 회전각
constexpr float kMouseHorizontalRotationSensitivity = 0.001f; // 좌우 1pixel 당 회전각
constexpr float kMouseVerticalRotationSensitivity = 0.001f; // 상하 1pixel 당 회전각
constexpr int kMazeColumnCount = 12; // 즉, 가로 길이
constexpr int kMazeRowCount = 14;    // 즉, 세로 길이
constexpr float kTileSize = 10.0f;
constexpr float kSkyBoxSize = 500.0f;

// Calculate the norm of 3-D vector
inline FLOAT CalculateLength(D3DXVECTOR3 myVec)
{
    return sqrtf(myVec.x * myVec.x + myVec.y * myVec.y + myVec.z * myVec.z);
}

// Calculate the angle between two 3-D vectors
inline FLOAT CalculateAngle(D3DXVECTOR3 vec1, D3DXVECTOR3 vec2)
{
    FLOAT angle, len1 = CalculateLength(vec1), len2 = CalculateLength(vec2), cos;
    // Calculate angle using dot product
    cos = D3DXVec3Dot(&vec1, &vec2) / (len1 * len2);
    // -1 <= cos <= 1
    cos = min(1.0f, max(-1.0f, cos));
    angle = acosf(cos);
    return angle;
}

struct CustomVertex
{
    D3DXVECTOR3 v3VerPos;       // 정점의 x, y, z 좌표
    D3DXVECTOR3 v3VerNormal;    // 정점의 수직 벡터
    D3DXVECTOR2 v2VerTex;       // 정점의 텍스처 좌표
};

struct UiVertex
{
    D3DXVECTOR3 v3VerPos;
    FLOAT w;
    D3DCOLOR VerColor;
    D3DXVECTOR2 v2VerTex;
};

struct Bullet
{
    D3DXVECTOR3 v3Position;
    D3DXVECTOR3 v3Direction;
    DWORD Time;
};

enum class MoveDirection : WORD
{
    left = 1,
    right,
    front,
    back
};

static UiVertex UIVertices[4] =
{
    D3DXVECTOR3(10.0f, 10.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(255, 0, 0), D3DXVECTOR2(0.0f, 0.0f),
    D3DXVECTOR3(210.0f, 10.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(0, 0, 0), D3DXVECTOR2(1.0f, 0.0f),
    D3DXVECTOR3(210.0f, 135.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(0, 0, 0), D3DXVECTOR2(1.0f, 1.0f),
    D3DXVECTOR3(10.0f, 135.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(255, 0, 0), D3DXVECTOR2(0.0f, 1.0f)
};

static UiVertex Log_UI_Vertices[4] =
{
    D3DXVECTOR3(10.0f, 110.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(255, 255, 255), D3DXVECTOR2(0.0f, 0.0f),
    D3DXVECTOR3(410.0f, 110.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(255, 255, 255), D3DXVECTOR2(1.0f, 0.0f),
    D3DXVECTOR3(410.0f, 200.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(255, 255, 255), D3DXVECTOR2(1.0f, 1.0f),
    D3DXVECTOR3(10.0f, 200.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(255, 255, 255), D3DXVECTOR2(0.0f, 1.0f)
};

// 게임 화면 크기에 따라 달라질 수 있게 위치 다시 찍어야할듯
static UiVertex PopUpVertices[4] =
{
    D3DXVECTOR3(100.0f, 150.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(0, 255, 0), D3DXVECTOR2(0.0f, 0.0f),
    D3DXVECTOR3(600.0f, 150.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(0, 255, 0), D3DXVECTOR2(0.0f, 0.0f),
    D3DXVECTOR3(600.0f, 550.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(0, 255, 0), D3DXVECTOR2(0.0f, 0.0f),
    D3DXVECTOR3(100.0f, 550.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(0, 255, 0), D3DXVECTOR2(0.0f, 0.0f)
};

// 시점 변환 시
const static D3DXVECTOR3 v3EyeCeiling(0.0f, 200.0f, 0.0f);
const static D3DXVECTOR3 v3UpCeiling(0.0f, 0.0f, 1.0f);

// 맵의 정보를 2차원 배열 형태로 저장
const static char chMap1[kMazeRowCount][kMazeColumnCount + 1] =
{
    "X   *@ * @* ",
    "*** ** * ** ",
    " @* ** * ** ",
    " ** ** *    ",
    "    ** **** ",
    " **       * ",
    " **     * * ",
    " **  ** * * ",
    " *   ** * * ",
    " * * ** *   ",
    " * * ** * * ",
    " *   ** * * ",
    " * **** *** ",
    "@*@*        "
};

static char chFileName[] = "tiger.x";

// 시점 변환 확인 변수
static BOOL bIsSkyView = FALSE;
// 자유시점 변환 확인 변수
static BOOL bIsNoClipOn = FALSE;
// 일시정지(or 환경설정) 확인 변수
static BOOL bIsPaused = FALSE;
// 컬링 확인 변수
static BOOL bIsFrustumCulling = TRUE;
// 상호작용 확인 변수
static BOOL bIsInteractive = FALSE;
// 플레이어 이동 여부 확인 변수
static BOOL bIsMoved = FALSE;
// 게임 종료 확인 변수
static BOOL bIsPlaying = TRUE;
// 버튼 클릭 확인 변수
static BOOL bIsClicked = FALSE;
// 낮밤 확인 변수
static BOOL bIsLightOn = FALSE;
// 커서 확인 변수
static SHORT bIsCursorOn = 1;

static D3DXVECTOR3 v3CurrentLookAt(0.0f, 5.0f, 10.0f);
static D3DXVECTOR3 v3LookAt(v3CurrentLookAt);
static D3DXVECTOR3 v3Eye(0.0f, 5.0f, 0.0f);
static D3DXVECTOR3 v3Up(0.0f, 1.0f, 0.0f);
static D3DXVECTOR3 v3DefaultPosition(0.0f, 0.0f, 0.0f);
const static D3DXVECTOR3 v3StartPosition(55.0f, kTileSize / 2, -65.0f);

static CustomVertex TileVertices[4 * kMazeRowCount * kMazeColumnCount];
static CustomVertex WallVertices[4][4 * kMazeRowCount];
static CustomVertex WallVertices2[4][4 * kMazeRowCount];

static CustomVertex MazeWallVertices[72][20];

static WORD wTileIndices[2 * kMazeRowCount * kMazeColumnCount][3];

// 자유 시점으로 전환하기 위해, 현재 플레이어 위치 및 lookat 정보 저장해두기
static D3DXMATRIX mtSavedWorld;
static D3DXVECTOR3 v3SavedLookAt;

//// FRAME ////
static DWORD FPS_Frames = 0;
static DWORD FPS_Num = 0, FPS_LastTime = timeGetTime();
static DWORD FPS_Time;

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
