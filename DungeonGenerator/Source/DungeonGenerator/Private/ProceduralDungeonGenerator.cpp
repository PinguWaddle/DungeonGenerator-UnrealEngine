#include "ProceduralDungeonGenerator.h"

FDungeonLayout FProceduralDungeonGenerator::Generate(const UDungeonConfig* Config)
{
    if (!Config) return {};
    Random.Initialize(Config->RandomSeed);

    TArray<FDungeonRoom> AllRooms = GenerateRooms(Config);
    SeparateRooms(AllRooms);
    OffsetRooms(AllRooms);

    TArray<int32> MainIdx = SelectMainRooms(AllRooms, Config->MainRoomSizeMultiplier);
    if (MainIdx.Num() < 2) return {};

    TArray<FDungeonEdge> AllEdges = BuildAllEdges(AllRooms, MainIdx);
    TArray<FDungeonEdge> MST      = BuildMST(AllEdges, MainIdx.Num(), Config->ExtraLoopChance);

    FDungeonLayout Layout;
    for (int32 i = 0; i < MainIdx.Num(); i++)
    {
        FDungeonRoom R = AllRooms[MainIdx[i]];
        R.ID = i;
        R.Area = R.Size.X * R.Size.Y;
        R.WorldCenter = FVector(R.Position.X + R.Size.X * 0.5f,
            R.Position.Y + R.Size.Y * 0.5f,
            0.f);
        Layout.Rooms.Add(R);
    }
    BuildCorridors(AllRooms, MainIdx, MST, Layout.Corridors);

    AssignRoomTypes(Layout.Rooms, Config);
    
    return Layout;
}

TArray<FDungeonRoom> FProceduralDungeonGenerator::GenerateRooms(const UDungeonConfig* Config)
{
    TArray<FDungeonRoom> Rooms;
    for (int32 i = 0; i < Config->NumRooms; i++)
    {
        FDungeonRoom R;
        R.ID = i;
        FVector2D Pos = RandInCircle(Config->SpawnRadius);
        float W = FMath::RoundToFloat(Random.FRandRange(Config->RoomMinSize.X, Config->RoomMaxSize.X) / TileSize) * TileSize;
        float H = FMath::RoundToFloat(Random.FRandRange(Config->RoomMinSize.Y, Config->RoomMaxSize.Y) / TileSize) * TileSize;
        R.Size     = FVector2D(FMath::Max(W, Config->RoomMinSize.X), FMath::Max(H, Config->RoomMinSize.Y));
        R.Position = SnapToGrid(Pos - R.Size * 0.5f);
        Rooms.Add(R);
    }
    return Rooms;
}

FVector2D FProceduralDungeonGenerator::RandInCircle(float Radius)
{
    float r     = Radius * FMath::Sqrt(Random.FRandRange(0.f, 1.f));
    float theta = Random.FRandRange(0.f, 2.f * PI);
    return FVector2D(r * FMath::Cos(theta), r * FMath::Sin(theta));
}

FVector2D FProceduralDungeonGenerator::SnapToGrid(FVector2D V) const
{
    return FVector2D(FMath::RoundToFloat(V.X / TileSize) * TileSize,
                     FMath::RoundToFloat(V.Y / TileSize) * TileSize);
}

bool FProceduralDungeonGenerator::RoomsOverlap(const FDungeonRoom& A, const FDungeonRoom& B) const
{
    float Gap = TileSize;
    return !(A.Position.X + A.Size.X + Gap <= B.Position.X ||
             B.Position.X + B.Size.X + Gap <= A.Position.X ||
             A.Position.Y + A.Size.Y + Gap <= B.Position.Y ||
             B.Position.Y + B.Size.Y + Gap <= A.Position.Y);
}

