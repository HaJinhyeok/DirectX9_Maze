
#include "MazeGenerator.h"
#include "Input.h"
#include "Frustum.h"
#include "FpsCounter.h"
#include "SkyBox.h"
#include "Tiger.h"
#include "ComUtils.h"

const char kMazeMap[kMazeRowCount][kMazeColumnCount + 1] =
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

const D3DXVECTOR3 kWorldUp(0.0f, 1.0f, 0.0f);
const D3DXVECTOR3 kPlayerStartPosition(55.0f, kTileSize / 2, -65.0f);

static UiVertex g_uiVertices[4] =
{
	D3DXVECTOR3(10.0f, 10.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(255, 0, 0), D3DXVECTOR2(0.0f, 0.0f),
	D3DXVECTOR3(210.0f, 10.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(0, 0, 0), D3DXVECTOR2(1.0f, 0.0f),
	D3DXVECTOR3(210.0f, 135.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(0, 0, 0), D3DXVECTOR2(1.0f, 1.0f),
	D3DXVECTOR3(10.0f, 135.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(255, 0, 0), D3DXVECTOR2(0.0f, 1.0f)
};

// TODO(P3-10): 창 크기 변경 시 팝업 좌표를 다시 계산한다.
static UiVertex g_popupVertices[4] =
{
	D3DXVECTOR3(100.0f, 150.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(0, 255, 0), D3DXVECTOR2(0.0f, 0.0f),
	D3DXVECTOR3(600.0f, 150.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(0, 255, 0), D3DXVECTOR2(0.0f, 0.0f),
	D3DXVECTOR3(600.0f, 550.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(0, 255, 0), D3DXVECTOR2(0.0f, 0.0f),
	D3DXVECTOR3(100.0f, 550.0f, 0.0f), 1.0f, D3DCOLOR_XRGB(0, 255, 0), D3DXVECTOR2(0.0f, 0.0f)
};

// 탑뷰 카메라 위치와 위쪽 방향
const static D3DXVECTOR3 g_topViewEye(0.0f, 200.0f, 0.0f);
const static D3DXVECTOR3 g_topViewUp(0.0f, 0.0f, 1.0f);

static char g_tigerModelPath[] = "tiger.x";

static BOOL g_isTopViewEnabled = FALSE;
static BOOL g_isNoClipEnabled = FALSE;
static BOOL g_isPaused = FALSE;
static BOOL g_didPlayerMove = FALSE;
static BOOL g_isPlaying = TRUE;
static BOOL g_isMouseButtonDown = FALSE;
static BOOL g_isDaytime = FALSE;
static SHORT g_cursorDisplayCount = 1;

static D3DXVECTOR3 g_topViewTarget(0.0f, 0.0f, 0.0f);

static CustomVertex g_tileVertices[4 * kMazeRowCount * kMazeColumnCount];
static CustomVertex g_outerWallVertices[4][4 * kMazeRowCount];
static CustomVertex g_upperWallVertices[4][4 * kMazeRowCount];

static CustomVertex g_mazeWallVertices[72][20];

static WORD g_tileIndices[2 * kMazeRowCount * kMazeColumnCount][3];

// 자유시점 해제 시 복원할 플레이어 변환과 시선 방향
static D3DXMATRIX g_savedPlayerWorldMatrix;
static D3DXVECTOR3 g_savedPlayerLookAt;

// 클라이언트 중앙을 화면 좌표로 변환한 커서 고정 위치
static POINT g_cursorCenter{};
static POINT g_mousePosition{};
static POINT g_currentMousePosition{};
static Frustum g_frustum;
static RECT g_exitButtonRect{};

static LPDIRECT3D9 g_pD3D = NULL;
static LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;

static LPDIRECT3DVERTEXBUFFER9 g_pTileVB = NULL;
static LPDIRECT3DINDEXBUFFER9 g_pTileIB = NULL;
static LPDIRECT3DVERTEXBUFFER9 g_pWallVB = NULL;
static LPDIRECT3DVERTEXBUFFER9 g_pWallVB2 = NULL;
static LPDIRECT3DVERTEXBUFFER9 g_pMazeVB = NULL;
static LPDIRECT3DTEXTURE9 g_pTileTexture = NULL;
static LPDIRECT3DTEXTURE9 g_pWallTexture = NULL;
static LPDIRECT3DTEXTURE9 g_pGrassTexture = NULL;
static LPDIRECT3DTEXTURE9 g_pNoticeTexture = NULL;
static LPDIRECT3DTEXTURE9 g_pExitTexture = NULL;
static LPDIRECT3DCUBETEXTURE9 g_pSkyboxTexture = NULL;
static LPD3DXFONT g_pClearFont = NULL;
static LPD3DXFONT g_pSettingFont = NULL;
static LPD3DXFONT g_pExitFont = NULL;
static LPD3DXFONT g_pFrameFont = NULL;
static LPD3DXFONT g_pTestFont = NULL;
static LPD3DXMESH g_pPlayerSphere = NULL;
static LPD3DXMESH g_pBulletSphere = NULL;
static LPD3DXMESH g_pSkyboxCube = NULL;

static Player g_player;
static vector<Notice> g_notices;
static Exit g_mazeExit;
static SettingsOverlay g_settingsOverlay;
static FpsCounter g_fpsCounter;
static Tiger g_tiger(D3DXVECTOR3(55.0f, 5.0f, 65.0f));
static SkyBox g_skyBox;

static HRESULT InitializeD3d(HWND windowHandle)
{
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	D3DPRESENT_PARAMETERS presentationParameters;
	ZeroMemory(&presentationParameters, sizeof(presentationParameters));
	presentationParameters.Windowed = TRUE;
	presentationParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentationParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentationParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // FPS 설정을 위한 설정?
	presentationParameters.EnableAutoDepthStencil = TRUE;
	presentationParameters.AutoDepthStencilFormat = D3DFMT_D16;

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, windowHandle, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentationParameters, &g_pd3dDevice)))
		return E_FAIL;

	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0);

	g_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

	return S_OK;
}

static VOID InitializeGameComponents()
{
	InitializeInput();
	g_fpsCounter.Initialize();
	// skybox texture load
	g_skyBox.LoadTextures(g_pd3dDevice);
	g_skyBox.CreateVertexBuffer(g_pd3dDevice);
	// tiger initialization
	g_tiger.Load(g_pd3dDevice, g_tigerModelPath);
	g_tiger.SetPosition(D3DXVECTOR3(55.0f, 5.0f, 65.0f));
	g_tiger.SetLookAt(g_player.GetPosition());

	SetRect(&g_exitButtonRect, 300, 450, 400, 500);
}

static VOID ConfigureDefaultMaterial()
{
	D3DMATERIAL9 material;
	ZeroMemory(&material, sizeof(D3DMATERIAL9));

	material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	g_pd3dDevice->SetMaterial(&material);
}

static VOID CreateFonts()
{
	// font
	D3DXCreateFont(g_pd3dDevice, 50, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pClearFont);
	D3DXCreateFont(g_pd3dDevice, 40, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pSettingFont);
	D3DXCreateFont(g_pd3dDevice, 30, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pExitFont);
	D3DXCreateFont(g_pd3dDevice, 20, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pTestFont);
	D3DXCreateFont(g_pd3dDevice, 25, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pFrameFont);
}

static VOID LoadSceneTextures()
{
	D3DXCreateTextureFromFile(g_pd3dDevice, kTileTexturePath, &g_pTileTexture);
	D3DXCreateTextureFromFile(g_pd3dDevice, kWallTexturePath, &g_pWallTexture);
	D3DXCreateTextureFromFile(g_pd3dDevice, kGrassTexturePath, &g_pGrassTexture);
	D3DXCreateTextureFromFileEx(g_pd3dDevice, kNoticeTexturePath,
		D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR,
		D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR,
		kTextureColorKey,
		NULL, NULL,
		&g_pNoticeTexture);
	D3DXCreateTextureFromFile(g_pd3dDevice, kExitTexturePath, &g_pExitTexture);
	D3DXCreateCubeTextureFromFile(g_pd3dDevice, kSkyBoxTexturePath, &g_pSkyboxTexture);
}

static VOID CreatePrimitiveMeshes()
{
	// 총알과 탑뷰 플레이어 표시에 사용할 구체 메시 생성
	D3DXCreateSphere(g_pd3dDevice, kBulletRadius, 10, 10, &g_pBulletSphere, NULL);
	D3DXCreateSphere(g_pd3dDevice, kPlayerRadius, 10, 10, &g_pPlayerSphere, NULL);
	// 스카이박스 렌더링에 사용할 큐브 메시 생성
	D3DXCreateBox(g_pd3dDevice, kSkyBoxSize, kSkyBoxSize, kSkyBoxSize, &g_pSkyboxCube, NULL);
}

static VOID CreateMazeGeometry()
{
	int i;

	// 미궁 내 벽을 구성할 vertex들의 buffer 생성
	GenerateMazeWalls(1, g_mazeWallVertices, &g_notices, &g_mazeExit);
	g_pd3dDevice->CreateVertexBuffer(sizeof(CustomVertex) * 72 * 20, 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pMazeVB, NULL);
	VOID** mazeVertexData;
	g_pMazeVB->Lock(0, sizeof(CustomVertex) * 72 * 20, (void**)&mazeVertexData, 0);
	memcpy(mazeVertexData, g_mazeWallVertices, sizeof(CustomVertex) * 72 * 20);
	g_pMazeVB->Unlock();

	// Notice를 구성하는 vertex buffer 생성
	for (i = 0; i < g_notices[0].GetNoticeCount(); i++)
	{
		g_notices[i].CreateVertexBuffer(g_pd3dDevice);
	}
	// Exit vertex buffer 생성
	g_mazeExit.CreateVertexBuffer(g_pd3dDevice);
}

static VOID CreateTileGeometry()
{
	int i, j;
	// 타일 정점 데이터 생성
	for (i = 0; i < kMazeRowCount * kMazeColumnCount; i++)
	{
		FLOAT tileX = (FLOAT)((i % kMazeColumnCount - kMazeColumnCount / 2.0f) * kTileSize);
		FLOAT tileZ = (FLOAT)((kMazeRowCount / 2.0f - i / kMazeColumnCount) * kTileSize);
		// D3DFVF_NORMAL: 조명 계산에 사용할 타일의 위쪽 법선
		for (j = 0; j < 4; j++)
		{
			g_tileVertices[i * 4 + j].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		}
		// D3DFVF_XYZ: 타일을 구성하는 네 꼭짓점 위치
		g_tileVertices[i * 4].position = D3DXVECTOR3(tileX, 0.0f, tileZ);
		g_tileVertices[i * 4 + 1].position = D3DXVECTOR3(tileX + kTileSize, 0.0f, tileZ);
		g_tileVertices[i * 4 + 2].position = D3DXVECTOR3(tileX + kTileSize, 0.0f, tileZ - kTileSize);
		g_tileVertices[i * 4 + 3].position = D3DXVECTOR3(tileX, 0.0f, tileZ - kTileSize);
		// D3DFVF_TEX1: 타일 한 장에 대응하는 텍스처 좌표
		g_tileVertices[i * 4].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
		g_tileVertices[i * 4 + 1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
		g_tileVertices[i * 4 + 2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
		g_tileVertices[i * 4 + 3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);
	}
	// 타일 인덱스 데이터 생성
	j = 0;
	for (i = 0; i < kMazeRowCount * kMazeColumnCount; i++)
	{
		g_tileIndices[j][0] = i * 4;
		g_tileIndices[j][1] = i * 4 + 1;
		g_tileIndices[j][2] = i * 4 + 2;
		g_tileIndices[++j][0] = i * 4;
		g_tileIndices[j][1] = i * 4 + 2;
		g_tileIndices[j++][2] = i * 4 + 3;
	}
	// 타일 정점 및 인덱스 버퍼 생성
	g_pd3dDevice->CreateVertexBuffer(sizeof(g_tileVertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pTileVB, NULL);
	VOID* tileVertexData;
	g_pTileVB->Lock(0, sizeof(g_tileVertices), (void**)&tileVertexData, 0);
	memcpy(tileVertexData, g_tileVertices, sizeof(g_tileVertices));
	g_pTileVB->Unlock();

	g_pd3dDevice->CreateIndexBuffer(sizeof(g_tileIndices), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pTileIB, NULL);
	VOID* tileIndexData;
	g_pTileIB->Lock(0, sizeof(g_tileIndices), (void**)&tileIndexData, 0);
	memcpy(tileIndexData, g_tileIndices, sizeof(g_tileIndices));
	g_pTileIB->Unlock();
}

static VOID CreateOuterWallGeometry()
{
	int i, j;
	// 위쪽 면
	for (i = 0; i < kMazeRowCount; i++)
	{
		for (j = 0; j < 4; j++)
			g_outerWallVertices[0][i * 4 + j].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

		g_outerWallVertices[0][i * 4].position = D3DXVECTOR3((i - kMazeRowCount / 2.0f) * kTileSize, 10.0f, (kMazeRowCount / 2.0f) * kTileSize);
		g_outerWallVertices[0][i * 4 + 1].position = D3DXVECTOR3((i + 1 - kMazeRowCount / 2.0f) * kTileSize, 10.0f, (kMazeRowCount / 2.0f) * kTileSize);
		g_outerWallVertices[0][i * 4 + 2].position = D3DXVECTOR3((i + 1 - kMazeRowCount / 2.0f) * kTileSize, 0.0f, (kMazeRowCount / 2.0f) * kTileSize);
		g_outerWallVertices[0][i * 4 + 3].position = D3DXVECTOR3((i - kMazeRowCount / 2.0f) * kTileSize, 0.0f, (kMazeRowCount / 2.0f) * kTileSize);

		g_outerWallVertices[0][i * 4].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
		g_outerWallVertices[0][i * 4 + 1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
		g_outerWallVertices[0][i * 4 + 2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
		g_outerWallVertices[0][i * 4 + 3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);
	}
	// 아래쪽 면
	for (i = 0; i < kMazeRowCount; i++)
	{
		for (j = 0; j < 4; j++)
			g_outerWallVertices[1][i * 4 + j].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

		g_outerWallVertices[1][i * 4].position = D3DXVECTOR3((kMazeRowCount / 2.0f - i) * kTileSize, 10.0f, (-kMazeRowCount / 2.0f) * kTileSize);
		g_outerWallVertices[1][i * 4 + 1].position = D3DXVECTOR3((kMazeRowCount / 2.0f - i - 1) * kTileSize, 10.0f, (-kMazeRowCount / 2.0f) * kTileSize);
		g_outerWallVertices[1][i * 4 + 2].position = D3DXVECTOR3((kMazeRowCount / 2.0f - i - 1) * kTileSize, 0.0f, (-kMazeRowCount / 2.0f) * kTileSize);
		g_outerWallVertices[1][i * 4 + 3].position = D3DXVECTOR3((kMazeRowCount / 2.0f - i) * kTileSize, 0.0f, (-kMazeRowCount / 2.0f) * kTileSize);

		g_outerWallVertices[1][i * 4].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
		g_outerWallVertices[1][i * 4 + 1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
		g_outerWallVertices[1][i * 4 + 2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
		g_outerWallVertices[1][i * 4 + 3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);
	}
	// 왼쪽 면
	for (i = 0; i < kMazeRowCount; i++)
	{
		for (j = 0; j < 4; j++)
			g_outerWallVertices[2][i * 4 + j].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);

		g_outerWallVertices[2][i * 4].position = D3DXVECTOR3((-kMazeRowCount / 2.0f + 1) * kTileSize, 10.0f, (i - kMazeRowCount / 2.0f) * kTileSize);
		g_outerWallVertices[2][i * 4 + 1].position = D3DXVECTOR3((-kMazeRowCount / 2.0f + 1) * kTileSize, 10.0f, (i + 1 - kMazeRowCount / 2.0f) * kTileSize);
		g_outerWallVertices[2][i * 4 + 2].position = D3DXVECTOR3((-kMazeRowCount / 2.0f + 1) * kTileSize, 0.0f, (i + 1 - kMazeRowCount / 2.0f) * kTileSize);
		g_outerWallVertices[2][i * 4 + 3].position = D3DXVECTOR3((-kMazeRowCount / 2.0f + 1) * kTileSize, 0.0f, (i - kMazeRowCount / 2.0f) * kTileSize);

		g_outerWallVertices[2][i * 4].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
		g_outerWallVertices[2][i * 4 + 1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
		g_outerWallVertices[2][i * 4 + 2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
		g_outerWallVertices[2][i * 4 + 3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);
	}
	// 오른쪽 면
	for (i = 0; i < kMazeRowCount; i++)
	{
		for (j = 0; j < 4; j++)
			g_outerWallVertices[3][i * 4 + j].normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);

		g_outerWallVertices[3][i * 4].position = D3DXVECTOR3((kMazeRowCount / 2.0f - 1) * kTileSize, 10.0f, (kMazeRowCount / 2.0f - i) * kTileSize);
		g_outerWallVertices[3][i * 4 + 1].position = D3DXVECTOR3((kMazeRowCount / 2.0f - 1) * kTileSize, 10.0f, (kMazeRowCount / 2.0f - i - 1) * kTileSize);
		g_outerWallVertices[3][i * 4 + 2].position = D3DXVECTOR3((kMazeRowCount / 2.0f - 1) * kTileSize, 0.0f, (kMazeRowCount / 2.0f - i - 1) * kTileSize);
		g_outerWallVertices[3][i * 4 + 3].position = D3DXVECTOR3((kMazeRowCount / 2.0f - 1) * kTileSize, 0.0f, (kMazeRowCount / 2.0f - i) * kTileSize);

		g_outerWallVertices[3][i * 4].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
		g_outerWallVertices[3][i * 4 + 1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
		g_outerWallVertices[3][i * 4 + 2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
		g_outerWallVertices[3][i * 4 + 3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);
	}
	// 외벽 정점 버퍼 생성
	g_pd3dDevice->CreateVertexBuffer(sizeof(g_outerWallVertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pWallVB, NULL);
	VOID* wallVertexData;
	g_pWallVB->Lock(0, sizeof(g_outerWallVertices), (void**)&wallVertexData, 0);
	memcpy(wallVertexData, g_outerWallVertices, sizeof(g_outerWallVertices));
	g_pWallVB->Unlock();
}

static VOID CreateUpperWallGeometry()
{
	int i, j;
	// 위쪽 면
	for (i = 0; i < kMazeRowCount; i++)
	{
		for (j = 0; j < 4; j++)
			g_outerWallVertices[0][i * 4 + j].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

		g_upperWallVertices[0][i * 4].position = D3DXVECTOR3((i - kMazeRowCount / 2.0f) * kTileSize, 10.0f, (kMazeRowCount / 2.0f + 1) * kTileSize);
		g_upperWallVertices[0][i * 4 + 1].position = D3DXVECTOR3((i + 1 - kMazeRowCount / 2.0f) * kTileSize, 10.0f, (kMazeRowCount / 2.0f + 1) * kTileSize);
		g_upperWallVertices[0][i * 4 + 2].position = D3DXVECTOR3((i + 1 - kMazeRowCount / 2.0f) * kTileSize, 10.0f, (kMazeRowCount / 2.0f) * kTileSize);
		g_upperWallVertices[0][i * 4 + 3].position = D3DXVECTOR3((i - kMazeRowCount / 2.0f) * kTileSize, 10.0f, (kMazeRowCount / 2.0f) * kTileSize);

		g_upperWallVertices[0][i * 4].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
		g_upperWallVertices[0][i * 4 + 1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
		g_upperWallVertices[0][i * 4 + 2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
		g_upperWallVertices[0][i * 4 + 3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);
	}
	// 아래쪽 면
	for (i = 0; i < kMazeRowCount; i++)
	{
		for (j = 0; j < 4; j++)
			g_outerWallVertices[1][i * 4 + j].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

		g_upperWallVertices[1][i * 4].position = D3DXVECTOR3((kMazeRowCount / 2.0f - i) * kTileSize, 10.0f, (-kMazeRowCount / 2.0f - 1) * kTileSize);
		g_upperWallVertices[1][i * 4 + 1].position = D3DXVECTOR3((kMazeRowCount / 2.0f - i - 1) * kTileSize, 10.0f, (-kMazeRowCount / 2.0f - 1) * kTileSize);
		g_upperWallVertices[1][i * 4 + 2].position = D3DXVECTOR3((kMazeRowCount / 2.0f - i - 1) * kTileSize, 10.0f, (-kMazeRowCount / 2.0f) * kTileSize);
		g_upperWallVertices[1][i * 4 + 3].position = D3DXVECTOR3((kMazeRowCount / 2.0f - i) * kTileSize, 10.0f, (-kMazeRowCount / 2.0f) * kTileSize);

		g_upperWallVertices[1][i * 4].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
		g_upperWallVertices[1][i * 4 + 1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
		g_upperWallVertices[1][i * 4 + 2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
		g_upperWallVertices[1][i * 4 + 3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);
	}
	// 왼쪽 면
	for (i = 0; i < kMazeRowCount; i++)
	{
		for (j = 0; j < 4; j++)
			g_outerWallVertices[2][i * 4 + j].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

		g_upperWallVertices[2][i * 4].position = D3DXVECTOR3((-kMazeRowCount / 2.0f) * kTileSize, 10.0f, (i - kMazeRowCount / 2.0f) * kTileSize);
		g_upperWallVertices[2][i * 4 + 1].position = D3DXVECTOR3((-kMazeRowCount / 2.0f) * kTileSize, 10.0f, (i + 1 - kMazeRowCount / 2.0f) * kTileSize);
		g_upperWallVertices[2][i * 4 + 2].position = D3DXVECTOR3((-kMazeRowCount / 2.0f + 1) * kTileSize, 10.0f, (i + 1 - kMazeRowCount / 2.0f) * kTileSize);
		g_upperWallVertices[2][i * 4 + 3].position = D3DXVECTOR3((-kMazeRowCount / 2.0f + 1) * kTileSize, 10.0f, (i - kMazeRowCount / 2.0f) * kTileSize);

		g_upperWallVertices[2][i * 4].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
		g_upperWallVertices[2][i * 4 + 1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
		g_upperWallVertices[2][i * 4 + 2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
		g_upperWallVertices[2][i * 4 + 3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);
	}
	// 오른쪽 면
	for (i = 0; i < kMazeRowCount; i++)
	{
		for (j = 0; j < 4; j++)
			g_outerWallVertices[3][i * 4 + j].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

		g_upperWallVertices[3][i * 4].position = D3DXVECTOR3((kMazeRowCount / 2.0f) * kTileSize, 10.0f, (kMazeRowCount / 2.0f - i) * kTileSize);
		g_upperWallVertices[3][i * 4 + 1].position = D3DXVECTOR3((kMazeRowCount / 2.0f) * kTileSize, 10.0f, (kMazeRowCount / 2.0f - i - 1) * kTileSize);
		g_upperWallVertices[3][i * 4 + 2].position = D3DXVECTOR3((kMazeRowCount / 2.0f - 1) * kTileSize, 10.0f, (kMazeRowCount / 2.0f - i - 1) * kTileSize);
		g_upperWallVertices[3][i * 4 + 3].position = D3DXVECTOR3((kMazeRowCount / 2.0f - 1) * kTileSize, 10.0f, (kMazeRowCount / 2.0f - i) * kTileSize);

		g_upperWallVertices[3][i * 4].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
		g_upperWallVertices[3][i * 4 + 1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
		g_upperWallVertices[3][i * 4 + 2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
		g_upperWallVertices[3][i * 4 + 3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);
	}
	// 상단 벽 정점 버퍼 생성
	g_pd3dDevice->CreateVertexBuffer(sizeof(g_upperWallVertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pWallVB2, NULL);
	VOID* upperWallVertexData;
	g_pWallVB2->Lock(0, sizeof(g_upperWallVertices), (void**)&upperWallVertexData, 0);
	memcpy(upperWallVertexData, g_upperWallVertices, sizeof(g_upperWallVertices));
	g_pWallVB2->Unlock();
}

static VOID InitializeResources()
{
	InitializeGameComponents();
	ConfigureDefaultMaterial();
	CreatePrimitiveMeshes();
	CreateFonts();
	LoadSceneTextures();

	CreateMazeGeometry();
	CreateTileGeometry();
	CreateOuterWallGeometry();
	CreateUpperWallGeometry();
}

static VOID ReleasePrimitiveMeshes()
{
	SafeRelease(g_pSkyboxCube);
	SafeRelease(g_pBulletSphere);
	SafeRelease(g_pPlayerSphere);
}

static VOID ReleaseFonts()
{
	SafeRelease(g_pTestFont);
	SafeRelease(g_pFrameFont);
	SafeRelease(g_pExitFont);
	SafeRelease(g_pSettingFont);
	SafeRelease(g_pClearFont);
}

static VOID ReleaseSceneTextures()
{
	SafeRelease(g_pSkyboxTexture);
	SafeRelease(g_pExitTexture);
	SafeRelease(g_pNoticeTexture);
	SafeRelease(g_pGrassTexture);
	SafeRelease(g_pWallTexture);
	SafeRelease(g_pTileTexture);
}

static VOID ReleaseGeometryBuffers()
{
	g_mazeExit.ReleaseVertexBuffer();
	for (int i = 0; i < g_notices[0].GetNoticeCount(); i++)
	{
		g_notices[i].ReleaseVertexBuffer();
	}
	SafeRelease(g_pMazeVB);
	SafeRelease(g_pWallVB2);
	SafeRelease(g_pWallVB);
	SafeRelease(g_pTileIB);
	SafeRelease(g_pTileVB);
}

static VOID ReleaseResources()
{
	ReleaseGeometryBuffers();
	ReleaseSceneTextures();
	ReleaseFonts();
	ReleasePrimitiveMeshes();

	SafeRelease(g_pd3dDevice);
	SafeRelease(g_pD3D);
}

static VOID HandleMovementInput(FLOAT deltaTimeSeconds)
{
	if (IsKeyDown('A') || IsKeyDown(VK_LEFT))
	{
		g_didPlayerMove = g_player.Move(MoveDirection::Left, kMazeMap, g_isNoClipEnabled, deltaTimeSeconds);
	}

	if (IsKeyDown('D') || IsKeyDown(VK_RIGHT))
	{
		g_didPlayerMove = g_player.Move(MoveDirection::Right, kMazeMap, g_isNoClipEnabled, deltaTimeSeconds);
	}

	if (IsKeyDown('W') || IsKeyDown(VK_UP))
	{
		g_didPlayerMove = g_player.Move(MoveDirection::Forward, kMazeMap, g_isNoClipEnabled, deltaTimeSeconds);
	}

	if (IsKeyDown('S') || IsKeyDown(VK_DOWN))
	{
		g_didPlayerMove = g_player.Move(MoveDirection::Backward, kMazeMap, g_isNoClipEnabled, deltaTimeSeconds);
	}
}

static VOID HandleRotationInput(FLOAT deltaTimeSeconds)
{
	if (IsKeyDown('Q'))
	{
		g_player.Rotate(TRUE, deltaTimeSeconds);
	}

	if (IsKeyDown('E'))
	{
		g_player.Rotate(FALSE, deltaTimeSeconds);
	}
}

static VOID HandlePauseInput()
{
	if (IsKeyPressed(VK_ESCAPE) == TRUE)
	{
		g_isPaused = !g_isPaused;
	}
}

static VOID HandleFeatureToggleInput()
{
	// light option on/off
	if (IsKeyPressed('1') == TRUE)
	{
		if (g_isDaytime == TRUE)
		{
			g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
			g_isDaytime = FALSE;
		}
		else
		{
			g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
			g_isDaytime = TRUE;
		}
	}

	// camera TopView on/off
	if (IsKeyPressed('2') == TRUE)
	{
		if (g_isTopViewEnabled == FALSE)
			g_isTopViewEnabled = TRUE;
		else
			g_isTopViewEnabled = FALSE;
	}

	// player flashlight on/off
	if (IsKeyPressed('3') == TRUE)
	{
		if (g_player.IsFlashlightOn() == TRUE)
		{
			g_player.SetFlashlight(FALSE);
		}
		else
		{
			g_player.SetFlashlight(TRUE);
		}
	}

	// NoClip(FreeFly) on/off
	if (IsKeyPressed('4') == TRUE)
	{
		if (g_isNoClipEnabled == TRUE)
		{
			g_isNoClipEnabled = FALSE;
			// 자유시점 종료 시, 저장해뒀던 player 정보 복구
			g_player.SetWorldMatrix(g_savedPlayerWorldMatrix);
			g_player.SetLookAt(g_savedPlayerLookAt);
		}
		else
		{
			g_isNoClipEnabled = TRUE;
			g_savedPlayerWorldMatrix = g_player.GetWorldMatrix();
			g_savedPlayerLookAt = g_player.GetLookAt();
		}
	}
}

static VOID HandleJumpInput()
{
	if (IsKeyPressed(VK_SPACE))
	{
		g_player.Jump();
	}
}

static VOID UpdateDynamicObjects(FLOAT deltaTimeSeconds)
{
	if (!g_isPlaying)
		return;

	// 총알 움직임 계산
	g_player.UpdateBullets(deltaTimeSeconds);
	// 호랑이 움직임 계산
	g_tiger.Move(kMazeMap, deltaTimeSeconds);
}

static VOID UpdateInteractionState()
{
	if (g_didPlayerMove)
	{
		for (int i = 0; i < g_notices[0].GetNoticeCount(); i++)
		{
			g_notices[i].UpdateFacing(g_player.GetPosition());
		}

		g_mazeExit.UpdateFacing(g_player.GetPosition());
	}

	for (int i = 0; i < g_notices[0].GetNoticeCount(); i++)
	{
		if (g_notices[i].CanInteract(g_player.GetPosition(), g_isNoClipEnabled) == TRUE)
		{
			g_isTopViewEnabled = TRUE;
			break;
		}
	}

	g_isPlaying = g_mazeExit.CanInteract(g_player.GetPosition(), g_isNoClipEnabled) ? FALSE : TRUE;
}

static VOID UpdateGame(FLOAT deltaTimeSeconds)
{
	// ESC
	HandlePauseInput();

	if (g_isPaused)
	{
		return;
	}

	UpdateDynamicObjects(deltaTimeSeconds);

	// wasd 또는 방향키 : 플레이어 앞뒤좌우 움직임
	HandleMovementInput(deltaTimeSeconds);

	// Notice & Exit rotation
	UpdateInteractionState();

	// Q/E : 플레이어 CCW/CW 회전
	HandleRotationInput(deltaTimeSeconds);

	// 스페이스
	HandleJumpInput();

	// 추가 기능 : 1, 2, 3, 4번
	HandleFeatureToggleInput();
}

static VOID RenderUi()
{
	RECT textRect;
	char textBuffer[500];

	g_pd3dDevice->SetTexture(0, NULL);
	g_pd3dDevice->SetFVF(D3DFVF_UI_VERTEX);
	// 탈출구 UI
	if (!g_isPlaying)
	{
		g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, g_popupVertices, sizeof(UiVertex));
		wsprintf(textBuffer, "C L E A R");
		SetRect(&textRect, 250, 200, 0, 0);
		g_pClearFont->DrawTextA(NULL, textBuffer, -1, &textRect, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

		g_mazeExit.RenderButton(g_pd3dDevice);
		wsprintf(textBuffer, "e x i t");
		SetRect(&textRect, 320, 460, 0, 0);
		g_pExitFont->DrawTextA(NULL, textBuffer, -1, &textRect, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	}

	// 좌상단 UI
	if (g_isTopViewEnabled == FALSE)
	{
		g_pd3dDevice->SetTexture(0, NULL);
		g_pd3dDevice->SetFVF(D3DFVF_UI_VERTEX);
		g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, g_uiVertices, sizeof(UiVertex));

		// 조작 안내 UI
		SetRect(&textRect, 20, 20, 0, 0);
		wsprintf(textBuffer, " 1: 낮밤 전환\n 2: 탑뷰 on/off\n 3: 손전등 on/off\n 4: 자유시점 on/off\n esc: 일시 정지");
		g_pTestFont->DrawTextA(NULL, textBuffer, -1, &textRect, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
	}

	// 환경설정 및 일시정지 UI
	if (g_isPaused)
	{
		g_settingsOverlay.Render(g_pd3dDevice);
		wsprintf(textBuffer, "P A U S E");
		SetRect(&textRect, 280, 200, 0, 0);
		g_pSettingFont->DrawTextA(NULL, textBuffer, -1, &textRect, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

		g_mazeExit.RenderButton(g_pd3dDevice);
		wsprintf(textBuffer, "e x i t");
		SetRect(&textRect, 320, 460, 0, 0);
		g_pExitFont->DrawTextA(NULL, textBuffer, -1, &textRect, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	}

	// 자유시점 표시
	if (g_isNoClipEnabled)
	{
		wsprintf(textBuffer, "자유시점 ON");
		// 텍스트 width 얻는 법: drawtext 시 DT_CALCRECT 설정하면 rect에 맞게 텍스트 크기만 계산
		// 그걸 활용해서 조절된 rect에서 값을 가져와 width 구하기
		SetRect(&textRect, 0, 0, 0, 0);
		g_pFrameFont->DrawTextA(NULL, textBuffer, -1, &textRect, DT_CALCRECT, D3DCOLOR_ARGB(0, 0, 0, 0));
		int width = textRect.right - textRect.left;
		SetRect(&textRect, kWindowWidth / 2 - width / 2, 0, 0, 0);
		g_pFrameFont->DrawTextA(NULL, textBuffer, -1, &textRect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 0));
	}

	// FPS 표시
	SetRect(&textRect, kWindowWidth - 110, 0, 0, 0);
	wsprintf(textBuffer, "FPS: %3d", g_fpsCounter.GetFps());
	g_pFrameFont->DrawTextA(NULL, textBuffer, -1, &textRect, DT_NOCLIP, D3DCOLOR_XRGB(0, 255, 0));
}

static VOID ConfigureLighting()
{
	D3DLIGHT9* playerLight = g_player.GetLight();
	// TODO: g_isTopViewEnabled == TRUE 이면 player의 spot light,
	// FALSE 이면 하늘 시점에서 point light로 바꿔서 맵 전체가 어느 정도 보이게 하는 것도 좋을듯
	g_pd3dDevice->SetLight(0, playerLight);
	if (g_player.IsFlashlightOn() == TRUE)
	{
		g_pd3dDevice->LightEnable(0, TRUE);
	}
	else
	{
		g_pd3dDevice->LightEnable(0, FALSE);
	}

	D3DLIGHT9 skyLight;
	// 하늘에서 플레이어를 향해 비추는 빛
	ZeroMemory(&skyLight, sizeof(D3DLIGHT9));
	skyLight.Type = D3DLIGHT_SPOT;
	skyLight.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	skyLight.Direction = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	skyLight.Position = g_player.GetPosition() + D3DXVECTOR3(0.0f, 10.0f, 0.0f); // 플레이어 머리 위에서 비추는 빛
	skyLight.Range = 300.0f;
	skyLight.Attenuation0 = 1.0f;
	skyLight.Falloff = 1.0f;
	skyLight.Phi = D3DXToRadian(90.0f);
	skyLight.Theta = D3DXToRadian(30.0f);
	g_pd3dDevice->SetLight(1, &skyLight);
	g_pd3dDevice->LightEnable(1, TRUE);
}

static VOID ConfigureCamera()
{
	D3DXMATRIX viewMatrix;
	D3DXVECTOR3 playerPosition = g_player.GetPosition();
	D3DXVECTOR3 playerLookAt = g_player.GetLookAt();

	// 1인칭 시점
	if (g_isTopViewEnabled == FALSE)
	{
		D3DXMatrixLookAtLH(&viewMatrix, &playerPosition, &playerLookAt, &kWorldUp);
		g_pd3dDevice->SetTransform(D3DTS_VIEW, &viewMatrix);
	}
	// 탑뷰 시점
	else
	{
		D3DXMatrixLookAtLH(&viewMatrix, &g_topViewEye, &g_topViewTarget, &g_topViewUp);
		g_pd3dDevice->SetTransform(D3DTS_VIEW, &viewMatrix);
	}

	D3DXMATRIX projectionMatrix;
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, D3DX_PI / 4, 1.0f, 0.1f, 1000.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &projectionMatrix);

	// frustum plane을 계산할, view matrix와 projection matrix의 곱
	D3DXMATRIX viewProjectionMatrix;

	// 하늘에서 바라볼 때, 오브젝트의 LookAt matrix를 따로 계산해야함
	if (g_isTopViewEnabled == TRUE)
	{
		D3DXMATRIX playerViewMatrix;
		D3DXMatrixLookAtLH(&playerViewMatrix, &playerPosition, &playerLookAt, &kWorldUp);
		D3DXMatrixMultiply(&viewProjectionMatrix, &playerViewMatrix, &projectionMatrix);
	}
	else
	{
		D3DXMatrixMultiply(&viewProjectionMatrix, &viewMatrix, &projectionMatrix);
	}

	g_frustum.Update(&viewProjectionMatrix);
}

static VOID RenderWorld()
{
	int i, j;

	D3DXMATRIX worldMatrix;
	D3DXMatrixIdentity(&worldMatrix);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);

	D3DXVECTOR3 playerPosition = g_player.GetPosition();
	// frustum culling 시 타일 중심 좌표 표시용
	D3DXVECTOR3 tileCenter;

	g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	// 스카이박스
	g_skyBox.Render(g_pd3dDevice);

	g_pd3dDevice->SetTexture(0, g_pGrassTexture);
	g_pd3dDevice->SetStreamSource(0, g_pTileVB, 0, sizeof(CustomVertex));
	g_pd3dDevice->SetIndices(g_pTileIB);

	// 타일: 중심을 감싸는 구로 프러스텀 컬링
	for (i = 0; i < kMazeRowCount * kMazeColumnCount; i++)
	{
		tileCenter = CalculateMidPoint(g_tileVertices[i * 4].position, g_tileVertices[i * 4 + 2].position);
		if (g_frustum.IntersectsSphere(&tileCenter, kTileSize / 2 * kSqrt2) == TRUE)
		{
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * 4, 2);
		}
	}

	// 외벽과 미로 벽
	g_pd3dDevice->SetTexture(0, g_pWallTexture);
	g_pd3dDevice->SetStreamSource(0, g_pWallVB, 0, sizeof(CustomVertex));
	// 외벽 측면 컬링
	for (i = 0; i < kMazeRowCount * 4; i++)
	{
		tileCenter = CalculateMidPoint(g_outerWallVertices[i / kMazeRowCount][(i * 4) % (kMazeRowCount * 4)].position, g_outerWallVertices[i / kMazeRowCount][(i * 4) % (kMazeRowCount * 4) + 2].position);
		if (g_frustum.IntersectsSphere(&tileCenter, kTileSize / 2 * kSqrt2) == TRUE)
		{
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * 4, 2);
		}
	}
	// 외벽 상단 컬링
	g_pd3dDevice->SetStreamSource(0, g_pWallVB2, 0, sizeof(CustomVertex));
	for (i = 0; i < kMazeRowCount * 4; i++)
	{
		tileCenter = CalculateMidPoint(g_upperWallVertices[i / kMazeRowCount][(i * 4) % (kMazeRowCount * 4)].position, g_upperWallVertices[i / kMazeRowCount][(i * 4) % (kMazeRowCount * 4) + 2].position);
		if (g_frustum.IntersectsSphere(&tileCenter, kTileSize / 2 * kSqrt2) == TRUE)
		{
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * 4, 2);
		}
	}
	// 미로 내부 벽 컬링
	g_pd3dDevice->SetStreamSource(0, g_pMazeVB, 0, sizeof(CustomVertex));
	for (i = 0; i < 72; i++)
	{
		for (j = 0; j < 5; j++)
		{
			tileCenter = CalculateMidPoint(g_mazeWallVertices[i][j * 4].position, g_mazeWallVertices[i][j * 4 + 2].position);
			if (g_frustum.IntersectsSphere(&tileCenter, kTileSize / 2 * kSqrt2) == TRUE)
			{
				g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * 20 + j * 4, 2);
			}
		}
	}

	// 안내문
	g_pd3dDevice->SetTexture(0, g_pNoticeTexture);
	D3DXMATRIX noticeWorldMatrix;
	for (i = 0; i < g_notices[0].GetNoticeCount(); i++)
	{
		noticeWorldMatrix = g_notices[i].GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &noticeWorldMatrix);
		g_notices[i].Render(g_pd3dDevice);
	}

	// 출구
	g_pd3dDevice->SetTexture(0, g_pExitTexture);
	noticeWorldMatrix = g_mazeExit.GetWorldMatrix();
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &noticeWorldMatrix);
	g_mazeExit.Render(g_pd3dDevice);

	// 총알
	g_pd3dDevice->SetTexture(0, g_pTileTexture);
	g_player.RenderBullets(g_pd3dDevice, g_pBulletSphere);

	// 탑뷰에서 플레이어 위치를 구체로 표시
	if (g_isTopViewEnabled == TRUE)
	{
		D3DXMATRIX playerWorldMatrix;
		D3DXMatrixTranslation(&playerWorldMatrix, playerPosition.x, playerPosition.y, playerPosition.z);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &playerWorldMatrix);
		g_pd3dDevice->SetTexture(0, g_pTileTexture);
		g_pPlayerSphere->DrawSubset(0);
	}

	// 호랑이
	D3DXMATRIX tigerWorldMatrix = g_tiger.GetWorldMatrix();

	g_pd3dDevice->SetTransform(D3DTS_WORLD, &tigerWorldMatrix);
	g_tiger.Render(g_pd3dDevice);
}

static VOID Render()
{
	if (NULL == g_pd3dDevice)
		return;

	if (g_isDaytime == TRUE)
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
	else
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(23, 23, 23), 1.0f, 0);
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);

	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		ConfigureLighting();
		ConfigureCamera();
		RenderWorld();
		RenderUi();
		g_pd3dDevice->EndScene();
	}
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

static LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		g_cursorCenter.x = kWindowWidth / 2;
		g_cursorCenter.y = kWindowHeight / 2;

		ClientToScreen(hWnd, &g_cursorCenter);
		SetCursorPos(g_cursorCenter.x, g_cursorCenter.y);
		g_cursorDisplayCount = ShowCursor(FALSE);
		break;

	case WM_LBUTTONDOWN:
		g_mousePosition.x = LOWORD(lParam);
		g_mousePosition.y = HIWORD(lParam);
		g_isMouseButtonDown = TRUE;
		if (!g_isPlaying || g_isPaused)
		{
			if (PtInRect(&g_exitButtonRect, g_mousePosition))
			{
				g_mazeExit.PressButton();
			}
		}
		break;

	case WM_MOUSEMOVE:
		GetCursorPos(&g_currentMousePosition);
		if (!g_isTopViewEnabled && g_isPlaying && !g_isPaused)
		{
			if (g_currentMousePosition.x > g_cursorCenter.x)
			{
				g_player.Rotate(FALSE, FALSE, (g_currentMousePosition.x - g_cursorCenter.x) * kMouseHorizontalRotationSensitivity);
			}
			else if (g_currentMousePosition.x < g_cursorCenter.x)
			{
				g_player.Rotate(TRUE, FALSE, (g_cursorCenter.x - g_currentMousePosition.x) * kMouseHorizontalRotationSensitivity);
			}
			// y좌표는 아래로 갈수록 커지므로, 이게 아래 회전
			if (g_currentMousePosition.y > g_cursorCenter.y)
			{
				g_player.Rotate(TRUE, TRUE, (g_currentMousePosition.y - g_cursorCenter.y) * kMouseVerticalRotationSensitivity);
			}
			else if (g_currentMousePosition.y < g_cursorCenter.y)
			{
				g_player.Rotate(FALSE, TRUE, (g_cursorCenter.y - g_currentMousePosition.y) * kMouseVerticalRotationSensitivity);
			}
		}
		if (!g_isPlaying || g_isPaused)
		{
			if (PtInRect(&g_exitButtonRect, g_mousePosition) && g_isMouseButtonDown)
				g_mazeExit.PressButton();
		}
		else
			g_mazeExit.ReleaseButton();
		// 게임 중엔 화면 정중앙으로 다시 세팅
		if (g_isPlaying && !g_isPaused)
			SetCursorPos(g_cursorCenter.x, g_cursorCenter.y);
		break;

	case WM_LBUTTONUP:
		g_mousePosition.x = LOWORD(lParam);
		g_mousePosition.y = HIWORD(lParam);
		g_isMouseButtonDown = FALSE;
		g_mazeExit.ReleaseButton();

		if (g_isPlaying && !g_isPaused)
		{
			g_player.FireBullet(&g_mousePosition);
		}

		if (!g_isPlaying || g_isPaused)
		{
			if (PtInRect(&g_exitButtonRect, g_mousePosition))
			{
				PostMessage(hWnd, WM_CLOSE, 0, 0);
			}
		}

		break;

	case WM_DESTROY:
		ReleaseResources();
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

/*------------------------------------------------------------------------------
 * 이 프로그램의 시작점
 *------------------------------------------------------------------------------
 */
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// 윈도우 클래스 등록
	WNDCLASSEX windowClass = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
					  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					  kProgramName, NULL };
	RegisterClassEx(&windowClass);
	// 윈도우 생성
	HWND windowHandle = CreateWindow(kProgramName, kProgramName,
		WS_OVERLAPPEDWINDOW, 100, 100, kWindowWidth, kWindowHeight,
		GetDesktopWindow(), NULL, windowClass.hInstance, NULL);

	// Direct3D 초기화
	if (SUCCEEDED(InitializeD3d(windowHandle)))
	{
		{
			InitializeResources();
			// 윈도우 출력
			ShowWindow(windowHandle, SW_SHOWDEFAULT);
			UpdateWindow(windowHandle);
			// 메시지 루프
			MSG msg;
			ZeroMemory(&msg, sizeof(msg));

			DWORD previousFrameTime = timeGetTime();

			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					DWORD currentFrameTime = timeGetTime();
					FLOAT deltaTimeSeconds =
						static_cast<FLOAT>(currentFrameTime - previousFrameTime) / 1000.0f;
					if (deltaTimeSeconds > kMaxDeltaTimeSeconds)
						deltaTimeSeconds = kMaxDeltaTimeSeconds;
					previousFrameTime = currentFrameTime;

					if (!g_isPlaying || g_isPaused)
					{
						while (g_cursorDisplayCount < 0)
							g_cursorDisplayCount = ShowCursor(TRUE);
					}
					else
					{
						while (g_cursorDisplayCount >= 0)
							g_cursorDisplayCount = ShowCursor(FALSE);
					}

					// 입력 상태 갱신 -> 게임 갱신 -> 렌더링
					UpdateInput();
					UpdateGame(deltaTimeSeconds);
					Render();
					g_fpsCounter.Update(deltaTimeSeconds);
				}
			}
		}
	}

	// 등록된 클래스 소거
	UnregisterClass(kProgramName, windowClass.hInstance);
	return 0;
}
