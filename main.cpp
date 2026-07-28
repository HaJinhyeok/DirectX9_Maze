
#include "MazeGenerator.h"
#include "Input.h"
#include "Frustum.h"
#include "FpsCounter.h"
#include "SkyBox.h"
#include "Tiger.h"
#include "ComUtils.h"
#include "MazeLoader.h"
#include "LevelCatalog.h"

const D3DXVECTOR3 kWorldUp(0.0f, 1.0f, 0.0f);

constexpr LONG kPauseOverlayWidth = 500;
constexpr LONG kPauseOverlayHeight = 500;
constexpr LONG kClearPopupWidth = 500;
constexpr LONG kClearPopupHeight = 400;
constexpr LONG kExitButtonWidth = 100;
constexpr LONG kExitButtonHeight = 50;
constexpr LONG kExitButtonTopOffsetFromCenter = 100;
constexpr LONG kMinimumWindowTrackWidth = 600;
constexpr LONG kMinimumWindowTrackHeight = 600;
constexpr int kTileVertexCount = 4;
constexpr char kLevelCatalogPath[] = "Assets\\Data\\Levels\\LevelList.txt";

using WallFaceVertices = std::array<CustomVertex, kVerticesPerWallFace>;

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

static char g_tigerModelPath[] = "Assets\\Models\\Tiger\\tiger.x";

enum class TopViewMode
{
	Disabled,
	NoticeHint,
	CullingDebug
};

static TopViewMode g_topViewMode = TopViewMode::Disabled;

static bool IsTopViewActive() noexcept
{
	return g_topViewMode != TopViewMode::Disabled;
}

static BOOL g_isNoClipEnabled = FALSE;
static BOOL g_isPaused = FALSE;
static BOOL g_didPlayerMove = FALSE;
static BOOL g_isPlaying = TRUE;
static BOOL g_isMouseButtonDown = FALSE;
static BOOL g_isDaytime = FALSE;
static SHORT g_cursorDisplayCount = 1;

static D3DXVECTOR3 g_topViewTarget(0.0f, 0.0f, 0.0f);

static std::vector<CustomVertex> g_tileVertices;
static std::vector<WallFaceVertices> g_outerWallVertices;
static std::vector<WallFaceVertices> g_upperWallVertices;

static std::vector<MazeWallBlockVertices> g_mazeWallVertices;

// 자유시점 해제 시 복원할 플레이어 변환과 시선 방향
static D3DXMATRIX g_savedPlayerWorldMatrix;
static D3DXVECTOR3 g_savedPlayerLookAt;

// 클라이언트 중앙을 화면 좌표로 변환한 커서 고정 위치
static POINT g_cursorCenter{};
static POINT g_mousePosition{};
static POINT g_currentMousePosition{};
static Frustum g_frustum;
static RECT g_pauseOverlayRect{};
static RECT g_clearPopupRect{};
static RECT g_exitButtonRect{};

static LPDIRECT3D9 g_pD3D = NULL;
static LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS g_presentationParameters{};
static UINT g_clientWidth = 0;
static UINT g_clientHeight = 0;
static BOOL g_isDeviceResetPending = FALSE;
static BOOL g_areFontResourcesLost = FALSE;

static LPDIRECT3DVERTEXBUFFER9 g_pTileVB = NULL;
static LPDIRECT3DVERTEXBUFFER9 g_pWallVB = NULL;
static LPDIRECT3DVERTEXBUFFER9 g_pWallVB2 = NULL;
static LPDIRECT3DVERTEXBUFFER9 g_pMazeVB = NULL;
static LPDIRECT3DTEXTURE9 g_pBulletTexture = NULL;
static LPDIRECT3DTEXTURE9 g_pPlayerMarkerTexture = NULL;
static LPDIRECT3DTEXTURE9 g_pWallTexture = NULL;
static LPDIRECT3DTEXTURE9 g_pGrassTexture = NULL;
static LPDIRECT3DTEXTURE9 g_pNoticeTexture = NULL;
static LPDIRECT3DTEXTURE9 g_pExitTexture = NULL;
static LPD3DXFONT g_pClearFont = NULL;
static LPD3DXFONT g_pSettingFont = NULL;
static LPD3DXFONT g_pExitFont = NULL;
static LPD3DXFONT g_pFrameFont = NULL;
static LPD3DXFONT g_pTestFont = NULL;
static LPD3DXMESH g_pPlayerSphere = NULL;
static LPD3DXMESH g_pBulletSphere = NULL;

static Player g_player;
static vector<Notice> g_notices;
static Exit g_mazeExit;
static SettingsOverlay g_settingsOverlay;
static FpsCounter g_fpsCounter;
static Tiger g_tiger(D3DXVECTOR3(0.0f, kTileSize / 2.0f, 0.0f));
static SkyBox g_skyBox;
static MazeDefinition g_maze;
static std::vector<std::string> g_levelPaths;
static size_t g_currentLevelIndex = 0;
static std::string g_initializationErrorMessage;

static VOID ConfigureDeviceRenderStates(LPDIRECT3DDEVICE9 device)
{
	// 장치 리셋 후에도 현재 낮·밤 상태를 유지한다.
	const DWORD isLightingEnabled = g_isDaytime == TRUE ? FALSE : TRUE;

	device->SetRenderState(D3DRS_LIGHTING, isLightingEnabled);

	device->SetRenderState(D3DRS_ZENABLE, TRUE);
	device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
	device->SetRenderState(D3DRS_ALPHAREF, 0);
	device->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
}