void FProceduralDungeonGenerator::SeparateRooms(TArray<FDungeonRoom>& Rooms)
{
    for (int32 Iter = 0; Iter < 1000; Iter++)
    {
        bool bMoved = false;
        for (int32 i = 0; i < Rooms.Num(); i++)
        {
            for (int32 j = i + 1; j < Rooms.Num(); j++)
            {
                if (!RoomsOverlap(Rooms[i], Rooms[j])) continue;
                bMoved = true;

                FVector2D Ci = Rooms[i].GetCenter();
                FVector2D Cj = Rooms[j].GetCenter();
                FVector2D Dir = Ci - Cj;
                if (Dir.IsNearlyZero()) Dir = FVector2D(1.f, 0.f);

                float OverlapX = (Rooms[i].Size.X + Rooms[j].Size.X) * 0.5f + TileSize - FMath::Abs(Ci.X - Cj.X);
                float OverlapY = (Rooms[i].Size.Y + Rooms[j].Size.Y) * 0.5f + TileSize - FMath::Abs(Ci.Y - Cj.Y);

                FVector2D Push;
                if (OverlapX < OverlapY)
                    Push = FVector2D(Dir.X >= 0.f ? OverlapX * 0.5f : -OverlapX * 0.5f, 0.f);
                else
                    Push = FVector2D(0.f, Dir.Y >= 0.f ? OverlapY * 0.5f : -OverlapY * 0.5f);

                Rooms[i].Position = SnapToGrid(Rooms[i].Position + Push);
                Rooms[j].Position = SnapToGrid(Rooms[j].Position - Push);
            }
        }
        if (!bMoved) break;
    }
}

void FProceduralDungeonGenerator::OffsetRooms(TArray<FDungeonRoom>& Rooms)
{
    if (Rooms.IsEmpty()) return;
    float MinX = Rooms[0].Position.X, MinY = Rooms[0].Position.Y;
    for (auto& R : Rooms) { MinX = FMath::Min(MinX, R.Position.X); MinY = FMath::Min(MinY, R.Position.Y); }
    FVector2D Off(-MinX + 500.f, -MinY + 500.f);
    for (auto& R : Rooms) R.Position += Off;
}

TArray<int32> FProceduralDungeonGenerator::SelectMainRooms(const TArray<FDungeonRoom>& Rooms, float Threshold)
{
    float TotalArea = 0.f;
    for (auto& R : Rooms) TotalArea += R.Size.X * R.Size.Y;
    float AvgArea = TotalArea / Rooms.Num();

    TArray<int32> Main;
    for (int32 i = 0; i < Rooms.Num(); i++)
        if (Rooms[i].Size.X * Rooms[i].Size.Y >= AvgArea * Threshold)
            Main.Add(i);
    return Main;
}

TArray<FDungeonEdge> FProceduralDungeonGenerator::BuildAllEdges(
    const TArray<FDungeonRoom>& Rooms, const TArray<int32>& Idx)
{
    TArray<FDungeonEdge> Edges;
    int32 N = Idx.Num();
    for (int32 i = 0; i < N; i++)
        for (int32 j = i + 1; j < N; j++)
        {
            FDungeonEdge E;
            E.A = i; E.B = j;
            E.Weight = FVector2D::Distance(Rooms[Idx[i]].GetCenter(), Rooms[Idx[j]].GetCenter());
            Edges.Add(E);
        }
    return Edges;
}

int32 FProceduralDungeonGenerator::Find(TArray<int32>& P, int32 X)
{
    if (P[X] != X) P[X] = Find(P, P[X]);
    return P[X];
}

void FProceduralDungeonGenerator::Unite(TArray<int32>& P, TArray<int32>& R, int32 X, int32 Y)
{
    int32 PX = Find(P, X), PY = Find(P, Y);
    if (PX == PY) return;
    if (R[PX] < R[PY]) Swap(PX, PY);
    P[PY] = PX;
    if (R[PX] == R[PY]) R[PX]++;
}

TArray<FDungeonEdge> FProceduralDungeonGenerator::BuildMST(
    TArray<FDungeonEdge> Edges, int32 N, float ExtraChance)
{
    Edges.Sort();
    TArray<int32> Parent, Rank;
    Parent.SetNum(N); Rank.SetNum(N);
    for (int32 i = 0; i < N; i++) { Parent[i] = i; Rank[i] = 0; }

    TArray<FDungeonEdge> MST, Remaining;
    for (auto& E : Edges)
    {
        if (Find(Parent, E.A) != Find(Parent, E.B))
        { Unite(Parent, Rank, E.A, E.B); MST.Add(E); }
        else
            Remaining.Add(E);
    }

    for (auto& E : Remaining)
        if (Random.FRandRange(0.f, 1.f) < ExtraChance)
            MST.Add(E);

    return MST;
}

