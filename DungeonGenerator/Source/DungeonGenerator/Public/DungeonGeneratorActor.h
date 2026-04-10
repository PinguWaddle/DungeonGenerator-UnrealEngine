#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonLayout.h"
#include "DungeonRoomType.h"
#include "DungeonGeneratorActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonGenerated, const TArray<FDungeonRoom>&, Rooms);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDungeonCleared);

UCLASS()
class DUNGEONGENERATOR_API ADungeonGeneratorActor : public AActor
{
    GENERATED_BODY()

public:
    ADungeonGeneratorActor();

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
    float MainRoomSizeMultiplier = 1.1f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Corridors")
    float ExtraLoopChance = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Corridors")
    float CorridorWidth = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|RoomTypes")
    FTreasureRoomConfig TreasureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|RoomTypes")
    FNPCRoomConfig NPCConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Meshes")
    TObjectPtr<UStaticMesh> FloorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Meshes")
    TObjectPtr<UStaticMesh> WallMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Spawning")
    TSubclassOf<AActor> PlayerStartClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Spawning")
    TSubclassOf<AActor> BossSpawnClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Spawning")
    TSubclassOf<AActor> NPCSpawnClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Spawning")
    TSubclassOf<AActor> TreasureSpawnClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Debug")
    bool bShowRoomTypeDebug = true;

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Dungeon")
    void GenerateDungeon();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Dungeon")
    void ClearDungeon();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Dungeon")
    void RandomizeSeed();

    // 查询接口
    UFUNCTION(BlueprintCallable, Category = "Dungeon|Query")
    FDungeonRoom GetStartRoom() const;

    UFUNCTION(BlueprintCallable, Category = "Dungeon|Query")
    FDungeonRoom GetBossRoom() const;

    UFUNCTION(BlueprintCallable, Category = "Dungeon|Query")
    TArray<FDungeonRoom> GetRoomsByType(ERoomType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Dungeon|Query")
    TArray<FDungeonRoom> GetAllRooms() const;

    UFUNCTION(BlueprintCallable, Category = "Dungeon|Query")
    bool GetRoomByID(int32 RoomID, FDungeonRoom& OutRoom) const;

    // 事件委托
    UPROPERTY(BlueprintAssignable, Category = "Dungeon|Events")
    FOnDungeonGenerated OnDungeonGenerated;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon|Events")
    FOnDungeonCleared OnDungeonCleared;

private:
    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> FloorISM;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> WallISM;

    UPROPERTY()
    TArray<FDungeonRoom> CachedRooms;

    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedActors;

    void SpawnRoomActors(const TArray<FDungeonRoom>& Rooms);
    void ClearSpawnedActors();
};