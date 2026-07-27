#include "MazeGenerator.h"

D3DXVECTOR3 CalculateMazeCellCenter(const MazeDefinition& maze, int row, int column)
{
    return D3DXVECTOR3(
        (-maze.GetWidth() / 2.0f + column + 0.5f) * kTileSize,
        kTileSize / 2,
        (maze.GetHeight() / 2.0f - row - 0.5f) * kTileSize);
}

D3DXVECTOR3 CalculateMidPoint(D3DXVECTOR3 firstPoint, D3DXVECTOR3 secondPoint)
{
    return D3DXVECTOR3((firstPoint + secondPoint).x / 2, (firstPoint + secondPoint).y / 2, (firstPoint + secondPoint).z / 2);
}

D3DXVECTOR3 CalculateAabbHalfExtents(D3DXVECTOR3 firstPoint, D3DXVECTOR3 secondPoint)
{
    return D3DXVECTOR3(
        fabsf(secondPoint.x - firstPoint.x) / 2.0f,
        fabsf(secondPoint.y - firstPoint.y) / 2.0f,
        fabsf(secondPoint.z - firstPoint.z) / 2.0f);
}

std::vector<MazeWallBlockVertices> GenerateMazeWalls(const MazeDefinition& maze)
{
    std::vector<MazeWallBlockVertices> mazeWalls;

    mazeWalls.reserve(static_cast<std::size_t>(maze.GetWidth()) * maze.GetHeight());

    for (int row = 0; row < maze.GetHeight(); row++)
    {
        for (int column = 0; column < maze.GetWidth(); column++)
        {
            if (maze.GetCell(row, column) != '*')
            {
                continue;
            }

            const D3DXVECTOR3 cellCenter = CalculateMazeCellCenter(maze, row, column);

            mazeWalls.emplace_back();
            GenerateWallBlock(mazeWalls.back().data(), cellCenter);
        }
    }

    return mazeWalls;
}

VOID InitializeMazeEntities(const MazeDefinition& maze, vector<Notice>* notices, Exit* exit)
{
    for (const MazeCellPosition& noticePosition : maze.notices)
    {
        Notice notice;

        notice.Initialize(
            CalculateMazeCellCenter(
                maze,
                noticePosition.row,
                noticePosition.column));

        notices->push_back(notice);
    }

    exit->Initialize(
        CalculateMazeCellCenter(
            maze,
            maze.exit.row,
            maze.exit.column));
}

VOID GenerateWallBlock(CustomVertex* blockVertices, D3DXVECTOR3 position)
{
    int i;
    // 바닥과 맞닿는 밑면을 제외한 다섯 면을 생성한다.
    for (i = 0; i < kWallBlockVertexCount; i++)
    {
        if (i % kVerticesPerWallFace == 0)
            blockVertices[i].textureCoordinate = D3DXVECTOR2(0.0f, 0.0f);
        else if (i % kVerticesPerWallFace == 1)
            blockVertices[i].textureCoordinate = D3DXVECTOR2(1.0f, 0.0f);
        else if (i % kVerticesPerWallFace == 2)
            blockVertices[i].textureCoordinate = D3DXVECTOR2(1.0f, 1.0f);
        else
            blockVertices[i].textureCoordinate = D3DXVECTOR2(0.0f, 1.0f);

        if (i / kVerticesPerWallFace == 0)
            blockVertices[i].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
        else if (i / kVerticesPerWallFace == 1)
            blockVertices[i].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
        else if (i / kVerticesPerWallFace == 2)
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
