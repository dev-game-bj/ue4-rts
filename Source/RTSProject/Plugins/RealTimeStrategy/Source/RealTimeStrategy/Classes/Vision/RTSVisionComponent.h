#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"

#include "RTSVisionComponent.generated.h"


/**
* Allows the actor to reveal areas covered by fog of war.
*/
UCLASS(meta = (BlueprintSpawnableComponent))
class REALTIMESTRATEGY_API URTSVisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URTSVisionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    /** Performance Optimization. Tile the actor has been updated its vision at most recently. */
    FIntVector ActorLocationTile;

    /** Gets the radius in which the actor reveals areas covered by fog of war, in cm. */
    UFUNCTION(BlueprintPure)
    float GetSightRadius() const;

private:
    /** Radius in which the actor reveals areas covered by fog of war, in cm. */
    UPROPERTY(EditDefaultsOnly, Category = "RTS", meta = (ClampMin = 0))
    float SightRadius;
};
