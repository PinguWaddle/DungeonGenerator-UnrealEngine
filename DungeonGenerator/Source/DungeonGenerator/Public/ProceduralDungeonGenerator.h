#pragma once

#include "CoreMinimal.h"
#include "DungeonLayout.h"
#include "DungeonConfig.h"

struct FDungeonEdge
{
    int32 A = -1;
    int32 B = -1;
    float Weight = 0.f;
    bool operator<(const FDungeonEdge& O) const { return Weight < O.Weight; }
};

class DUNGEONGENERATOR_API FProceduralDungeonGenerator
{
public:
    FDungeonLayout Generate(const UDungeonConfig* Config);

private:
    FRandomStream Random;
    const float TileSize = 50.f;

    TArray<FDungeonRoom>  GenerateRooms(const UDungeonConfig* Config);
    void                  SeparateRooms(TArray<FDungeonRoom>& Rooms);
    void                  OffsetRooms(TArray<FDungeonRoom>& Rooms);
    TArray<int32>         SelectMainRooms(const TArray<FDungeonRoom>& Rooms, float Threshold);
    TArray<FDungeonEdge>  BuildAllEdges(const TArray<FDungeonRoom>& Rooms, const TArray<int32>& Idx);
    TArray<FDungeonEdge>  BuildMST(TArray<FDungeonEdge> Edges, int32 N, float ExtraChance);
    void                  BuildCorridors(const TArray<FDungeonRoom>& Rooms, const TArray<int32>& Idx,
                                         const TArray<FDungeonEdge>& MST, TArray<FDungeonCorridor>& Out);

    bool      RoomsOverlap(const FDungeonRoom& A, const FDungeonRoom& B) const;
    FVector2D RandInCircle(float Radius);
    FVector2D SnapToGrid(FVector2D V) const;
    int32     Find(TArray<int32>& P, int32 X);
    void      Unite(TArray<int32>& P, TArray<int32>& R, int32 X, int32 Y);

    void AssignRoomTypes(TArray<FDungeonRoom>& Rooms, const UDungeonConfig* Config);
    float GetDistanceBetweenRooms(const FDungeonRoom& A, const FDungeonRoom& B) const;
};