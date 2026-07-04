
#include "maze_function.h"
#include "Input.h"
#include "CFrustum.h"
#include "CFrame.h"
#include "CSkyBox.h"
#include "XFileUtil.h"

LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;

LPDIRECT3DVERTEXBUFFER9 g_pTileVB = NULL;
LPDIRECT3DINDEXBUFFER9 g_pTileIB = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pWallVB = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pWallVB2 = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pMazeVB = NULL;
LPDIRECT3DTEXTURE9 g_pTileTexture = NULL;
LPDIRECT3DTEXTURE9 g_pWallTexture = NULL;
LPDIRECT3DTEXTURE9 g_pGrassTexture = NULL;
LPDIRECT3DTEXTURE9 g_pNoticeTexture = NULL;
LPDIRECT3DTEXTURE9 g_pExitTexture = NULL;
LPDIRECT3DCUBETEXTURE9 g_pSkyboxTexture = NULL;
LPDIRECT3DSURFACE9 g_pFace2 = NULL;
LPD3DXFONT g_pClearFont = NULL;
LPD3DXFONT g_pSettingFont = NULL;
LPD3DXFONT g_pExitFont = NULL;
LPD3DXFONT g_pFrameFont = NULL;
LPD3DXFONT g_pTestFont = NULL;
LPD3DXMESH g_pPlayerSphere = NULL;
LPD3DXMESH g_pBulletSphere = NULL;
LPD3DXMESH g_pSkyboxCube = NULL;
LPPOINT g_pMidPoint = new POINT; // 마우스는 클라이언트 중앙으로 고정
LPPOINT g_pMouse = new POINT;
LPPOINT g_pCurrentMouse = new POINT; // 마우스 이동 시, 이동한 좌표 받아올 임시 마우스 좌표

D3DMATERIAL9 material;
D3DLIGHT9 skyLight;

DWORD dwRotationTime = timeGetTime();

CPlayer player;
vector<CNotice> notice;
CExit Exit;
CSetting setting;
CFrame FPS;
// 프레임 값에 상관없이 플레이어 및 호랑이 움직임이 일정하도록 조절하기 위한 스톱워치
Stopwatch PlayerWatch, TigerWatch;
CXFileUtil X_Tiger(D3DXVECTOR3(55.0f, 5.0f, 65.0f));
CSkyBox SkyBox;

CFrustum* g_pFrustum = new CFrustum;
RECT rt, rtExitButton;

char testSTR[500];
wchar_t test2[500];

HRESULT InitD3D(HWND hWnd)
{
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // FPS 설정을 위한 설정?
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
		return E_FAIL;

	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0);

	g_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

	return S_OK;
}

