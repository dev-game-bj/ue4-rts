#include "Vision/RTSVisionManager.h"

#include "EngineUtils.h"
#include "Engine/World.h"

#include "RTSLog.h"
#include "Vision/RTSFogOfWarActor.h"
#include "Vision/RTSVisibleActor.h"
#include "Vision/RTSVisibleComponent.h"
#include "Vision/RTSVisionComponent.h"
#include "Vision/RTSVisionInfo.h"
#include "Vision/RTSVisionVolume.h"


ARTSVisionManager::ARTSVisionManager(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

void ARTSVisionManager::Initialize()
{
    UWorld* World = GetWorld();

    // Find manager and info actors.
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;

        if (IsValid(Actor))
        {
            if (Actor->IsA(ARTSFogOfWarActor::StaticClass()))
            {
                FogOfWarActor = Cast<ARTSFogOfWarActor>(Actor);
            }
            else if (Actor->IsA(ARTSVisionInfo::StaticClass()))
            {
                ARTSVisionInfo* VisionInfo = Cast<ARTSVisionInfo>(Actor);
                VisionInfos.Add(VisionInfo);
            }
            else if (Actor->IsA(ARTSVisionVolume::StaticClass()))
            {
                VisionVolume = Cast<ARTSVisionVolume>(Actor);
            }
        }
    }

    if (IsValid(VisionVolume))
    {
        VisionVolume->Initialize();
    }

    for (ARTSVisionInfo* VisionInfo : VisionInfos)
    {
        VisionInfo->Initialize(VisionVolume);
    }

    if (IsValid(FogOfWarActor))
    {
        FogOfWarActor->Initialize(VisionVolume);
    }

    // Discover initial vision actors.
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;

        if (!IsValid(Actor))
        {
            continue;
        }

        URTSVisibleComponent* VisibleComponent = Actor->FindComponentByClass<URTSVisibleComponent>();

        if (!IsValid(VisibleComponent))
        {
            continue;
        }

        AddVisibleActor(Actor);
    }
}

void ARTSVisionManager::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Update unit visibility.
    for (FRTSVisibleActor& VisibleActor : VisibleActors)
    {
        if (!VisibleActor.IsActorValid())
        {
            continue;
        }

        FIntVector TileLocation;

        if (IsValid(VisibleActor.VisionComponent))
        {
            TileLocation = VisibleActor.VisionComponent->ActorLocationTile;
        }
        else
        {
            TileLocation = IsValid(VisionVolume)
                ? VisionVolume->WorldToTile(VisibleActor.Actor->GetActorLocation())
                : FIntVector::ZeroValue;
        }

        ERTSVisionState NewVision = IsValid(LocalVisionInfo)
            ? LocalVisionInfo->GetVision(TileLocation.X, TileLocation.Y)
            : ERTSVisionState::VISION_Visible;

        VisibleActor.VisibleComponent->SetClientVisionState(NewVision);
    }
}

void ARTSVisionManager::SetLocalVisionInfo(ARTSVisionInfo* InLocalVisionInfo)
{
    LocalVisionInfo = InLocalVisionInfo;

    if (!VisionInfos.Contains(LocalVisionInfo))
    {
        // It seems like this has been replicated after our initialization.
        VisionInfos.Add(LocalVisionInfo);
        LocalVisionInfo->Initialize(VisionVolume);
    }

    if (IsValid(FogOfWarActor))
    {
        FogOfWarActor->SetupVisionInfo(LocalVisionInfo);
    }

    if (IsValid(LocalVisionInfo))
    {
        UE_LOG(LogRTS, Log, TEXT("Using local vision info %s."), *LocalVisionInfo->GetName());
    }
}

void ARTSVisionManager::AddVisibleActor(AActor* Actor)
{
    for (int32 Index = 0; Index < VisibleActors.Num(); ++Index)
    {
        if (VisibleActors[Index].Actor == Actor)
        {
            // Already added.
            return;
        }
    }

    FRTSVisibleActor VisibleActor(Actor);

    if (!VisibleActor.IsActorValid())
    {
        return;
    }

    VisibleActors.Add(VisibleActor);
}

void ARTSVisionManager::RemoveVisibleActor(AActor* Actor)
{
    int32 IndexToRemove = -1;

    for (int32 Index = 0; Index < VisibleActors.Num(); ++Index)
    {
        if (VisibleActors[Index].Actor == Actor)
        {
            IndexToRemove = Index;
            break;
        }
    }

    if (VisibleActors.IsValidIndex(IndexToRemove))
    {
        VisibleActors.RemoveAt(IndexToRemove);
    }
}
