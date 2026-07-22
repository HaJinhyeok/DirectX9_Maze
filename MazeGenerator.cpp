#include "MazeGenerator.h"

D3DXVECTOR3 CalculateMidPoint(D3DXVECTOR3 firstPoint, D3DXVECTOR3 secondPoint)
{
    return D3DXVECTOR3((firstPoint + secondPoint).x / 2, (firstPoint + secondPoint).y / 2, (firstPoint + secondPoint).z / 2);
}

// 몇 번째 맵을 만들건지 주어지면, (맵 외곽을 제외한) 맵 내부를 구성하는 벽들의 vertex 좌표를 생성
// 일단 맵은 하나만...
// 맵 내부 구조를 ' '과 '*'로 이루어진 문자열로 표현하고, 이 문자열을 받으면 내부 vertex를 생성할 수 있게 만들면 베스트일듯...
// 벽멱을 전부 vertex 정보로 저장할지, 아니면 한 칸 한 칸의 블록형태로 만들지 미지수
VOID GenerateMazeWalls(int mapNumber, CustomVertex(*mazeVertices)[20], vector<Notice>* notices, Exit* exit)
{
    int i,j;
    int blockIndex = 0;
    // 현재 맵은 하나 뿐임
    if (mapNumber == 1)
    {
        for (i = 0; i < kMazeRowCount; i++)
        {
            for (j = 0; j < kMazeColumnCount; j++)
            {
                if (kMazeMap[i][j] == '*')
                {
                    GenerateWallBlock(mazeVertices[blockIndex++], D3DXVECTOR3((-kMazeColumnCount / 2 + j + 0.5f) * kTileSize, 5.0f, (kMazeRowCount / 2 - i - 0.5f) * kTileSize));
                }
                else if (kMazeMap[i][j] == '@')
                {
                    Notice notice;
                    notice.Initialize(D3DXVECTOR3((-kMazeColumnCount / 2 + j + 0.5f) * kTileSize, 5.0f, (kMazeRowCount / 2 - i - 0.5f) * kTileSize));
                    notices->push_back(notice);
                }
                // 탈출구는 모든 맵마다 단 하나만 존재
                else if (kMazeMap[i][j] == 'X')
                {
                    exit->Initialize(D3DXVECTOR3((-kMazeColumnCount / 2 + j + 0.5f) * kTileSize, 5.0f, (kMazeRowCount / 2 - i - 0.5f) * kTileSize));
                }
            }
        }
    }
    // 추후 맵 추가 생각
    else
        return;
}

VOID GenerateWallBlock(CustomVertex* blockVertices, D3DXVECTOR3 position)
{
    int i;
    // 밑면을 제외한 5개 면의 vertex만 생성해주자
    // 여기서 겹치는 면 vertex는 없애주는 과정이 필요할듯?
    for (i = 0; i < 20; i++)
    {
        if (i % 4 == 0)
            blockVertices[i].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
        else if (i % 4 == 1)
            blockVertices[i].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
        else if (i % 4 == 2)
            blockVertices[i].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
        else
            blockVertices[i].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);

        if (i / 4 == 0)
            blockVertices[i].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
        else if (i / 4 == 1)
            blockVertices[i].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
        else if (i / 4 == 2)
            blockVertices[i].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
        else
            blockVertices[i].normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
    }
    blockVertices[0].position = D3DXVECTOR3(position.x - kTileSize / 2, position.y + kTileSize / 2, position.z - kTileSize / 2);
    blockVertices[1].position = D3DXVECTOR3(position.x + kTileSize / 2, position.y + kTileSize / 2, position.z - kTileSize / 2);
    blockVertices[2].position = D3DXVECTOR3(position.x + kTileSize / 2, position.y - kTileSize / 2, position.z - kTileSize / 2);
    blockVertices[3].position = D3DXVECTOR3(position.x - kTileSize / 2, position.y - kTileSize / 2, position.z - kTileSize / 2);
    
    blockVertices[4].position = D3DXVECTOR3(position.x + kTileSize / 2, position.y + kTileSize / 2, position.z - kTileSize / 2);
    blockVertices[5].position = D3DXVECTOR3(position.x + kTileSize / 2, position.y + kTileSize / 2, position.z + kTileSize / 2);
    blockVertices[6].position = D3DXVECTOR3(position.x + kTileSize / 2, position.y - kTileSize / 2, position.z + kTileSize / 2);
    blockVertices[7].position = D3DXVECTOR3(position.x + kTileSize / 2, position.y - kTileSize / 2, position.z - kTileSize / 2);
    
    blockVertices[8].position = D3DXVECTOR3(position.x + kTileSize / 2, position.y + kTileSize / 2, position.z + kTileSize / 2);
    blockVertices[9].position = D3DXVECTOR3(position.x - kTileSize / 2, position.y + kTileSize / 2, position.z + kTileSize / 2);
    blockVertices[10].position = D3DXVECTOR3(position.x - kTileSize / 2, position.y - kTileSize / 2, position.z + kTileSize / 2);
    blockVertices[11].position = D3DXVECTOR3(position.x + kTileSize / 2, position.y - kTileSize / 2, position.z + kTileSize / 2);
    
    blockVertices[12].position = D3DXVECTOR3(position.x - kTileSize / 2, position.y + kTileSize / 2, position.z + kTileSize / 2);
    blockVertices[13].position = D3DXVECTOR3(position.x - kTileSize / 2, position.y + kTileSize / 2, position.z - kTileSize / 2);
    blockVertices[14].position = D3DXVECTOR3(position.x - kTileSize / 2, position.y - kTileSize / 2, position.z - kTileSize / 2);
    blockVertices[15].position = D3DXVECTOR3(position.x - kTileSize / 2, position.y - kTileSize / 2, position.z + kTileSize / 2);
    
    blockVertices[16].position = D3DXVECTOR3(position.x - kTileSize / 2, position.y + kTileSize / 2, position.z + kTileSize / 2);
    blockVertices[17].position = D3DXVECTOR3(position.x + kTileSize / 2, position.y + kTileSize / 2, position.z + kTileSize / 2);
    blockVertices[18].position = D3DXVECTOR3(position.x + kTileSize / 2, position.y + kTileSize / 2, position.z - kTileSize / 2);
    blockVertices[19].position = D3DXVECTOR3(position.x - kTileSize / 2, position.y + kTileSize / 2, position.z - kTileSize / 2);
}