VOID InitGeometry()
{
	int i, j;

	InitInput();
	FPS.Initialize();
	// skybox texture load
	SkyBox.LoadTexture();
	SkyBox.CreateVertexBuffer();
	// tiger initialization
	X_Tiger.XFileLoad(g_pd3dDevice, chFileName);
	X_Tiger.SetPosition(D3DXVECTOR3(55.0f, 5.0f, 65.0f));
	X_Tiger.SetLookAt(player.GetPosition());

	SetRect(&rtExitButton, 300, 450, 400, 500);

	ZeroMemory(&material, sizeof(D3DMATERIAL9));
	material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	g_pd3dDevice->SetMaterial(&material);

	// sphere object: player, bullet
	D3DXCreateSphere(g_pd3dDevice, BULLET_RADIUS, 10, 10, &g_pBulletSphere, 0);
	D3DXCreateSphere(g_pd3dDevice, PLAYER_RADIUS, 10, 10, &g_pPlayerSphere, 0);
	// cube object: skybox
	D3DXCreateBox(g_pd3dDevice, LENGTH_OF_SKYBOX_SURFACE, LENGTH_OF_SKYBOX_SURFACE, LENGTH_OF_SKYBOX_SURFACE, &g_pSkyboxCube, 0);
	// font
	D3DXCreateFont(g_pd3dDevice, 50, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pClearFont);
	D3DXCreateFont(g_pd3dDevice, 40, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pSettingFont);
	D3DXCreateFont(g_pd3dDevice, 30, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pExitFont);
	D3DXCreateFont(g_pd3dDevice, 20, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pTestFont);
	D3DXCreateFont(g_pd3dDevice, 25, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pFrameFont);

	// texture
	D3DXCreateTextureFromFile(g_pd3dDevice, TEXTURE_TILE, &g_pTileTexture);
	D3DXCreateTextureFromFile(g_pd3dDevice, TEXTURE_WALL, &g_pWallTexture);
	D3DXCreateTextureFromFile(g_pd3dDevice, TEXTURE_GRASS, &g_pGrassTexture);
	D3DXCreateTextureFromFileEx(g_pd3dDevice, TEXTURE_NOTICE,
		D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR,
		D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR,
		TRANSPARENCY_COLOR,
		NULL, NULL,
		&g_pNoticeTexture);
	D3DXCreateTextureFromFile(g_pd3dDevice, TEXTURE_EXIT, &g_pExitTexture);
	D3DXCreateCubeTextureFromFile(g_pd3dDevice, TEXTURE_SKYBOX, &g_pSkyboxTexture);

	//// 미궁 내 벽을 구성할 vertex들의 buffer 생성
	GenerateMazeWall(1, MazeWallVertices, &notice, &Exit);
	g_pd3dDevice->CreateVertexBuffer(sizeof(CUSTOMVERTEX) * 72 * 20, 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pMazeVB, NULL);
	VOID** MazeVertices;
	g_pMazeVB->Lock(0, sizeof(CUSTOMVERTEX) * 72 * 20, (void**)&MazeVertices, 0);
	memcpy(MazeVertices, MazeWallVertices, sizeof(CUSTOMVERTEX) * 72 * 20);
	g_pMazeVB->Unlock();

	//// Notice를 구성하는 vertex buffer 생성
	for (i = 0; i < notice[0].GetNumOfNotice(); i++)
	{
		notice[i].MakeNoticeVB(g_pd3dDevice);
	}
	//// Exit vertex buffer 생성
	Exit.MakeNoticeVB(g_pd3dDevice);

	// tile vertex 좌표 입력
	{
		for (i = 0; i < NUM_OF_ROW * NUM_OF_COLUMN; i++)
		{
			FLOAT fCoX = (FLOAT)((i % NUM_OF_COLUMN - NUM_OF_COLUMN / 2.0f) * LENGTH_OF_TILE);
			FLOAT fCoZ = (FLOAT)((NUM_OF_ROW / 2.0f - i / NUM_OF_COLUMN) * LENGTH_OF_TILE);
			// D3DFVF_NORMAL
			for (j = 0; j < 4; j++)
			{
				TileVertices[i * 4 + j].v3VerNormal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

			}
			// D3DFVF_XYZ
			TileVertices[i * 4].v3VerPos = D3DXVECTOR3(fCoX, 0.0f, fCoZ);
			TileVertices[i * 4 + 1].v3VerPos = D3DXVECTOR3(fCoX + LENGTH_OF_TILE, 0.0f, fCoZ);
			TileVertices[i * 4 + 2].v3VerPos = D3DXVECTOR3(fCoX + LENGTH_OF_TILE, 0.0f, fCoZ - LENGTH_OF_TILE);
			TileVertices[i * 4 + 3].v3VerPos = D3DXVECTOR3(fCoX, 0.0f, fCoZ - LENGTH_OF_TILE);
			// D3DFVF_TEX1
			TileVertices[i * 4].v2VerTex = D3DXVECTOR2(0.0f, 0.0f);
			TileVertices[i * 4 + 1].v2VerTex = D3DXVECTOR2(1.0f, 0.0f);
			TileVertices[i * 4 + 2].v2VerTex = D3DXVECTOR2(1.0f, 1.0f);
			TileVertices[i * 4 + 3].v2VerTex = D3DXVECTOR2(0.0f, 1.0f);
		}
	}
	// tile index 입력
	{
		j = 0;
		for (i = 0; i < NUM_OF_ROW * NUM_OF_COLUMN; i++)
		{
			wTileIndices[j][0] = i * 4;
			wTileIndices[j][1] = i * 4 + 1;
			wTileIndices[j][2] = i * 4 + 2;
			wTileIndices[++j][0] = i * 4;
			wTileIndices[j][1] = i * 4 + 2;
			wTileIndices[j++][2] = i * 4 + 3;
		}
	}
	// Create tile vertex & index buffer
	g_pd3dDevice->CreateVertexBuffer(sizeof(TileVertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pTileVB, NULL);
	VOID* pVertices2;
	g_pTileVB->Lock(0, sizeof(TileVertices), (void**)&pVertices2, 0);
	memcpy(pVertices2, TileVertices, sizeof(TileVertices));
	g_pTileVB->Unlock();

	g_pd3dDevice->CreateIndexBuffer(sizeof(wTileIndices), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pTileIB, NULL);
	VOID* pIndices2;
	g_pTileIB->Lock(0, sizeof(wTileIndices), (void**)&pIndices2, 0);
	memcpy(pIndices2, wTileIndices, sizeof(wTileIndices));
	g_pTileIB->Unlock();

	//// wall vertex 좌표 입력 - 맵 외곽
	{
		// 위쪽 면
		for (i = 0; i < NUM_OF_ROW; i++)
		{
			for (j = 0; j < 4; j++)
				WallVertices[0][i * 4 + j].v3VerNormal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

			WallVertices[0][i * 4].v3VerPos = D3DXVECTOR3((i - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE, 10.0f, (NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices[0][i * 4 + 1].v3VerPos = D3DXVECTOR3((i + 1 - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE, 10.0f, (NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices[0][i * 4 + 2].v3VerPos = D3DXVECTOR3((i + 1 - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE, 0.0f, (NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices[0][i * 4 + 3].v3VerPos = D3DXVECTOR3((i - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE, 0.0f, (NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);

			WallVertices[0][i * 4].v2VerTex = D3DXVECTOR2(0.0f, 0.0f);
			WallVertices[0][i * 4 + 1].v2VerTex = D3DXVECTOR2(1.0f, 0.0f);
			WallVertices[0][i * 4 + 2].v2VerTex = D3DXVECTOR2(1.0f, 1.0f);
			WallVertices[0][i * 4 + 3].v2VerTex = D3DXVECTOR2(0.0f, 1.0f);
		}
		// 아래쪽 면
		for (i = 0; i < NUM_OF_ROW; i++)
		{
			for (j = 0; j < 4; j++)
				WallVertices[1][i * 4 + j].v3VerNormal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

			WallVertices[1][i * 4].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - i) * LENGTH_OF_TILE, 10.0f, (-NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices[1][i * 4 + 1].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - i - 1) * LENGTH_OF_TILE, 10.0f, (-NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices[1][i * 4 + 2].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - i - 1) * LENGTH_OF_TILE, 0.0f, (-NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices[1][i * 4 + 3].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - i) * LENGTH_OF_TILE, 0.0f, (-NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);

			WallVertices[1][i * 4].v2VerTex = D3DXVECTOR2(0.0f, 0.0f);
			WallVertices[1][i * 4 + 1].v2VerTex = D3DXVECTOR2(1.0f, 0.0f);
			WallVertices[1][i * 4 + 2].v2VerTex = D3DXVECTOR2(1.0f, 1.0f);
			WallVertices[1][i * 4 + 3].v2VerTex = D3DXVECTOR2(0.0f, 1.0f);
		}
		// 왼쪽 면
		for (i = 0; i < NUM_OF_ROW; i++)
		{
			for (j = 0; j < 4; j++)
				WallVertices[2][i * 4 + j].v3VerNormal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);

			WallVertices[2][i * 4].v3VerPos = D3DXVECTOR3((-NUM_OF_ROW / 2.0f + 1) * LENGTH_OF_TILE, 10.0f, (i - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices[2][i * 4 + 1].v3VerPos = D3DXVECTOR3((-NUM_OF_ROW / 2.0f + 1) * LENGTH_OF_TILE, 10.0f, (i + 1 - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices[2][i * 4 + 2].v3VerPos = D3DXVECTOR3((-NUM_OF_ROW / 2.0f + 1) * LENGTH_OF_TILE, 0.0f, (i + 1 - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices[2][i * 4 + 3].v3VerPos = D3DXVECTOR3((-NUM_OF_ROW / 2.0f + 1) * LENGTH_OF_TILE, 0.0f, (i - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);

			WallVertices[2][i * 4].v2VerTex = D3DXVECTOR2(0.0f, 0.0f);
			WallVertices[2][i * 4 + 1].v2VerTex = D3DXVECTOR2(1.0f, 0.0f);
			WallVertices[2][i * 4 + 2].v2VerTex = D3DXVECTOR2(1.0f, 1.0f);
			WallVertices[2][i * 4 + 3].v2VerTex = D3DXVECTOR2(0.0f, 1.0f);
		}
		// 오른쪽 면
		for (i = 0; i < NUM_OF_ROW; i++)
		{
			for (j = 0; j < 4; j++)
				WallVertices[3][i * 4 + j].v3VerNormal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);

			WallVertices[3][i * 4].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - 1) * LENGTH_OF_TILE, 10.0f, (NUM_OF_ROW / 2.0f - i) * LENGTH_OF_TILE);
			WallVertices[3][i * 4 + 1].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - 1) * LENGTH_OF_TILE, 10.0f, (NUM_OF_ROW / 2.0f - i - 1) * LENGTH_OF_TILE);
			WallVertices[3][i * 4 + 2].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - 1) * LENGTH_OF_TILE, 0.0f, (NUM_OF_ROW / 2.0f - i - 1) * LENGTH_OF_TILE);
			WallVertices[3][i * 4 + 3].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - 1) * LENGTH_OF_TILE, 0.0f, (NUM_OF_ROW / 2.0f - i) * LENGTH_OF_TILE);

			WallVertices[3][i * 4].v2VerTex = D3DXVECTOR2(0.0f, 0.0f);
			WallVertices[3][i * 4 + 1].v2VerTex = D3DXVECTOR2(1.0f, 0.0f);
			WallVertices[3][i * 4 + 2].v2VerTex = D3DXVECTOR2(1.0f, 1.0f);
			WallVertices[3][i * 4 + 3].v2VerTex = D3DXVECTOR2(0.0f, 1.0f);
		}
		// Create wall vertex buffer
		g_pd3dDevice->CreateVertexBuffer(sizeof(WallVertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pWallVB, NULL);
		VOID* pWallVertices;
		g_pWallVB->Lock(0, sizeof(WallVertices), (void**)&pWallVertices, 0);
		memcpy(pWallVertices, WallVertices, sizeof(WallVertices));
		g_pWallVB->Unlock();
	}

	//// wall vertex 2 좌표 입력 - 외곽 벽 뚜껑
	{
		// 위쪽 면
		for (i = 0; i < NUM_OF_ROW; i++)
		{
			for (j = 0; j < 4; j++)
				WallVertices[0][i * 4 + j].v3VerNormal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

			WallVertices2[0][i * 4].v3VerPos = D3DXVECTOR3((i - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE, 10.0f, (NUM_OF_ROW / 2.0f + 1) * LENGTH_OF_TILE);
			WallVertices2[0][i * 4 + 1].v3VerPos = D3DXVECTOR3((i + 1 - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE, 10.0f, (NUM_OF_ROW / 2.0f + 1) * LENGTH_OF_TILE);
			WallVertices2[0][i * 4 + 2].v3VerPos = D3DXVECTOR3((i + 1 - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE, 10.0f, (NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices2[0][i * 4 + 3].v3VerPos = D3DXVECTOR3((i - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE, 10.0f, (NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);

			WallVertices2[0][i * 4].v2VerTex = D3DXVECTOR2(0.0f, 0.0f);
			WallVertices2[0][i * 4 + 1].v2VerTex = D3DXVECTOR2(1.0f, 0.0f);
			WallVertices2[0][i * 4 + 2].v2VerTex = D3DXVECTOR2(1.0f, 1.0f);
			WallVertices2[0][i * 4 + 3].v2VerTex = D3DXVECTOR2(0.0f, 1.0f);
		}
		// 아래쪽 면
		for (i = 0; i < NUM_OF_ROW; i++)
		{
			for (j = 0; j < 4; j++)
				WallVertices[1][i * 4 + j].v3VerNormal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

			WallVertices2[1][i * 4].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - i) * LENGTH_OF_TILE, 10.0f, (-NUM_OF_ROW / 2.0f - 1) * LENGTH_OF_TILE);
			WallVertices2[1][i * 4 + 1].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - i - 1) * LENGTH_OF_TILE, 10.0f, (-NUM_OF_ROW / 2.0f - 1) * LENGTH_OF_TILE);
			WallVertices2[1][i * 4 + 2].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - i - 1) * LENGTH_OF_TILE, 10.0f, (-NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices2[1][i * 4 + 3].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - i) * LENGTH_OF_TILE, 10.0f, (-NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);

			WallVertices2[1][i * 4].v2VerTex = D3DXVECTOR2(0.0f, 0.0f);
			WallVertices2[1][i * 4 + 1].v2VerTex = D3DXVECTOR2(1.0f, 0.0f);
			WallVertices2[1][i * 4 + 2].v2VerTex = D3DXVECTOR2(1.0f, 1.0f);
			WallVertices2[1][i * 4 + 3].v2VerTex = D3DXVECTOR2(0.0f, 1.0f);
		}
		// 왼쪽 면
		for (i = 0; i < NUM_OF_ROW; i++)
		{
			for (j = 0; j < 4; j++)
				WallVertices[2][i * 4 + j].v3VerNormal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

			WallVertices2[2][i * 4].v3VerPos = D3DXVECTOR3((-NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE, 10.0f, (i - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices2[2][i * 4 + 1].v3VerPos = D3DXVECTOR3((-NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE, 10.0f, (i + 1 - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices2[2][i * 4 + 2].v3VerPos = D3DXVECTOR3((-NUM_OF_ROW / 2.0f + 1) * LENGTH_OF_TILE, 10.0f, (i + 1 - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);
			WallVertices2[2][i * 4 + 3].v3VerPos = D3DXVECTOR3((-NUM_OF_ROW / 2.0f + 1) * LENGTH_OF_TILE, 10.0f, (i - NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE);

			WallVertices2[2][i * 4].v2VerTex = D3DXVECTOR2(0.0f, 0.0f);
			WallVertices2[2][i * 4 + 1].v2VerTex = D3DXVECTOR2(1.0f, 0.0f);
			WallVertices2[2][i * 4 + 2].v2VerTex = D3DXVECTOR2(1.0f, 1.0f);
			WallVertices2[2][i * 4 + 3].v2VerTex = D3DXVECTOR2(0.0f, 1.0f);
		}
		// 오른쪽 면
		for (i = 0; i < NUM_OF_ROW; i++)
		{
			for (j = 0; j < 4; j++)
				WallVertices[3][i * 4 + j].v3VerNormal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

			WallVertices2[3][i * 4].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE, 10.0f, (NUM_OF_ROW / 2.0f - i) * LENGTH_OF_TILE);
			WallVertices2[3][i * 4 + 1].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f) * LENGTH_OF_TILE, 10.0f, (NUM_OF_ROW / 2.0f - i - 1) * LENGTH_OF_TILE);
			WallVertices2[3][i * 4 + 2].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - 1) * LENGTH_OF_TILE, 10.0f, (NUM_OF_ROW / 2.0f - i - 1) * LENGTH_OF_TILE);
			WallVertices2[3][i * 4 + 3].v3VerPos = D3DXVECTOR3((NUM_OF_ROW / 2.0f - 1) * LENGTH_OF_TILE, 10.0f, (NUM_OF_ROW / 2.0f - i) * LENGTH_OF_TILE);

			WallVertices2[3][i * 4].v2VerTex = D3DXVECTOR2(0.0f, 0.0f);
			WallVertices2[3][i * 4 + 1].v2VerTex = D3DXVECTOR2(1.0f, 0.0f);
			WallVertices2[3][i * 4 + 2].v2VerTex = D3DXVECTOR2(1.0f, 1.0f);
			WallVertices2[3][i * 4 + 3].v2VerTex = D3DXVECTOR2(0.0f, 1.0f);
		}
		// Create wall vertex buffer
		g_pd3dDevice->CreateVertexBuffer(sizeof(WallVertices2), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pWallVB2, NULL);
		VOID* pWallVertices2;
		g_pWallVB2->Lock(0, sizeof(WallVertices2), (void**)&pWallVertices2, 0);
		memcpy(pWallVertices2, WallVertices2, sizeof(WallVertices2));
		g_pWallVB2->Unlock();
	}
}

VOID CleanUp()
{
	delete g_pFrustum;
	delete g_pCurrentMouse;
	delete g_pMouse;
	delete g_pMidPoint;
	if (g_pSkyboxCube != NULL)
		g_pSkyboxCube->Release();
	if (g_pBulletSphere != NULL)
		g_pBulletSphere->Release();
	if (g_pPlayerSphere != NULL)
		g_pPlayerSphere->Release();
	if (g_pTestFont != NULL)
		g_pTestFont->Release();
	if (g_pFrameFont != NULL)
		g_pFrameFont->Release();
	if (g_pExitFont != NULL)
		g_pExitFont->Release();
	if (g_pSettingFont != NULL)
		g_pSettingFont->Release();
	if (g_pClearFont != NULL)
		g_pClearFont->Release();
	if (g_pFace2 != NULL)
		g_pFace2->Release();
	if (g_pSkyboxTexture != NULL)
		g_pSkyboxTexture->Release();
	if (g_pExitTexture != NULL)
		g_pExitTexture->Release();
	if (g_pNoticeTexture != NULL)
		g_pNoticeTexture->Release();
	if (g_pGrassTexture != NULL)
		g_pGrassTexture->Release();
	if (g_pWallTexture != NULL)
		g_pWallTexture->Release();
	if (g_pTileTexture != NULL)
		g_pTileTexture->Release();

	Exit.ReleaseNoticeVB();
	for (int i = 0; i < notice[0].GetNumOfNotice(); i++)
	{
		notice[i].ReleaseNoticeVB();
	}
	if (g_pMazeVB != NULL)
		g_pMazeVB->Release();
	if (g_pWallVB2 != NULL)
		g_pWallVB2->Release();
	if (g_pWallVB != NULL)
		g_pWallVB->Release();
	if (g_pTileIB != NULL)
		g_pTileIB->Release();
	if (g_pTileVB != NULL)
		g_pTileVB->Release();
	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();
	if (g_pD3D != NULL)
		g_pD3D->Release();
}

VOID __KeyProc()
{
	int i;

	UpdateInput();

	if (!bIsPaused && bIsPlaying)
	{
		// 총알 움직임 계산
		player.MoveBullet();
		// 호랑이 움직임 계산
		X_Tiger.Move(chMap1);
	}

	// wasd 또는 방향키 : 플레이어 앞뒤좌우 움직임

	// 이동 시, 벽과 일정 거리(PLAYER_RADIUS) 이하로는 가까워지지 않도록 조정하기
	// 현재 위치를 단순화하여 좌표로 나타내고, 그 점을 둘러싼 8개의 좌표에 대해 충돌 검사 시행
	// 만약 현재 위치가 블록 사이 선에 걸쳐있다고 하더라도, 문제는 없을 것으로 예상.

	if (GetAsyncKeyState('A') || GetAsyncKeyState(VK_LEFT))
	{
		bIsMoved = player.Move(MOVE_DIRECTION::left, chMap1, bIsNoClipOn);
	}
	if (GetAsyncKeyState('D') || GetAsyncKeyState(VK_RIGHT))
	{
		bIsMoved = player.Move(MOVE_DIRECTION::right, chMap1, bIsNoClipOn);
	}
	if (GetAsyncKeyState('W') || GetAsyncKeyState(VK_UP))
	{
		bIsMoved = player.Move(MOVE_DIRECTION::front, chMap1, bIsNoClipOn);
	}
	if (GetAsyncKeyState('S') || GetAsyncKeyState(VK_DOWN))
	{
		bIsMoved = player.Move(MOVE_DIRECTION::back, chMap1, bIsNoClipOn);
	}
	// Notice & Exit rotation
	if (bIsMoved)
	{
		for (i = 0; i < notice[0].GetNumOfNotice(); i++)
		{
			notice[i].RotateNotice(player.GetPosition());
		}
		Exit.RotateNotice(player.GetPosition());
	}

	for (i = 0; i < notice[0].GetNumOfNotice(); i++)
	{
		if (notice[i].IsPossibleInteraction(player.GetPosition()) == TRUE)
		{
			bIsInteractive = TRUE;
			bIsSkyView = TRUE;
			break;
		}
		else
		{
			bIsInteractive = FALSE;
			//bIsSkyView = FALSE;
		}
	}
	bIsPlaying = Exit.IsPossibleInteraction(player.GetPosition()) ? FALSE : TRUE;

	// Q/E : 플레이어 CCW/CW 회전
	if (GetAsyncKeyState('Q'))
	{
		player.Rotate(TRUE);
	}
	if (GetAsyncKeyState('E'))
	{
		player.Rotate(FALSE);
	}
	//// 추가 기능
	{
		// 점프?는 일단 제외
		if (GetAsyncKeyState(VK_SPACE))
		{

		}
		// light option on/off
		if (GetKeyDown('1') == TRUE)
		{
			if (bIsLightOn == TRUE)
			{
				g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
				bIsLightOn = FALSE;
			}
			else
			{
				g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
				bIsLightOn = TRUE;
			}
		}
		// camera TopView on/off
		if (GetKeyDown('2') == TRUE)
		{
			if (bIsSkyView == FALSE)
				bIsSkyView = TRUE;
			else
				bIsSkyView = FALSE;
		}
		// player flashlight on/off
		if (GetKeyDown('3') == TRUE)
		{
			if (player.IsFlashlightOn() == TRUE)
			{
				player.SetFlashlight(FALSE);
			}
			else
			{
				player.SetFlashlight(TRUE);
			}
		}
		// NoClip(FreeFly) on/off
		if (GetKeyDown('4') == TRUE)
		{
			if (bIsNoClipOn == TRUE)
			{
				bIsNoClipOn = FALSE;
				// 자유시점 종료 시, 저장해뒀던 player 정보 복구
				player.SetPlayerWorld(mtSavedWorld);
				player.SetLookAt(v3SavedLookAt);
			}
			else
			{
				bIsNoClipOn = TRUE;
				mtSavedWorld = player.GetPlayerWorld();
				v3SavedLookAt = player.GetLookAt();
			}
		}
		// preference UI on/off
		if (GetKeyDown(VK_ESCAPE) == TRUE)
		{
			if (bIsPaused == TRUE)
			{
				bIsPaused = FALSE;
			}
			else
			{
				bIsPaused = TRUE;
			}
		}
	}

}

VOID Render()
{
	if (NULL == g_pd3dDevice)
		return;

	if (bIsLightOn == TRUE)
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
	else
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(23, 23, 23), 1.0f, 0);
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);

	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		int i, j;
		D3DLIGHT9* p_light = player.GetPlayerLight();
		// bIsSkyView == TRUE 이면 player의 spot light,
		// FALSE 이면 하늘 시점에서 point light로 바꿔서 맵 전체가 어느 정도 보이게 하는 것도 좋을듯
		g_pd3dDevice->SetLight(0, p_light);
		if (player.IsFlashlightOn() == TRUE)
		{
			g_pd3dDevice->LightEnable(0, TRUE);
		}
		else
		{
			g_pd3dDevice->LightEnable(0, FALSE);
		}
		// 하늘에서 플레이어를 향해 비추는 빛
		ZeroMemory(&skyLight, sizeof(D3DLIGHT9));
		skyLight.Type = D3DLIGHT_SPOT;
		skyLight.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		skyLight.Direction = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		skyLight.Position = player.GetPosition() + D3DXVECTOR3(0.0f, 10.0f, 0.0f); // 플레이어 머리 위에서 비추는 빛
		skyLight.Range = 300.0f;
		skyLight.Attenuation0 = 1.0f;
		skyLight.Falloff = 1.0f;
		skyLight.Phi = D3DXToRadian(90.0f);
		skyLight.Theta = D3DXToRadian(30.0f);
		g_pd3dDevice->SetLight(1, &skyLight);
		g_pd3dDevice->LightEnable(1, TRUE);

		D3DXMATRIX mtWorld;
		D3DXMatrixIdentity(&mtWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &mtWorld);

		D3DXMATRIX mtView;
		D3DXVECTOR3 v3CurrentPosition = player.GetPosition();
		D3DXVECTOR3 v3CurrentLookAt = player.GetLookAt();
		D3DXMATRIX tmpPlayerWorld = player.GetPlayerWorld();
		// frustum culling 시 타일 중심 좌표 표시용
		D3DXVECTOR3 v3TileCenter;

		// 1인칭 시점
		if (bIsSkyView == FALSE)
		{
			D3DXMatrixLookAtLH(&mtView, &v3CurrentPosition, &v3CurrentLookAt, &v3Up);
			g_pd3dDevice->SetTransform(D3DTS_VIEW, &mtView);
		}
		// 탑뷰 시점
		else
		{
			D3DXMatrixLookAtLH(&mtView, &v3EyeCeiling, &v3DefaultPosition, &v3UpCeiling);
			g_pd3dDevice->SetTransform(D3DTS_VIEW, &mtView);
		}

		D3DXMATRIX mtProjection;
		D3DXMatrixPerspectiveFovLH(&mtProjection, D3DX_PI / 4, 1.0f, 0.1f, 1000.0f);
		g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mtProjection);

		//// *****frustum culling*****

		// frustum plane을 계산할, view matrix와 projection matrix의 곱
		D3DXMATRIX mtViewProjection;

		// 하늘에서 바라볼 때, 오브젝트의 LookAt matrix를 따로 계산해야함
		if (bIsSkyView == TRUE)
		{
			D3DXMATRIX mtViewTMP;
			D3DXMatrixLookAtLH(&mtViewTMP, &v3CurrentPosition, &v3CurrentLookAt, &v3Up);
			D3DXMatrixMultiply(&mtViewProjection, &mtViewTMP, &mtProjection);
		}
		else
		{
			D3DXMatrixMultiply(&mtViewProjection, &mtView, &mtProjection);
		}
		g_pFrustum->MakeFrustum(&mtViewProjection);

		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

		//// skybox rendering
		SkyBox.Render();

		g_pd3dDevice->SetTexture(0, g_pGrassTexture);
		g_pd3dDevice->SetStreamSource(0, g_pTileVB, 0, sizeof(CUSTOMVERTEX));
		g_pd3dDevice->SetIndices(g_pTileIB);
		//// tile rendering
		{
			// frustum plane과 사각형 단위로 비교한다
			// 네 꼭짓점 중 하나라도 inside이면 rendering한다.
			// 그냥 원 검사하는 방식으로 변경
			//// tile culling
			for (i = 0; i < NUM_OF_ROW * NUM_OF_COLUMN; i++)
			{
				v3TileCenter = CalculateMidPoint(TileVertices[i * 4].v3VerPos, TileVertices[i * 4 + 2].v3VerPos);
				if (g_pFrustum->IsInFrustum(&v3TileCenter, LENGTH_OF_TILE / 2 * SQRT2) == TRUE)
				{
					g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * 4, 2);
				}
			}

			//// wall rendering
			g_pd3dDevice->SetTexture(0, g_pWallTexture);
			g_pd3dDevice->SetStreamSource(0, g_pWallVB, 0, sizeof(CUSTOMVERTEX));
			//// wall_outside culling
			for (i = 0; i < NUM_OF_ROW * 4; i++)
			{
				v3TileCenter = CalculateMidPoint(WallVertices[i / NUM_OF_ROW][(i * 4) % (NUM_OF_ROW * 4)].v3VerPos, WallVertices[i / NUM_OF_ROW][(i * 4) % (NUM_OF_ROW * 4) + 2].v3VerPos);
				if (g_pFrustum->IsInFrustum(&v3TileCenter, LENGTH_OF_TILE / 2 * SQRT2) == TRUE)
				{
					g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * 4, 2);
				}
			}
			//// wall_outside_upper culling
			g_pd3dDevice->SetStreamSource(0, g_pWallVB2, 0, sizeof(CUSTOMVERTEX));
			for (i = 0; i < NUM_OF_ROW * 4; i++)
			{
				v3TileCenter = CalculateMidPoint(WallVertices2[i / NUM_OF_ROW][(i * 4) % (NUM_OF_ROW * 4)].v3VerPos, WallVertices2[i / NUM_OF_ROW][(i * 4) % (NUM_OF_ROW * 4) + 2].v3VerPos);
				if (g_pFrustum->IsInFrustum(&v3TileCenter, LENGTH_OF_TILE / 2 * SQRT2) == TRUE)
				{
					g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * 4, 2);
				}
			}
			//// wall_inside culling
			g_pd3dDevice->SetStreamSource(0, g_pMazeVB, 0, sizeof(CUSTOMVERTEX));
			for (i = 0; i < 72; i++)
			{
				for (j = 0; j < 5; j++)
				{
					v3TileCenter = CalculateMidPoint(MazeWallVertices[i][j * 4].v3VerPos, MazeWallVertices[i][j * 4 + 2].v3VerPos);
					if (g_pFrustum->IsInFrustum(&v3TileCenter, LENGTH_OF_TILE / 2 * SQRT2) == TRUE)
					{
						g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, i * 20 + j * 4, 2);
					}
				}
			}
		}

		//// notice rendering
		g_pd3dDevice->SetTexture(0, g_pNoticeTexture);
		D3DXMATRIX mtNoticeWorld;
		for (i = 0; i < notice[0].GetNumOfNotice(); i++)
		{
			mtNoticeWorld = notice[i].GetNoticeWorld();
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &mtNoticeWorld);
			notice[i].DrawNotice(g_pd3dDevice);
		}

		//// Exit rendering
		g_pd3dDevice->SetTexture(0, g_pExitTexture);
		mtNoticeWorld = Exit.GetNoticeWorld();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &mtNoticeWorld);
		Exit.DrawNotice(g_pd3dDevice);

		//// Bullet rendering
		g_pd3dDevice->SetTexture(0, g_pTileTexture);
		player.DrawBullet(g_pd3dDevice, g_pBulletSphere);

		// Player 위치 표시용 구체
		if (bIsSkyView == TRUE)
		{
			D3DXMATRIX tmpMatrix;
			D3DXMatrixTranslation(&tmpMatrix, v3CurrentPosition.x, v3CurrentPosition.y, v3CurrentPosition.z);
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &tmpMatrix);
			g_pd3dDevice->SetTexture(0, g_pTileTexture);
			g_pPlayerSphere->DrawSubset(0);
		}
		// Tiger rendering
		D3DXMATRIX X_World = X_Tiger.GetTigerWorld();

		g_pd3dDevice->SetTransform(D3DTS_WORLD, &X_World);
		X_Tiger.XFileDisplay(g_pd3dDevice);

		//// UI rendering ////
		g_pd3dDevice->SetTexture(0, NULL);
		g_pd3dDevice->SetFVF(D3DFVF_UI_VERTEX);
		// 탈출구 UI
		if (!bIsPlaying)
		{
			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, PopUpVertices, sizeof(UI_VERTEX));
			wsprintf(testSTR, "C L E A R");
			SetRect(&rt, 250, 200, 0, 0);
			g_pClearFont->DrawTextA(NULL, testSTR, -1, &rt, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

			Exit.DrawExitButton(g_pd3dDevice);
			wsprintf(testSTR, "e x i t");
			SetRect(&rt, 320, 460, 0, 0);
			g_pExitFont->DrawTextA(NULL, testSTR, -1, &rt, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
		}

		//// 좌상단 UI
		if (bIsSkyView == FALSE)
		{
			//// Transformed Vertex
			g_pd3dDevice->SetTexture(0, NULL);
			g_pd3dDevice->SetFVF(D3DFVF_UI_VERTEX);
			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, UIVertices, sizeof(UI_VERTEX));

			//// Menu
			SetRect(&rt, 20, 20, 0, 0);
			wsprintf(testSTR, " 1: 낮밤 전환\n 2: 탑뷰 on/off\n 3: 손전등 on/off\n 4: 자유시점 on/off\n esc: 일시 정지");
			g_pTestFont->DrawTextA(NULL, testSTR, -1, &rt, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
		}

		// 환경설정 및 일시정지 UI
		if (bIsPaused)
		{
			setting.DrawSetting(g_pd3dDevice);
			wsprintf(testSTR, "P A U S E");
			SetRect(&rt, 280, 200, 0, 0);
			g_pSettingFont->DrawTextA(NULL, testSTR, -1, &rt, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

			Exit.DrawExitButton(g_pd3dDevice);
			wsprintf(testSTR, "e x i t");
			SetRect(&rt, 320, 460, 0, 0);
			g_pExitFont->DrawTextA(NULL, testSTR, -1, &rt, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
		}

		// 자유시점 표시
		if (bIsNoClipOn)
		{
			wsprintf(testSTR, "자유시점 ON");
			// 텍스트 width 얻는 법: drawtext 시 DT_CALCRECT 설정하면 rect에 맞게 텍스트 크기만 계산
			// 그걸 활용해서 조절된 rect에서 값을 가져와 width 구하기
			SetRect(&rt, 0, 0, 0, 0);
			g_pFrameFont->DrawTextA(NULL, testSTR, -1, &rt, DT_CALCRECT, D3DCOLOR_ARGB(0, 0, 0, 0));
			int width = rt.right - rt.left;
			SetRect(&rt, WINDOW_WIDTH / 2 - width / 2, 0, 0, 0);
			g_pFrameFont->DrawTextA(NULL, testSTR, -1, &rt, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 0));
		}

		// FPS 표시
		SetRect(&rt, WINDOW_WIDTH - 110, 0, 0, 0);
		wsprintf(testSTR, "FPS: %3d", FPS.GetFps());
		g_pFrameFont->DrawTextA(NULL, testSTR, -1, &rt, DT_NOCLIP, D3DCOLOR_XRGB(0, 255, 0));

		g_pd3dDevice->EndScene();
	}
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		g_pMidPoint->x = WINDOW_WIDTH / 2;
		g_pMidPoint->y = WINDOW_HEIGHT / 2;

		ClientToScreen(hWnd, g_pMidPoint);
		SetCursorPos(g_pMidPoint->x, g_pMidPoint->y);
		bIsCursorOn = ShowCursor(FALSE);
		break;

	case WM_LBUTTONDOWN:
		g_pMouse->x = LOWORD(lParam);
		g_pMouse->y = HIWORD(lParam);
		bIsClicked = TRUE;
		if (!bIsPlaying || bIsPaused)
		{
			if (PtInRect(&rtExitButton, *g_pMouse))
			{
				Exit.ButtonPressed();
			}
		}
		break;

	case WM_MOUSEMOVE:
		GetCursorPos(g_pCurrentMouse);
		if (!bIsSkyView && bIsPlaying && !bIsPaused)
		{
			DWORD currentTime = timeGetTime();
			if (currentTime - dwRotationTime >= 10)
			{
				if (g_pCurrentMouse->x > g_pMidPoint->x)
				{
					player.Rotate(FALSE, FALSE, (g_pCurrentMouse->x - g_pMidPoint->x) * ROTATION_LEFT_RIGHT);
				}
				else if (g_pCurrentMouse->x < g_pMidPoint->x)
				{
					player.Rotate(TRUE, FALSE, (g_pMidPoint->x - g_pCurrentMouse->x) * ROTATION_LEFT_RIGHT);
				}
				// y좌표는 아래로 갈수록 커지므로, 이게 아래 회전
				if (g_pCurrentMouse->y > g_pMidPoint->y)
				{
					player.Rotate(TRUE, TRUE, (g_pCurrentMouse->y - g_pMidPoint->y) * ROTATION_UP_DOWN);
				}
				else if (g_pCurrentMouse->y < g_pMidPoint->y)
				{
					player.Rotate(FALSE, TRUE, (g_pMidPoint->y - g_pCurrentMouse->y) * ROTATION_UP_DOWN);
				}
			}

		}
		if (!bIsPlaying || bIsPaused)
		{
			if (PtInRect(&rtExitButton, *g_pMouse) && bIsClicked)
				Exit.ButtonPressed();
		}
		else
			Exit.ButtonUnpressed();
		// 게임 중엔 화면 정중앙으로 다시 세팅
		if (bIsPlaying && !bIsPaused)
			SetCursorPos(g_pMidPoint->x, g_pMidPoint->y);
		break;

	case WM_LBUTTONUP:
		g_pMouse->x = LOWORD(lParam);
		g_pMouse->y = HIWORD(lParam);
		bIsClicked = FALSE;
		Exit.ButtonUnpressed();
		player.Attack(g_pMouse);
		//OutputDebugString("Clicked\n");
		if (!bIsPlaying || bIsPaused)
		{
			if (PtInRect(&rtExitButton, *g_pMouse))
			{
				Sleep(100);
				exit(0);
			}
		}
		break;

	case WM_DESTROY:
		CleanUp();
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
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
					  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					  PROGRAM_NAME, NULL };
	RegisterClassEx(&wc);
	// 윈도우 생성
	HWND hWnd = CreateWindow(PROGRAM_NAME, PROGRAM_NAME,
		WS_OVERLAPPEDWINDOW, 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	// Direct3D 초기화
	if (SUCCEEDED(InitD3D(hWnd)))
	{
		{
			InitGeometry();
			// 윈도우 출력
			ShowWindow(hWnd, SW_SHOWDEFAULT);
			UpdateWindow(hWnd);
			// 메시지 루프
			MSG msg;
			ZeroMemory(&msg, sizeof(msg));

			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					if (!bIsPlaying || bIsPaused)
					{
						while (bIsCursorOn < 0)
							bIsCursorOn = ShowCursor(TRUE);
					}

					else
					{
						while (bIsCursorOn >= 0)
							bIsCursorOn = ShowCursor(FALSE);
					}
					__KeyProc();
					Render();
					FPS.Frame();
				}
			}
		}
	}

	// 등록된 클래스 소거
	UnregisterClass(PROGRAM_NAME, wc.hInstance);
	return 0;
}