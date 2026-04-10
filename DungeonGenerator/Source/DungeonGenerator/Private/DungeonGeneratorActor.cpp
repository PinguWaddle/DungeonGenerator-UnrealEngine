#include "DungeonGeneratorActor.h"
#include "ProceduralDungeonGenerator.h"
#include "DungeonConfig.h"
#include "DungeonTileGrid.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DrawDebugHelpers.h"

#if WITH_EDITOR
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif

ADungeonGeneratorActor::ADungeonGeneratorActor()
{
    PrimaryActorTick.bCanEverTick = false;
    FloorISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorISM"));
    RootComponent = FloorISM;
    WallISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallISM"));
    WallISM->SetupAttachment(RootComponent);
}

void ADungeonGeneratorActor::GenerateDungeon()
{
    if (NumRooms < 5) { UE_LOG(LogTemp, Warning, TEXT("NumRooms too small.")); return; }
    if (RoomMinSize.X > RoomMaxSize.X || RoomMinSize.Y > RoomMaxSize.Y) { UE_LOG(LogTemp, Warning, TEXT("RoomMinSize > RoomMaxSize.")); return; }
    if (CorridorWidth < 50.f) { UE_LOG(LogTemp, Warning, TEXT("CorridorWidth too small.")); return; }

#if WITH_EDITOR
    TSharedPtr<SNotificationItem> Notification;
    FNotificationInfo Info(FText::FromString(TEXT("Generating Dungeon...")));
    Info.bFireAndForget = false;
    Info.FadeOutDuration = 1.f;
    Notification = FSlateNotificationManager::Get().AddNotification(Info);
    if (Notification.IsValid())
        Notification->SetCompletionState(SNotificationItem::CS_Pending);
#endif

    ClearDungeon();

    UDungeonConfig* Config = NewObject<UDungeonConfig>(this);
    Config->RandomSeed = RandomSeed;
    Config->NumRooms = NumRooms;
    Config->SpawnRadius = SpawnRadius;
    Config->RoomMinSize = RoomMinSize;
    Config->RoomMaxSize = RoomMaxSize;
    Config->MainRoomSizeMultiplier = MainRoomSizeMultiplier;
    Config->ExtraLoopChance = ExtraLoopChance;
    Config->CorridorWidth = CorridorWidth;
    Config->TreasureConfig = TreasureConfig;
    Config->NPCConfig = NPCConfig;

    FDungeonLayout Layout = FProceduralDungeonGenerator().Generate(Config);

    const float TileSize = 50.f;
    const float WallH = 250.f;
    const float BaseZ = -100.f;
    const float FloorZ = 0.f;

    FVector2D MaxPos(0.f, 0.f);
    for (auto& R : Layout.Rooms)
    {
        MaxPos.X = FMath::Max(MaxPos.X, R.Position.X + R.Size.X);
        MaxPos.Y = FMath::Max(MaxPos.Y, R.Position.Y + R.Size.Y);
    }

    int32 GridW = FMath::CeilToInt(MaxPos.X / TileSize) + 4;
    int32 GridH = FMath::CeilToInt(MaxPos.Y / TileSize) + 4;

    FDungeonTileGrid Grid;
    Grid.Init(GridW, GridH, TileSize);

    // 雕刻房间
    for (auto& Room : Layout.Rooms)
    {
        int32 X0 = FMath::RoundToInt(Room.Position.X / TileSize);
        int32 Y0 = FMath::RoundToInt(Room.Position.Y / TileSize);
        int32 X1 = FMath::RoundToInt((Room.Position.X + Room.Size.X) / TileSize);
        int32 Y1 = FMath::RoundToInt((Room.Position.Y + Room.Size.Y) / TileSize);
        for (int32 Y = Y0; Y < Y1; Y++)
            for (int32 X = X0; X < X1; X++)
                Grid.Set(X, Y, ETileType::Floor);
    }

    // 雕刻走廊
    int32 HC = FMath::Max(1, FMath::RoundToInt(CorridorWidth / TileSize)) / 2;

    for (auto& C : Layout.Corridors)
    {
        int32 SX = FMath::RoundToInt(C.Start.X / TileSize);
        int32 SY = FMath::RoundToInt(C.Start.Y / TileSize);
        int32 EX = FMath::RoundToInt(C.End.X / TileSize);
        int32 EY = FMath::RoundToInt(C.End.Y / TileSize);

        int32 HMinX = FMath::Min(SX, EX), HMaxX = FMath::Max(SX, EX);
        for (int32 X = HMinX; X <= HMaxX; X++)
            for (int32 d = -HC; d <= HC; d++)
                Grid.Set(X, SY + d, ETileType::Corridor);

        int32 VMinY = FMath::Min(SY, EY), VMaxY = FMath::Max(SY, EY);
        for (int32 Y = VMinY; Y <= VMaxY; Y++)
            for (int32 d = -HC; d <= HC; d++)
                Grid.Set(EX + d, Y, ETileType::Corridor);
    }

    // 生成 Mesh
    if (FloorMesh) FloorISM->SetStaticMesh(FloorMesh);
    if (WallMesh)  WallISM->SetStaticMesh(WallMesh);

    for (int32 Y = 0; Y < GridH; Y++)
    {
        for (int32 X = 0; X < GridW; X++)
        {
            ETileType Tile = Grid.Get(X, Y);
            FVector   Pos = Grid.TileToWorld(X, Y);

            if (Tile == ETileType::Floor || Tile == ETileType::Corridor)
            {
                if (!FloorMesh) continue;
                FTransform T;
                T.SetLocation(FVector(Pos.X, Pos.Y, (FloorZ + BaseZ) * 0.5f));
                T.SetScale3D(FVector(1.f, 1.f, (FloorZ - BaseZ) / 100.f));
                FloorISM->AddInstance(T);
            }
            else if (Tile == ETileType::Wall)
            {
                bool bAdj = false;
                int32 DX[] = { 1,-1,0,0 };
                int32 DY[] = { 0,0,1,-1 };
                for (int32 i = 0; i < 4; i++)
                {
                    ETileType N = Grid.Get(X + DX[i], Y + DY[i]);
                    if (N == ETileType::Floor || N == ETileType::Corridor) { bAdj = true; break; }
                }
                if (!bAdj || !WallMesh) continue;

                float TopZ = FloorZ + WallH;
                float TotalH = TopZ - BaseZ;
                float CenterZ = (TopZ + BaseZ) * 0.5f;
                FTransform T;
                T.SetLocation(FVector(Pos.X, Pos.Y, CenterZ));
                T.SetScale3D(FVector(1.f, 1.f, TotalH / 100.f));
                WallISM->AddInstance(T);
            }
        }
    }
    CachedRooms = Layout.Rooms;

    UE_LOG(LogTemp, Display, TEXT("DungeonGenerator: %d rooms, %d corridors."),
        Layout.Rooms.Num(), Layout.Corridors.Num());

    OnDungeonGenerated.Broadcast(CachedRooms);
    SpawnRoomActors(CachedRooms);

#if WITH_EDITOR
    if (Notification.IsValid())
    {
        Notification->SetText(FText::FromString(
            FString::Printf(TEXT("Dungeon Generated: %d rooms, %d corridors"),
                Layout.Rooms.Num(), Layout.Corridors.Num())));
        Notification->SetCompletionState(SNotificationItem::CS_Success);
        Notification->ExpireAndFadeout();
    }
#endif

#if WITH_EDITOR
    if (bShowRoomTypeDebug && GetWorld())
    {
        FlushPersistentDebugLines(GetWorld());
        for (const FDungeonRoom& Room : Layout.Rooms)
        {
            FColor Color;
            switch (Room.RoomType)
            {
            case ERoomType::Start:            Color = FColor::Green;  break;
            case ERoomType::Boss:             Color = FColor::Red;    break;
            case ERoomType::Treasure_Common:  Color = FColor::Yellow; break;
            case ERoomType::Treasure_Rare:    Color = FColor::Blue;   break;
            case ERoomType::Treasure_Epic:    Color = FColor::Purple; break;
            case ERoomType::NPC:              Color = FColor::Cyan;   break;
            default:                          Color = FColor::White;  break;
            }

            FVector Center(
                Room.Position.X + Room.Size.X * 0.5f,
                Room.Position.Y + Room.Size.Y * 0.5f,
                200.f
            );
            FVector Extent(Room.Size.X * 0.5f, Room.Size.Y * 0.5f, 10.f);
            DrawDebugBox(GetWorld(), Center, Extent, Color, false, 60.f, 0, 15.f);
        }
    }
#endif
}