static HRESULT InitializeD3d(HWND windowHandle)
{
	if (windowHandle == nullptr)
		return E_INVALIDARG;

	SafeRelease(g_pd3dDevice);
	SafeRelease(g_pD3D);

	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (g_pD3D == nullptr)
		return E_FAIL;

	D3DCAPS9 deviceCapabilities = {};

	const HRESULT capabilitiesResult = g_pD3D->GetDeviceCaps(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		&deviceCapabilities);

	if (FAILED(capabilitiesResult))
	{
		SafeRelease(g_pD3D);
		return capabilitiesResult;
	}

	const DWORD vertexProcessingFlag =
		(deviceCapabilities.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0
		? D3DCREATE_HARDWARE_VERTEXPROCESSING
		: D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	ZeroMemory(&g_presentationParameters, sizeof(g_presentationParameters));
	g_presentationParameters.Windowed = TRUE;
	g_presentationParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_presentationParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	g_presentationParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // 수직 동기화를 끄고 프레젠테이션을 즉시 수행한다.
	g_presentationParameters.EnableAutoDepthStencil = TRUE;
	g_presentationParameters.AutoDepthStencilFormat = D3DFMT_D16;

	const HRESULT deviceCreationResult = g_pD3D->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		windowHandle,
		vertexProcessingFlag,
		&g_presentationParameters,
		&g_pd3dDevice);

	if (FAILED(deviceCreationResult))
	{
		SafeRelease(g_pD3D);
		return deviceCreationResult;
	}

	ConfigureDeviceRenderStates(g_pd3dDevice);

	return S_OK;
}

static VOID UpdateUiLayout()
{
	if (g_clientWidth == 0 || g_clientHeight == 0)
		return;

	const LONG centerX = static_cast<LONG>(g_clientWidth) / 2;
	const LONG centerY = static_cast<LONG>(g_clientHeight) / 2;

	SetRect(
		&g_pauseOverlayRect,
		centerX - kPauseOverlayWidth / 2,
		centerY - kPauseOverlayHeight / 2,
		centerX + kPauseOverlayWidth / 2,
		centerY + kPauseOverlayHeight / 2);

	g_settingsOverlay.SetBounds(g_pauseOverlayRect);

	SetRect(
		&g_clearPopupRect,
		centerX - kClearPopupWidth / 2,
		centerY - kClearPopupHeight / 2,
		centerX + kClearPopupWidth / 2,
		centerY + kClearPopupHeight / 2);

	const FLOAT popupLeft = static_cast<FLOAT>(g_clearPopupRect.left);
	const FLOAT popupTop = static_cast<FLOAT>(g_clearPopupRect.top);
	const FLOAT popupRight = static_cast<FLOAT>(g_clearPopupRect.right);
	const FLOAT popupBottom = static_cast<FLOAT>(g_clearPopupRect.bottom);

	g_popupVertices[0].position.x = popupLeft;
	g_popupVertices[0].position.y = popupTop;
	g_popupVertices[1].position.x = popupRight;
	g_popupVertices[1].position.y = popupTop;
	g_popupVertices[2].position.x = popupRight;
	g_popupVertices[2].position.y = popupBottom;
	g_popupVertices[3].position.x = popupLeft;
	g_popupVertices[3].position.y = popupBottom;

	const LONG buttonTop = centerY + kExitButtonTopOffsetFromCenter;

	SetRect(
		&g_exitButtonRect,
		centerX - kExitButtonWidth / 2,
		buttonTop,
		centerX + kExitButtonWidth / 2,
		buttonTop + kExitButtonHeight);

	g_mazeExit.SetButtonBounds(g_exitButtonRect);
}

static bool HasNextLevel() noexcept
{
	return !g_levelPaths.empty() &&
		g_currentLevelIndex < g_levelPaths.size() - 1;
}

static HRESULT LoadLevelMaze(size_t levelIndex)
{
	g_initializationErrorMessage.clear();

	if (levelIndex >= g_levelPaths.size())
	{
		g_initializationErrorMessage =
			"Level index out of range: " +
			std::to_string(levelIndex);

		return E_INVALIDARG;
	}

	const MazeLoadResult loadResult =
		LoadMazeFromFile(g_levelPaths[levelIndex]);

	if (!loadResult.isSuccessful)
	{
		g_initializationErrorMessage = loadResult.errorMessage;

		return E_FAIL;
	}

	g_maze = loadResult.maze;
	g_currentLevelIndex = levelIndex;

	return S_OK;
}

static HRESULT InitializeMaze()
{
	g_initializationErrorMessage.clear();

	const LevelCatalogLoadResult catalogResult = LoadLevelCatalogFromFile(kLevelCatalogPath);

	if (!catalogResult.isSuccessful)
	{
		g_initializationErrorMessage = catalogResult.errorMessage;

		return E_FAIL;
	}

	g_levelPaths = catalogResult.levelPaths;

	return LoadLevelMaze(0);
}

static VOID ResetLevelEntities()
{
	const MazeCellPosition& playerStart = g_maze.playerStart;

	const D3DXVECTOR3 playerStartPosition =
		CalculateMazeCellCenter(
			g_maze,
			playerStart.row,
			playerStart.column);

	g_player.ResetForLevel(playerStartPosition);

	const MazeCellPosition& tigerStart = g_maze.tigerStart;

	const D3DXVECTOR3 tigerStartPosition =
		CalculateMazeCellCenter(
			g_maze,
			tigerStart.row,
			tigerStart.column);

	g_tiger.ResetForLevel(
		tigerStartPosition,
		g_player.GetPosition());
}

static VOID ResetLevelPlayState()
{
	g_isNoClipEnabled = FALSE;
	g_isPaused = FALSE;
	g_didPlayerMove = FALSE;
	g_isPlaying = TRUE;
	g_isMouseButtonDown = FALSE;
	g_topViewMode = TopViewMode::Disabled;

	g_mazeExit.ReleaseButton();
	InitializeInput();
}

static VOID UpdateBillboardFacing()
{
	const D3DXVECTOR3 playerPosition = g_player.GetPosition();

	for (Notice& notice : g_notices)
	{
		notice.UpdateFacing(playerPosition);
	}

	g_mazeExit.UpdateFacing(playerPosition);
}

