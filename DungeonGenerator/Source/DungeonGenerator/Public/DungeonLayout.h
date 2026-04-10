#pragma once

#include "CoreMinimal.h"
#include "DungeonRoomType.h"
#include "DungeonLayout.generated.h"

USTRUCT(BlueprintType)
struct FDungeonRoom
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector2D Position = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FVector2D Size = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    int32 ID = -1;

    UPROPERTY(BlueprintReadOnly)
    ERoomType RoomType = ERoomType::Normal;

    // 房间中心世界坐标（3D）
    UPROPERTY(BlueprintReadOnly)
    FVector WorldCenter = FVector::ZeroVector;

    // 房间面积
    UPROPERTY(BlueprintReadOnly)
    float Area = 0.f;

    FVector2D GetCenter() const { return Position + Size * 0.5f; }
    FBox2D GetBounds() const { return FBox2D(Position, Position + Size); }
};

USTRUCT(BlueprintType)
struct FDungeonCorridor
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 RoomA = -1;

    UPROPERTY(BlueprintReadOnly)
    int32 RoomB = -1;

    UPROPERTY(BlueprintReadOnly)
    FVector2D Start = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FVector2D End = FVector2D::ZeroVector;
};

USTRUCT(BlueprintType)
struct FDungeonLayout
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TArray<FDungeonRoom> Rooms;

    UPROPERTY(BlueprintReadOnly)
    TArray<FDungeonCorridor> Corridors;
};