void FProceduralDungeonGenerator::BuildCorridors(
    const TArray<FDungeonRoom>& AllRooms, const TArray<int32>& Idx,
    const TArray<FDungeonEdge>& MST, TArray<FDungeonCorridor>& Out)
{
    for (auto& E : MST)
    {
        FVector2D CA = AllRooms[Idx[E.A]].GetCenter();
        FVector2D CB = AllRooms[Idx[E.B]].GetCenter();

        FDungeonCorridor C;
        C.RoomA = E.A;
        C.RoomB = E.B;

        // 随机选择 L 型走廊的方向
        if (Random.FRandRange(0.f, 1.f) > 0.5f)
        { C.Start = CA; C.End = CB; }
        else
        { C.Start = CB; C.End = CA; }

        Out.Add(C);
    }
}

float FProceduralDungeonGenerator::GetDistanceBetweenRooms(const FDungeonRoom& A, const FDungeonRoom& B) const
{
    return FVector2D::Distance(A.GetCenter(), B.GetCenter());
}

void FProceduralDungeonGenerator::AssignRoomTypes(TArray<FDungeonRoom>& Rooms, const UDungeonConfig* Config)
{
    if (Rooms.IsEmpty()) return;

    // 找最大面积的房间 → Boss
    int32 BossIdx = 0;
    float MaxArea = 0.f;
    for (int32 i = 0; i < Rooms.Num(); i++)
    {
        float Area = Rooms[i].Size.X * Rooms[i].Size.Y;
        if (Area > MaxArea) { MaxArea = Area; BossIdx = i; }
    }
    Rooms[BossIdx].RoomType = ERoomType::Boss;

    // 找距离 Boss 最远的房间 → Start
    int32 StartIdx = 0;
    float MaxDist  = 0.f;
    for (int32 i = 0; i < Rooms.Num(); i++)
    {
        if (i == BossIdx) continue;
        float Dist = GetDistanceBetweenRooms(Rooms[i], Rooms[BossIdx]);
        if (Dist > MaxDist) { MaxDist = Dist; StartIdx = i; }
    }
    Rooms[StartIdx].RoomType = ERoomType::Start;

    // 剩余房间的候选列表
    TArray<int32> Candidates;
    for (int32 i = 0; i < Rooms.Num(); i++)
        if (i != BossIdx && i != StartIdx)
            Candidates.Add(i);

    // 打乱候选列表
    for (int32 i = Candidates.Num() - 1; i > 0; i--)
    {
        int32 j = Random.RandRange(0, i);
        Candidates.Swap(i, j);
    }

    int32 CursorIdx = 0;

    // 分配 Epic 宝箱房
    for (int32 i = 0; i < Config->TreasureConfig.EpicCount && CursorIdx < Candidates.Num(); i++, CursorIdx++)
        Rooms[Candidates[CursorIdx]].RoomType = ERoomType::Treasure_Epic;

    // 分配 Rare 宝箱房
    for (int32 i = 0; i < Config->TreasureConfig.RareCount && CursorIdx < Candidates.Num(); i++, CursorIdx++)
        Rooms[Candidates[CursorIdx]].RoomType = ERoomType::Treasure_Rare;

    // 分配 Common 宝箱房
    for (int32 i = 0; i < Config->TreasureConfig.CommonCount && CursorIdx < Candidates.Num(); i++, CursorIdx++)
        Rooms[Candidates[CursorIdx]].RoomType = ERoomType::Treasure_Common;

    // 分配 NPC 房间
    for (int32 i = 0; i < Config->NPCConfig.Count && CursorIdx < Candidates.Num(); i++, CursorIdx++)
        Rooms[Candidates[CursorIdx]].RoomType = ERoomType::NPC;

    // 其余全是 Normal，已经是默认值不需要额外设置
}