static HRESULT InitializeGameComponents()
{
	g_fpsCounter.Initialize();

	const HRESULT skyBoxTextureResult = g_skyBox.LoadTextures(g_pd3dDevice);

	if (FAILED(skyBoxTextureResult))
		return skyBoxTextureResult;

	const HRESULT skyBoxBufferResult = g_skyBox.CreateVertexBuffer(g_pd3dDevice);

	if (FAILED(skyBoxBufferResult))
		return skyBoxBufferResult;

	const HRESULT tigerLoadResult = g_tiger.Load(g_pd3dDevice, g_tigerModelPath);

	if (FAILED(tigerLoadResult))
		return tigerLoadResult;

	ResetLevelEntities();
	ResetLevelPlayState();
	UpdateUiLayout();

	return S_OK;
}

static VOID ConfigureDefaultMaterial()
{
	D3DMATERIAL9 material;
	ZeroMemory(&material, sizeof(D3DMATERIAL9));

	material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	g_pd3dDevice->SetMaterial(&material);
}

static HRESULT CreateFontResource(INT height, LPD3DXFONT* font)
{
	if (font == nullptr)
		return E_INVALIDARG;

	SafeRelease(*font);

	return D3DXCreateFont(
		g_pd3dDevice,
		height,
		0,
		FW_NORMAL,
		1,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		"Arial",
		font);
}

static HRESULT CreateFonts()
{
	const HRESULT clearFontResult =
		CreateFontResource(50, &g_pClearFont);

	if (FAILED(clearFontResult))
		return clearFontResult;

	const HRESULT settingFontResult =
		CreateFontResource(40, &g_pSettingFont);

	if (FAILED(settingFontResult))
		return settingFontResult;

	const HRESULT exitFontResult =
		CreateFontResource(30, &g_pExitFont);

	if (FAILED(exitFontResult))
		return exitFontResult;

	const HRESULT testFontResult =
		CreateFontResource(20, &g_pTestFont);

	if (FAILED(testFontResult))
		return testFontResult;

	const HRESULT frameFontResult =
		CreateFontResource(25, &g_pFrameFont);

	if (FAILED(frameFontResult))
		return frameFontResult;

	return S_OK;
}

static VOID NotifyFontsLostDevice()
{
	LPD3DXFONT fonts[] =
	{
		g_pClearFont,
		g_pSettingFont,
		g_pExitFont,
		g_pTestFont,
		g_pFrameFont
	};

	for (LPD3DXFONT font : fonts)
	{
		if (font != NULL)
		{
			font->OnLostDevice();
		}
	}
}

static VOID NotifyFontsResetDevice()
{
	LPD3DXFONT fonts[] =
	{
		g_pClearFont,
		g_pSettingFont,
		g_pExitFont,
		g_pTestFont,
		g_pFrameFont
	};

	for (LPD3DXFONT font : fonts)
	{
		if (font != NULL)
		{
			font->OnResetDevice();
		}
	}
}

static HRESULT ResetD3dDevice()
{
	if (g_pd3dDevice == NULL ||
		g_clientWidth == 0 ||
		g_clientHeight == 0)
	{
		return E_FAIL;
	}

	g_presentationParameters.BackBufferWidth = g_clientWidth;
	g_presentationParameters.BackBufferHeight = g_clientHeight;

	if (g_areFontResourcesLost == FALSE)
	{
		NotifyFontsLostDevice();
		g_areFontResourcesLost = TRUE;
	}

	const HRESULT resetResult = g_pd3dDevice->Reset(&g_presentationParameters);

	if (FAILED(resetResult))
		return resetResult;

	NotifyFontsResetDevice();
	g_areFontResourcesLost = FALSE;

	ConfigureDeviceRenderStates(g_pd3dDevice);
	ConfigureDefaultMaterial();

	g_isDeviceResetPending = FALSE;
	return S_OK;
}

static HRESULT LoadTextureResource(const char* path, LPDIRECT3DTEXTURE9* texture)
{
	if (path == nullptr || texture == nullptr)
		return E_INVALIDARG;

	SafeRelease(*texture);

	return D3DXCreateTextureFromFile(
		g_pd3dDevice,
		path,
		texture);
}

static HRESULT LoadSceneTextures()
{
	HRESULT textureResult =
		LoadTextureResource(kBulletTexturePath, &g_pBulletTexture);

	if (FAILED(textureResult))
		return textureResult;

	textureResult =
		LoadTextureResource(kPlayerMarkerTexturePath, &g_pPlayerMarkerTexture);

	if (FAILED(textureResult))
		return textureResult;

	textureResult =
		LoadTextureResource(kWallTexturePath, &g_pWallTexture);

	if (FAILED(textureResult))
		return textureResult;

	textureResult =
		LoadTextureResource(kGrassTexturePath, &g_pGrassTexture);

	if (FAILED(textureResult))
		return textureResult;

	SafeRelease(g_pNoticeTexture);

	textureResult = D3DXCreateTextureFromFileEx(
		g_pd3dDevice,
		kNoticeTexturePath,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,
		D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR,
		D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR,
		kTextureColorKey,
		nullptr,
		nullptr,
		&g_pNoticeTexture);

	if (FAILED(textureResult))
		return textureResult;

	textureResult =
		LoadTextureResource(kExitTexturePath, &g_pExitTexture);

	if (FAILED(textureResult))
		return textureResult;

	return S_OK;
}

static HRESULT CreatePrimitiveMeshes()
{
	SafeRelease(g_pBulletSphere);
	SafeRelease(g_pPlayerSphere);

	const HRESULT bulletMeshResult = D3DXCreateSphere(
		g_pd3dDevice,
		kBulletRadius,
		10,
		10,
		&g_pBulletSphere,
		nullptr);

	if (FAILED(bulletMeshResult))
		return bulletMeshResult;

	const HRESULT playerMeshResult = D3DXCreateSphere(
		g_pd3dDevice,
		kPlayerRadius,
		10,
		10,
		&g_pPlayerSphere,
		nullptr);

	if (FAILED(playerMeshResult))
	{
		SafeRelease(g_pBulletSphere);
		return playerMeshResult;
	}

	return S_OK;
}

