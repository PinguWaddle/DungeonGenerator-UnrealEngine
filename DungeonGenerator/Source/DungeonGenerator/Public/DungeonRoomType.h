#pragma once

#include "CoreMinimal.h"
#include "DungeonRoomType.generated.h"

UENUM(BlueprintType)
enum class ERoomType : uint8
{
    Normal,
    Start,
    Boss,
    Treasure_Common,
    Treasure_Rare,
    Treasure_Epic,
    NPC
};

USTRUCT(BlueprintType)
struct FTreasureRoomConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Treasure")
    int32 CommonCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Treasure")
    int32 RareCount = 2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Treasure")
    int32 EpicCount = 1;
};

USTRUCT(BlueprintType)
struct FNPCRoomConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
    int32 Count = 2;
};