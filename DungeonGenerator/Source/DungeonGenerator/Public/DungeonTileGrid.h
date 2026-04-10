#pragma once

#include "CoreMinimal.h"

UENUM()
enum class ETileType : uint8
{
    Wall,
    Floor,
    Corridor
};

struct FDungeonTileGrid
{
    int32 Width    = 0;
    int32 Height   = 0;
    float TileSize = 50.f;
    TArray<ETileType> Tiles;

    void Init(int32 InWidth, int32 InHeight, float InTileSize)
    {
        Width    = InWidth;
        Height   = InHeight;
        TileSize = InTileSize;
        // 全部初始化为墙
        Tiles.Init(ETileType::Wall, Width * Height);
    }

    ETileType Get(int32 X, int32 Y) const
    {
        if (X < 0 || X >= Width || Y < 0 || Y >= Height)
            return ETileType::Wall;
        return Tiles[Y * Width + X];
    }

    void Set(int32 X, int32 Y, ETileType Type)
    {
        if (X < 0 || X >= Width || Y < 0 || Y >= Height) return;
        Tiles[Y * Width + X] = Type;
    }

    FVector TileToWorld(int32 X, int32 Y) const
    {
        return FVector(X * TileSize, Y * TileSize, 0.f);
    }
};