static HRESULT CreateMazeGeometry()
{
	int i;

	g_mazeWallVertices = GenerateMazeWalls(g_maze);

	InitializeMazeEntities(g_maze, &g_notices, &g_mazeExit);

	const UINT mazeVertexDataSize = static_cast<UINT>(sizeof(MazeWallBlockVertices) * g_mazeWallVertices.size());

	const HRESULT mazeBufferResult = CreateManagedVertexBuffer(
		g_pd3dDevice,
		g_mazeWallVertices.data(),
		mazeVertexDataSize,
		D3DFVF_CUSTOMVERTEX,
		&g_pMazeVB);

	if (FAILED(mazeBufferResult))
		return mazeBufferResult;

	for (i = 0; i < static_cast<int>(g_notices.size()); i++)
	{
		const HRESULT noticeBufferResult = g_notices[i].CreateVertexBuffer(g_pd3dDevice);

		if (FAILED(noticeBufferResult))
			return noticeBufferResult;
	}

	return g_mazeExit.CreateVertexBuffer(g_pd3dDevice);
}

static HRESULT CreateTileGeometry()
{
	int i, j;

	const int mazeWidth = g_maze.GetWidth();
	const int mazeHeight = g_maze.GetHeight();
	const int tileCount = mazeWidth * mazeHeight;

	g_tileVertices.resize(static_cast<std::size_t>(tileCount) * kTileVertexCount);

	// 타일 정점 데이터 생성
	for (i = 0; i < tileCount; i++)
	{
		const int row = i / mazeWidth;
		const int column = i % mazeWidth;

		const FLOAT tileX = (column - mazeWidth / 2.0f) * kTileSize;
		const FLOAT tileZ = (mazeHeight / 2.0f - row) * kTileSize;

		// D3DFVF_NORMAL: 조명 계산에 사용할 타일의 위쪽 법선
		for (j = 0; j < kTileVertexCount; j++)
		{
			g_tileVertices[i * kTileVertexCount + j].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		}
		// D3DFVF_XYZ: 타일을 구성하는 네 꼭짓점 위치
		g_tileVertices[i * kTileVertexCount].position = D3DXVECTOR3(tileX, 0.0f, tileZ);
		g_tileVertices[i * kTileVertexCount + 1].position = D3DXVECTOR3(tileX + kTileSize, 0.0f, tileZ);
		g_tileVertices[i * kTileVertexCount + 2].position = D3DXVECTOR3(tileX + kTileSize, 0.0f, tileZ - kTileSize);
		g_tileVertices[i * kTileVertexCount + 3].position = D3DXVECTOR3(tileX, 0.0f, tileZ - kTileSize);
		// D3DFVF_TEX1: 타일 한 장에 대응하는 텍스처 좌표
		g_tileVertices[i * kTileVertexCount].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
		g_tileVertices[i * kTileVertexCount + 1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
		g_tileVertices[i * kTileVertexCount + 2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
		g_tileVertices[i * kTileVertexCount + 3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);
	}
	
	// 타일 정점 버퍼 생성
	const UINT tileVertexDataSize = static_cast<UINT>(sizeof(CustomVertex) * g_tileVertices.size());

	return CreateManagedVertexBuffer(
		g_pd3dDevice,
		g_tileVertices.data(),
		tileVertexDataSize,
		D3DFVF_CUSTOMVERTEX,
		&g_pTileVB);
}

static void AppendWallFace(
	std::vector<WallFaceVertices>* wallFaces,
	const D3DXVECTOR3& first,
	const D3DXVECTOR3& second,
	const D3DXVECTOR3& third,
	const D3DXVECTOR3& fourth,
	const D3DXVECTOR3& normal)
{
	wallFaces->emplace_back();
	WallFaceVertices& face = wallFaces->back();

	for (CustomVertex& vertex : face)
	{
		vertex.normal = normal;
	}

	face[0].position = first;
	face[1].position = second;
	face[2].position = third;
	face[3].position = fourth;

	face[0].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
	face[1].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
	face[2].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
	face[3].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);
}

static HRESULT CreateOuterWallGeometry()
{
	const int width = g_maze.GetWidth();
	const int height = g_maze.GetHeight();
	const float halfWidth = width / 2.0f;
	const float halfHeight = height / 2.0f;

	g_outerWallVertices.clear();
	g_outerWallVertices.reserve(2 * (width + height));

	// 가로 벽 동적 생성
	for (int i = 0; i < width; i++)
	{
		const float minX = (i - halfWidth) * kTileSize;
		const float maxX = minX + kTileSize;

		AppendWallFace(&g_outerWallVertices,
			{ minX, kTileSize, halfHeight * kTileSize },
			{ maxX, kTileSize, halfHeight * kTileSize },
			{ maxX, 0.0f, halfHeight * kTileSize },
			{ minX, 0.0f, halfHeight * kTileSize },
			{ 0.0f, 0.0f, -1.0f });

		AppendWallFace(&g_outerWallVertices,
			{ -minX, kTileSize, -halfHeight * kTileSize },
			{ -maxX, kTileSize, -halfHeight * kTileSize },
			{ -maxX, 0.0f, -halfHeight * kTileSize },
			{ -minX, 0.0f, -halfHeight * kTileSize },
			{ 0.0f, 0.0f, 1.0f });
	}
	// 세로 벽 동적 생성
	for (int i = 0; i < height; i++)
	{
		const float minZ = (i - halfHeight) * kTileSize;
		const float maxZ = minZ + kTileSize;

		AppendWallFace(&g_outerWallVertices,
			{ -halfWidth * kTileSize, kTileSize, minZ },
			{ -halfWidth * kTileSize, kTileSize, maxZ },
			{ -halfWidth * kTileSize, 0.0f, maxZ },
			{ -halfWidth * kTileSize, 0.0f, minZ },
			{ 1.0f, 0.0f, 0.0f });

		AppendWallFace(&g_outerWallVertices,
			{ halfWidth * kTileSize, kTileSize, -minZ },
			{ halfWidth * kTileSize, kTileSize, -maxZ },
			{ halfWidth * kTileSize, 0.0f, -maxZ },
			{ halfWidth * kTileSize, 0.0f, -minZ },
			{ -1.0f, 0.0f, 0.0f });
	}

	const UINT dataSize = static_cast<UINT>(sizeof(WallFaceVertices) * g_outerWallVertices.size());

	return CreateManagedVertexBuffer(
		g_pd3dDevice,
		g_outerWallVertices.data(),
		dataSize,
		D3DFVF_CUSTOMVERTEX,
		&g_pWallVB);
}

static HRESULT CreateUpperWallGeometry()
{
	const int width = g_maze.GetWidth();
	const int height = g_maze.GetHeight();
	const float halfWidth = width / 2.0f;
	const float halfHeight = height / 2.0f;
	const D3DXVECTOR3 upwardNormal(0.0f, 1.0f, 0.0f);

	g_upperWallVertices.clear();
	g_upperWallVertices.reserve(2 * (width + height));

	for (int i = 0; i < width; i++)
	{
		const float minX = (i - halfWidth) * kTileSize;
		const float maxX = minX + kTileSize;

		AppendWallFace(&g_upperWallVertices,
			{ minX, kTileSize, (halfHeight + 1.0f) * kTileSize },
			{ maxX, kTileSize, (halfHeight + 1.0f) * kTileSize },
			{ maxX, kTileSize, halfHeight * kTileSize },
			{ minX, kTileSize, halfHeight * kTileSize },
			upwardNormal);

		AppendWallFace(&g_upperWallVertices,
			{ -minX, kTileSize, (-halfHeight - 1.0f) * kTileSize },
			{ -maxX, kTileSize, (-halfHeight - 1.0f) * kTileSize },
			{ -maxX, kTileSize, -halfHeight * kTileSize },
			{ -minX, kTileSize, -halfHeight * kTileSize },
			upwardNormal);
	}

	for (int i = 0; i < height; i++)
	{
		const float minZ = (i - halfHeight) * kTileSize;
		const float maxZ = minZ + kTileSize;

		AppendWallFace(&g_upperWallVertices,
			{ (-halfWidth - 1.0f) * kTileSize, kTileSize, minZ },
			{ (-halfWidth - 1.0f) * kTileSize, kTileSize, maxZ },
			{ -halfWidth * kTileSize, kTileSize, maxZ },
			{ -halfWidth * kTileSize, kTileSize, minZ },
			upwardNormal);

		AppendWallFace(&g_upperWallVertices,
			{ (halfWidth + 1.0f) * kTileSize, kTileSize, -minZ },
			{ (halfWidth + 1.0f) * kTileSize, kTileSize, -maxZ },
			{ halfWidth * kTileSize, kTileSize, -maxZ },
			{ halfWidth * kTileSize, kTileSize, -minZ },
			upwardNormal);
	}

	const UINT dataSize = static_cast<UINT>(sizeof(WallFaceVertices) * g_upperWallVertices.size());

	return CreateManagedVertexBuffer(
		g_pd3dDevice,
		g_upperWallVertices.data(),
		dataSize,
		D3DFVF_CUSTOMVERTEX,
		&g_pWallVB2);
}

static HRESULT CreateLevelGeometry()
{
	HRESULT creationResult = CreateMazeGeometry();

	if (FAILED(creationResult))
		return creationResult;

	creationResult = CreateTileGeometry();

	if (FAILED(creationResult))
		return creationResult;

	creationResult = CreateOuterWallGeometry();

	if (FAILED(creationResult))
		return creationResult;

	return CreateUpperWallGeometry();
}

static HRESULT InitializeResources()
{
	HRESULT initializationResult = InitializeMaze();

	if (FAILED(initializationResult))
		return initializationResult;

	initializationResult = InitializeGameComponents();

	if (FAILED(initializationResult))
		return initializationResult;

	ConfigureDefaultMaterial();

	initializationResult = CreatePrimitiveMeshes();

	if (FAILED(initializationResult))
		return initializationResult;

	initializationResult = CreateFonts();

	if (FAILED(initializationResult))
		return initializationResult;

	initializationResult = LoadSceneTextures();

	if (FAILED(initializationResult))
		return initializationResult;

	initializationResult = CreateLevelGeometry();

	if (FAILED(initializationResult))
		return initializationResult;

	UpdateBillboardFacing();

	return S_OK;
}

static VOID ReleasePrimitiveMeshes()
{
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
	SafeRelease(g_pExitTexture);
	SafeRelease(g_pNoticeTexture);
	SafeRelease(g_pGrassTexture);
	SafeRelease(g_pWallTexture);
	SafeRelease(g_pPlayerMarkerTexture);
	SafeRelease(g_pBulletTexture);
}

static VOID ReleaseGeometryBuffers()
{
	g_mazeExit.ReleaseVertexBuffer();

	for (Notice& notice : g_notices)
	{
		notice.ReleaseVertexBuffer();
	}

	g_notices.clear();

	SafeRelease(g_pMazeVB);
	SafeRelease(g_pWallVB2);
	SafeRelease(g_pWallVB);
	SafeRelease(g_pTileVB);

	g_mazeWallVertices.clear();
	g_upperWallVertices.clear();
	g_outerWallVertices.clear();
	g_tileVertices.clear();
}

static HRESULT TransitionToLevel(size_t levelIndex)
{
	const HRESULT loadResult = LoadLevelMaze(levelIndex);

	if (FAILED(loadResult))
		return loadResult;

	ReleaseGeometryBuffers();

	const HRESULT geometryResult = CreateLevelGeometry();

	if (FAILED(geometryResult))
	{
		ReleaseGeometryBuffers();

		g_initializationErrorMessage =
			"Failed to create geometry for level: " +
			g_levelPaths[levelIndex];

		return geometryResult;
	}

	ResetLevelEntities();
	ResetLevelPlayState();
	UpdateBillboardFacing();

	return S_OK;
}

static VOID ReleaseResources()
{
	g_skyBox.ReleaseResources();
	g_tiger.ReleaseResources();

	ReleaseGeometryBuffers();
	ReleaseSceneTextures();
	ReleaseFonts();
	ReleasePrimitiveMeshes();

	SafeRelease(g_pd3dDevice);
	SafeRelease(g_pD3D);
}

static VOID HandleMovementInput(FLOAT deltaTimeSeconds)
{
	g_didPlayerMove = FALSE;

	if (IsKeyDown('A') || IsKeyDown(VK_LEFT))
	{
		g_didPlayerMove = g_player.Move(
			MoveDirection::Left,
			g_maze,
			g_isNoClipEnabled,
			deltaTimeSeconds) || g_didPlayerMove;
	}

	if (IsKeyDown('D') || IsKeyDown(VK_RIGHT))
	{
		g_didPlayerMove = g_player.Move(
			MoveDirection::Right,
			g_maze,
			g_isNoClipEnabled,
			deltaTimeSeconds) || g_didPlayerMove;
	}

	if (IsKeyDown('W') || IsKeyDown(VK_UP))
	{
		g_didPlayerMove = g_player.Move(
			MoveDirection::Forward,
			g_maze,
			g_isNoClipEnabled,
			deltaTimeSeconds) || g_didPlayerMove;
	}

	if (IsKeyDown('S') || IsKeyDown(VK_DOWN))
	{
		g_didPlayerMove = g_player.Move(
			MoveDirection::Backward,
			g_maze,
			g_isNoClipEnabled,
			deltaTimeSeconds) || g_didPlayerMove;
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

		if (g_isPaused == FALSE)
		{
			SetCursorPos(g_cursorCenter.x, g_cursorCenter.y);
		}
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

#ifdef _DEBUG
	// 컬링 확인용 탑뷰
	if (IsKeyPressed(VK_F2) == TRUE)
	{
		g_topViewMode =
			g_topViewMode == TopViewMode::CullingDebug
			? TopViewMode::Disabled
			: TopViewMode::CullingDebug;
	}
#endif // _DEBUG

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

			g_didPlayerMove = TRUE;
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
	g_tiger.Move(g_maze, deltaTimeSeconds);
}

static VOID UpdateInteractionState()
{
	if (g_didPlayerMove)
	{
		UpdateBillboardFacing();
	}

	bool isNoticeInRange = false;

	for (Notice& notice : g_notices)
	{
		if (notice.CanInteract(g_player.GetPosition(), g_isNoClipEnabled) == TRUE)
		{
			isNoticeInRange = true;
			break;
		}
	}

	if (g_topViewMode != TopViewMode::CullingDebug)
	{
		g_topViewMode = isNoticeInRange
			? TopViewMode::NoticeHint
			: TopViewMode::Disabled;
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

	// 추가 기능 : 1, 3, 4 및 Debug 기능
	HandleFeatureToggleInput();

	// Notice & Exit rotation
	UpdateInteractionState();

	// Q/E : 플레이어 CCW/CW 회전
	HandleRotationInput(deltaTimeSeconds);

	// 스페이스
	HandleJumpInput();
}

static VOID RenderUi()
{
	const LONG clientWidth = static_cast<LONG>(g_clientWidth);
	RECT textRect;
	char textBuffer[500];

	g_pd3dDevice->SetTexture(0, NULL);
	g_pd3dDevice->SetFVF(D3DFVF_UI_VERTEX);
	// 탈출구 UI
	if (!g_isPlaying)
	{
		g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, g_popupVertices, sizeof(UiVertex));
		wsprintf(textBuffer, "C L E A R");

		textRect = g_clearPopupRect;
		textRect.top += 50;
		textRect.bottom = textRect.top + 60;

		g_pClearFont->DrawTextA(
			NULL,
			textBuffer,
			-1,
			&textRect,
			DT_CENTER | DT_TOP,
			D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

		g_mazeExit.RenderButton(g_pd3dDevice);
		wsprintf(
			textBuffer,
			HasNextLevel() ? "N E X T" : "e x i t");

		textRect = g_exitButtonRect;

		g_pExitFont->DrawTextA(
			NULL,
			textBuffer,
			-1,
			&textRect,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	}

	// 좌상단 UI
	if (!IsTopViewActive())
	{
		g_pd3dDevice->SetTexture(0, NULL);
		g_pd3dDevice->SetFVF(D3DFVF_UI_VERTEX);
		g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, g_uiVertices, sizeof(UiVertex));

		// 조작 안내 UI
		SetRect(&textRect, 20, 20, 0, 0);
		wsprintf(textBuffer, " 1: 낮밤 전환\n 3: 손전등 on/off\n 4: 자유시점 on/off\n esc: 일시 정지");
		g_pTestFont->DrawTextA(NULL, textBuffer, -1, &textRect, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
	}

	// 환경설정 및 일시정지 UI
	if (g_isPaused)
	{
		g_settingsOverlay.Render(g_pd3dDevice);
		wsprintf(textBuffer, "P A U S E");

		textRect = g_pauseOverlayRect;
		textRect.top += 100;
		textRect.bottom = textRect.top + 50;

		g_pSettingFont->DrawTextA(
			NULL,
			textBuffer,
			-1,
			&textRect,
			DT_CENTER | DT_TOP,
			D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

		g_mazeExit.RenderButton(g_pd3dDevice);
		wsprintf(textBuffer, "e x i t");

		textRect = g_exitButtonRect;

		g_pExitFont->DrawTextA(
			NULL,
			textBuffer,
			-1,
			&textRect,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	}

	// 자유시점 표시
	if (g_isNoClipEnabled)
	{
		wsprintf(textBuffer, "자유시점 ON");
		// 텍스트 너비를 측정해 화면 상단 중앙에 배치한다.
		SetRect(&textRect, 0, 0, 0, 0);
		g_pFrameFont->DrawTextA(NULL, textBuffer, -1, &textRect, DT_CALCRECT, D3DCOLOR_ARGB(0, 0, 0, 0));
		int width = textRect.right - textRect.left;
		SetRect(&textRect, clientWidth / 2 - width / 2, 0, 0, 0);
		g_pFrameFont->DrawTextA(NULL, textBuffer, -1, &textRect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 0));
	}

	// 성능 지표 표시
	SetRect(&textRect, clientWidth - 220, 0, clientWidth - 20, 50);
	std::snprintf(
		textBuffer,
		sizeof(textBuffer),
		"FPS: %3d\nFrame: %.2f ms",
		g_fpsCounter.GetFps(),
		g_fpsCounter.GetAverageFrameTimeMilliseconds());
	g_pFrameFont->DrawTextA(
		NULL,
		textBuffer,
		-1,
		&textRect,
		DT_RIGHT | DT_TOP,
		D3DCOLOR_XRGB(0, 255, 0));
}

static VOID ConfigureLighting()
{
	D3DLIGHT9* playerLight = g_player.GetLight();
	// TODO: IsTopViewActive() == true 이면 player의 spot light,
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
	if (!IsTopViewActive())
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

	FLOAT aspectRatio = 1.0f;

	if (g_clientWidth > 0 && g_clientHeight > 0)
	{
		aspectRatio = static_cast<FLOAT>(g_clientWidth) / static_cast<FLOAT>(g_clientHeight);
	}

	D3DXMATRIX projectionMatrix;
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, D3DX_PI / 4, aspectRatio, 0.1f, 1000.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &projectionMatrix);

	// frustum plane을 계산할, view matrix와 projection matrix의 곱
	D3DXMATRIX viewProjectionMatrix;

	// 개발용 탑뷰에서는 플레이어 카메라의 컬링 결과를 표시한다.
	if (g_topViewMode == TopViewMode::CullingDebug)
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
	D3DXVECTOR3 tileHalfExtents;

	g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	// 스카이박스
	g_skyBox.Render(g_pd3dDevice);

	g_pd3dDevice->SetTexture(0, g_pGrassTexture);
	g_pd3dDevice->SetStreamSource(0, g_pTileVB, 0, sizeof(CustomVertex));

	// 바닥 타일 AABB 프러스텀 컬링
	const int tileCount = static_cast<int>(g_tileVertices.size() / kTileVertexCount);

	for (i = 0; i < tileCount; i++)
	{
		const D3DXVECTOR3& firstPoint = g_tileVertices[i * kTileVertexCount].position;
		const D3DXVECTOR3& oppositePoint = g_tileVertices[i * kTileVertexCount + 2].position;

		tileCenter = CalculateMidPoint(firstPoint, oppositePoint);
		tileHalfExtents = CalculateAabbHalfExtents(firstPoint, oppositePoint);

		if (g_frustum.IntersectsAabb(&tileCenter, &tileHalfExtents) == TRUE)
		{
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * kTileVertexCount, 2);
		}
	}

	// 외벽과 미로 벽
	g_pd3dDevice->SetTexture(0, g_pWallTexture);

	g_pd3dDevice->SetStreamSource(0, g_pWallVB, 0, sizeof(CustomVertex));
	// 외벽 측면 AABB 컬링
	for (i = 0; i < static_cast<int>(g_outerWallVertices.size()); i++)
	{
		const D3DXVECTOR3& firstPoint = g_outerWallVertices[i][0].position;
		const D3DXVECTOR3& oppositePoint = g_outerWallVertices[i][2].position;


		tileCenter = CalculateMidPoint(firstPoint, oppositePoint);
		tileHalfExtents = CalculateAabbHalfExtents(firstPoint, oppositePoint);

		if (g_frustum.IntersectsAabb(&tileCenter, &tileHalfExtents) == TRUE)
		{
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * kVerticesPerWallFace, 2);
		}
	}

	g_pd3dDevice->SetStreamSource(0, g_pWallVB2, 0, sizeof(CustomVertex));
	// 외벽 상단 AABB 컬링
	for (i = 0; i < static_cast<int>(g_upperWallVertices.size()); i++)
	{
		const D3DXVECTOR3& firstPoint = g_upperWallVertices[i][0].position;
		const D3DXVECTOR3& oppositePoint = g_upperWallVertices[i][2].position;

		tileCenter = CalculateMidPoint(firstPoint, oppositePoint);
		tileHalfExtents = CalculateAabbHalfExtents(firstPoint, oppositePoint);

		if (g_frustum.IntersectsAabb(&tileCenter, &tileHalfExtents) == TRUE)
		{
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * kVerticesPerWallFace, 2);
		}
	}

	g_pd3dDevice->SetStreamSource(0, g_pMazeVB, 0, sizeof(CustomVertex));
	// 미로 내부 벽 AABB 컬링
	for (i = 0; i < static_cast<int>(g_mazeWallVertices.size()); i++)
	{
		for (j = 0; j < kWallBlockFaceCount; j++)
		{
			const int vertexOffset = j * kVerticesPerWallFace;

			const D3DXVECTOR3& firstPoint = g_mazeWallVertices[i][vertexOffset].position;
			const D3DXVECTOR3& oppositePoint = g_mazeWallVertices[i][vertexOffset + 2].position;

			tileCenter = CalculateMidPoint(firstPoint, oppositePoint);
			tileHalfExtents = CalculateAabbHalfExtents(firstPoint, oppositePoint);

			if (g_frustum.IntersectsAabb(&tileCenter, &tileHalfExtents) == TRUE)
			{
				g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * kWallBlockVertexCount + vertexOffset, 2);
			}
		}
	}

	// 안내문
	g_pd3dDevice->SetTexture(0, g_pNoticeTexture);
	D3DXMATRIX noticeWorldMatrix;
	for (Notice& notice : g_notices)
	{
		noticeWorldMatrix = notice.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &noticeWorldMatrix);
		notice.Render(g_pd3dDevice);
	}

	// 출구
	g_pd3dDevice->SetTexture(0, g_pExitTexture);
	noticeWorldMatrix = g_mazeExit.GetWorldMatrix();
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &noticeWorldMatrix);
	g_mazeExit.Render(g_pd3dDevice);

	// 총알
	g_pd3dDevice->SetTexture(0, g_pBulletTexture);
	g_player.RenderBullets(g_pd3dDevice, g_pBulletSphere);

	// 탑뷰에서 플레이어 위치를 구체로 표시
	if (IsTopViewActive())
	{
		D3DXMATRIX playerWorldMatrix;
		D3DXMatrixTranslation(&playerWorldMatrix, playerPosition.x, playerPosition.y, playerPosition.z);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &playerWorldMatrix);
		g_pd3dDevice->SetTexture(0, g_pPlayerMarkerTexture);
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