void ADungeonGeneratorActor::ClearDungeon()
{
    if (FloorISM) FloorISM->ClearInstances();
    if (WallISM)  WallISM->ClearInstances();

    ClearSpawnedActors();
    CachedRooms.Empty();
    OnDungeonCleared.Broadcast();

#if WITH_EDITOR
    if (GetWorld())
        FlushPersistentDebugLines(GetWorld());
#endif
}

void ADungeonGeneratorActor::RandomizeSeed()
{
    RandomSeed = FMath::RandRange(0, 999999);
    GenerateDungeon();
}

FDungeonRoom ADungeonGeneratorActor::GetStartRoom() const
{
    for (const FDungeonRoom& Room : CachedRooms)
        if (Room.RoomType == ERoomType::Start)
            return Room;
    return FDungeonRoom();
}

FDungeonRoom ADungeonGeneratorActor::GetBossRoom() const
{
    for (const FDungeonRoom& Room : CachedRooms)
        if (Room.RoomType == ERoomType::Boss)
            return Room;
    return FDungeonRoom();
}

TArray<FDungeonRoom> ADungeonGeneratorActor::GetRoomsByType(ERoomType Type) const
{
    TArray<FDungeonRoom> Result;
    for (const FDungeonRoom& Room : CachedRooms)
        if (Room.RoomType == Type)
            Result.Add(Room);
    return Result;
}

