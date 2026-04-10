#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DungeonRoomType.h"
#include "DungeonConfig.generated.h"

UCLASS(BlueprintType)
class DUNGEONGENERATOR_API UDungeonConfig : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Layout")
    int32 RandomSeed = 12345;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Layout")
    int32 NumRooms = 50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Layout")
    float SpawnRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Rooms")
    FVector2D RoomMinSize = FVector2D(200.f, 200.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Rooms")
    FVector2D RoomMaxSize = FVector2D(600.f, 600.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Rooms")
    float MainRoomSizeMultiplier = 1.25f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Corridors")
    float ExtraLoopChance = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Corridors")
    float CorridorWidth = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|RoomTypes")
    FTreasureRoomConfig TreasureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|RoomTypes")
    FNPCRoomConfig NPCConfig;
};