static VOID UpdateCursorCenter(HWND windowHandle)
{
	RECT clientRect;
	if (GetClientRect(windowHandle, &clientRect) == FALSE)
		return;

	POINT clientCenter =
	{
		(clientRect.left + clientRect.right) / 2,
		(clientRect.top + clientRect.bottom) / 2
	};

	if (ClientToScreen(windowHandle, &clientCenter) == FALSE)
		return;

	g_cursorCenter = clientCenter;
}

static LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		UpdateCursorCenter(hWnd);
		SetCursorPos(g_cursorCenter.x, g_cursorCenter.y);
		g_cursorDisplayCount = ShowCursor(FALSE);
		break;

	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* minMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);

		minMaxInfo->ptMinTrackSize.x = kMinimumWindowTrackWidth;
		minMaxInfo->ptMinTrackSize.y = kMinimumWindowTrackHeight;

		break;
	}

	case WM_MOVE:
		UpdateCursorCenter(hWnd);
		break;

	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			g_clientWidth = LOWORD(lParam);
			g_clientHeight = HIWORD(lParam);
			UpdateUiLayout();

			if (g_clientWidth > 0 &&
				g_clientHeight > 0 &&
				g_pd3dDevice != NULL)
			{
				// 장치 리셋은 메인 루프에서 처리한다.
				g_isDeviceResetPending = TRUE;
			}

			UpdateCursorCenter(hWnd);
		}
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
		if (!IsTopViewActive() && g_isPlaying && !g_isPaused)
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

		if (PtInRect(&g_exitButtonRect, g_mousePosition))
		{
			if (!g_isPlaying)
			{
				if (HasNextLevel())
				{
					const HRESULT transitionResult = TransitionToLevel(g_currentLevelIndex + 1);

					if (FAILED(transitionResult))
					{
						MessageBoxA(
							hWnd,
							g_initializationErrorMessage.c_str(),
							"Level Transition Failed",
							MB_OK | MB_ICONERROR);

						PostMessage(hWnd, WM_CLOSE, 0, 0);
					}
				}
				else
				{
					PostMessage(hWnd, WM_CLOSE, 0, 0);
				}
			}
			else if (g_isPaused)
			{
				PostMessage(hWnd, WM_CLOSE, 0, 0);
			}
		}

		break;

	case WM_DESTROY:
		while (g_cursorDisplayCount < 0)
		{
			g_cursorDisplayCount = ShowCursor(TRUE);
		}

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

	// 모니터 중앙에 게임 창 생성
	RECT workArea =
	{
		0,
		0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN)
	};

	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

	const int workAreaWidth = workArea.right - workArea.left;
	const int workAreaHeight = workArea.bottom - workArea.top;

	const int windowX = workArea.left + (workAreaWidth - kWindowWidth) / 2;
	const int windowY = workArea.top + (workAreaHeight - kWindowHeight) / 2;

	// 윈도우 생성
	HWND windowHandle = CreateWindow(
		kProgramName,
		kProgramName,
		WS_OVERLAPPEDWINDOW,
		windowX,
		windowY,
		kWindowWidth,
		kWindowHeight,
		GetDesktopWindow(),
		NULL,
		windowClass.hInstance,
		NULL);

	// Direct3D 초기화
	HRESULT initializationResult = InitializeD3d(windowHandle);

	if (SUCCEEDED(initializationResult))
		initializationResult = InitializeResources();

	if (SUCCEEDED(initializationResult))
	{
		// 윈도우 출력
		ShowWindow(windowHandle, SW_SHOWDEFAULT);
		UpdateWindow(windowHandle);
		// 메시지 루프
		MSG msg;
		ZeroMemory(&msg, sizeof(msg));

		LARGE_INTEGER performanceFrequency;
		LARGE_INTEGER previousFrameCounter;

		QueryPerformanceFrequency(&performanceFrequency);
		QueryPerformanceCounter(&previousFrameCounter);

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				LARGE_INTEGER currentFrameCounter;
				QueryPerformanceCounter(&currentFrameCounter);

				const LONGLONG elapsedCounts = currentFrameCounter.QuadPart - previousFrameCounter.QuadPart;

				const FLOAT frameTimeSeconds = static_cast<FLOAT>(
					static_cast<double>(elapsedCounts) /
					static_cast<double>(performanceFrequency.QuadPart));

				FLOAT deltaTimeSeconds = frameTimeSeconds;

				if (deltaTimeSeconds > kMaxDeltaTimeSeconds)
					deltaTimeSeconds = kMaxDeltaTimeSeconds;

				previousFrameCounter = currentFrameCounter;

				if (g_isDeviceResetPending == TRUE)
				{
					if (FAILED(ResetD3dDevice()))
						continue;
				}

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
				g_fpsCounter.Update(frameTimeSeconds);
			}
		}
	}
	else
	{
		while (g_cursorDisplayCount < 0)
		{
			g_cursorDisplayCount = ShowCursor(TRUE);
		}

		const char* initializationErrorMessage = g_initializationErrorMessage.empty()
			? "게임 초기화에 실패했습니다."
			: g_initializationErrorMessage.c_str();

		MessageBox(
			windowHandle,
			initializationErrorMessage,
			kProgramName,
			MB_OK | MB_ICONERROR);

		ReleaseResources();

		if (windowHandle != NULL)
			DestroyWindow(windowHandle);
	}

	// 등록된 클래스 소거
	UnregisterClass(kProgramName, windowClass.hInstance);
	return 0;
}