TArray<FDungeonRoom> ADungeonGeneratorActor::GetAllRooms() const
{
    return CachedRooms;
}

bool ADungeonGeneratorActor::GetRoomByID(int32 RoomID, FDungeonRoom& OutRoom) const
{
    for (const FDungeonRoom& Room : CachedRooms)
    {
        if (Room.ID == RoomID)
        {
            OutRoom = Room;
            return true;
        }
    }
    return false;
}

void ADungeonGeneratorActor::SpawnRoomActors(const TArray<FDungeonRoom>& Rooms)
{
    if (!GetWorld()) return;

    for (const FDungeonRoom& Room : Rooms)
    {
        TSubclassOf<AActor> ClassToSpawn = nullptr;

        switch (Room.RoomType)
        {
        case ERoomType::Start:
            ClassToSpawn = PlayerStartClass;
            break;
        case ERoomType::Boss:
            ClassToSpawn = BossSpawnClass;
            break;
        case ERoomType::NPC:
            ClassToSpawn = NPCSpawnClass;
            break;
        case ERoomType::Treasure_Common:
        case ERoomType::Treasure_Rare:
        case ERoomType::Treasure_Epic:
            ClassToSpawn = TreasureSpawnClass;
            break;
        default:
            break;
        }

        if (!ClassToSpawn) continue;

        FVector SpawnLocation = Room.WorldCenter;
        SpawnLocation.Z = 50.f;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor* Spawned = GetWorld()->SpawnActor<AActor>(
            ClassToSpawn, SpawnLocation, FRotator::ZeroRotator, Params);

        if (Spawned)
            SpawnedActors.Add(Spawned);
    }
}

void ADungeonGeneratorActor::ClearSpawnedActors()
{
    for (AActor* Actor : SpawnedActors)
        if (IsValid(Actor))
            Actor->Destroy();

    SpawnedActors.Empty();
}