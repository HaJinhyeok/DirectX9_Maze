#include "maze_function.h"

D3DXVECTOR3 CalculateMidPoint(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2)
{
    return D3DXVECTOR3((pos1 + pos2).x / 2, (pos1 + pos2).y / 2, (pos1 + pos2).z / 2);
}

// 몇 번째 맵을 만들건지 주어지면, (맵 외곽을 제외한) 맵 내부를 구성하는 벽들의 vertex 좌표를 생성
// 일단 맵은 하나만...
// 맵 내부 구조를 ' '과 '*'로 이루어진 문자열로 표현하고, 이 문자열을 받으면 내부 vertex를 생성할 수 있게 만들면 베스트일듯...
// 벽멱을 전부 vertex 정보로 저장할지, 아니면 한 칸 한 칸의 블록형태로 만들지 미지수
VOID GenerateMazeWall(int nMapNumber, CUSTOMVERTEX(*Maze)[20], vector<CNotice>* notice, CExit* Exit)
{
    int i,j;
    int nBlockNum = 0, nNoticeNum = 0;
    // 현재 맵은 하나 뿐임
    if (nMapNumber == 1)
    {
        for (i = 0; i < kMazeRowCount; i++)
        {
            for (j = 0; j < kMazeColumnCount; j++)
            {
                if (chMap1[i][j] == '*')
                {
                    MakeWallBlock(Maze[nBlockNum++], D3DXVECTOR3((-kMazeColumnCount / 2 + j + 0.5f) * kTileSize, 5.0f, (kMazeRowCount / 2 - i - 0.5f) * kTileSize));
                }
                else if (chMap1[i][j] == '@')
                {
                    CNotice tmpNotice;
                    tmpNotice.MakeNotice(D3DXVECTOR3((-kMazeColumnCount / 2 + j + 0.5f) * kTileSize, 5.0f, (kMazeRowCount / 2 - i - 0.5f) * kTileSize));
                    notice->push_back(tmpNotice);
                }
                // 탈출구는 모든 맵마다 단 하나만 존재
                else if (chMap1[i][j] == 'X')
                {
                    Exit->MakeExit(D3DXVECTOR3((-kMazeColumnCount / 2 + j + 0.5f) * kTileSize, 5.0f, (kMazeRowCount / 2 - i - 0.5f) * kTileSize));
                }
            }
        }
    }
    // 추후 맵 추가 생각
    else
        return;
}

VOID MakeWallBlock(CUSTOMVERTEX* Block, D3DXVECTOR3 position)
{
    int i;
    // 밑면을 제외한 5개 면의 vertex만 생성해주자
    // 여기서 겹치는 면 vertex는 없애주는 과정이 필요할듯?
    for (i = 0; i < 20; i++)
    {
        if (i % 4 == 0)
            Block[i].v2VerTex = D3DXVECTOR2(0.0f, 0.0f);
        else if (i % 4 == 1)
            Block[i].v2VerTex = D3DXVECTOR2(1.0f, 0.0f);
        else if (i % 4 == 2)
            Block[i].v2VerTex = D3DXVECTOR2(1.0f, 1.0f);
        else
            Block[i].v2VerTex = D3DXVECTOR2(0.0f, 1.0f);

        if (i / 4 == 0)
            Block[i].v3VerNormal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
        else if (i / 4 == 1)
            Block[i].v3VerNormal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
        else if (i / 4 == 2)
            Block[i].v3VerNormal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
        else
            Block[i].v3VerNormal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
    }
    Block[0].v3VerPos = D3DXVECTOR3(position.x - kTileSize / 2, position.y + kTileSize / 2, position.z - kTileSize / 2);
    Block[1].v3VerPos = D3DXVECTOR3(position.x + kTileSize / 2, position.y + kTileSize / 2, position.z - kTileSize / 2);
    Block[2].v3VerPos = D3DXVECTOR3(position.x + kTileSize / 2, position.y - kTileSize / 2, position.z - kTileSize / 2);
    Block[3].v3VerPos = D3DXVECTOR3(position.x - kTileSize / 2, position.y - kTileSize / 2, position.z - kTileSize / 2);
    
    Block[4].v3VerPos = D3DXVECTOR3(position.x + kTileSize / 2, position.y + kTileSize / 2, position.z - kTileSize / 2);
    Block[5].v3VerPos = D3DXVECTOR3(position.x + kTileSize / 2, position.y + kTileSize / 2, position.z + kTileSize / 2);
    Block[6].v3VerPos = D3DXVECTOR3(position.x + kTileSize / 2, position.y - kTileSize / 2, position.z + kTileSize / 2);
    Block[7].v3VerPos = D3DXVECTOR3(position.x + kTileSize / 2, position.y - kTileSize / 2, position.z - kTileSize / 2);
    
    Block[8].v3VerPos = D3DXVECTOR3(position.x + kTileSize / 2, position.y + kTileSize / 2, position.z + kTileSize / 2);
    Block[9].v3VerPos = D3DXVECTOR3(position.x - kTileSize / 2, position.y + kTileSize / 2, position.z + kTileSize / 2);
    Block[10].v3VerPos = D3DXVECTOR3(position.x - kTileSize / 2, position.y - kTileSize / 2, position.z + kTileSize / 2);
    Block[11].v3VerPos = D3DXVECTOR3(position.x + kTileSize / 2, position.y - kTileSize / 2, position.z + kTileSize / 2);
    
    Block[12].v3VerPos = D3DXVECTOR3(position.x - kTileSize / 2, position.y + kTileSize / 2, position.z + kTileSize / 2);
    Block[13].v3VerPos = D3DXVECTOR3(position.x - kTileSize / 2, position.y + kTileSize / 2, position.z - kTileSize / 2);
    Block[14].v3VerPos = D3DXVECTOR3(position.x - kTileSize / 2, position.y - kTileSize / 2, position.z - kTileSize / 2);
    Block[15].v3VerPos = D3DXVECTOR3(position.x - kTileSize / 2, position.y - kTileSize / 2, position.z + kTileSize / 2);
    
    Block[16].v3VerPos = D3DXVECTOR3(position.x - kTileSize / 2, position.y + kTileSize / 2, position.z + kTileSize / 2);
    Block[17].v3VerPos = D3DXVECTOR3(position.x + kTileSize / 2, position.y + kTileSize / 2, position.z + kTileSize / 2);
    Block[18].v3VerPos = D3DXVECTOR3(position.x + kTileSize / 2, position.y + kTileSize / 2, position.z - kTileSize / 2);
    Block[19].v3VerPos = D3DXVECTOR3(position.x - kTileSize / 2, position.y + kTileSize / 2, position.z - kTileSize / 2);
}
