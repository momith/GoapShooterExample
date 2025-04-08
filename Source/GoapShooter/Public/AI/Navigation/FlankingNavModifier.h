#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "NavModifierComponent.h"
#include "FlankingNavModifier.generated.h"

/**
 * Actor which holds a nav modifier component with area class UNavArea_FlankingBlock.
 * Can be spawned, scaled and located, in order to block specific part of the navigable area of a path search.
 * The path search needs to be filtered by UNavQueryFilter_AvoidFlanking.
 */
UCLASS()
class GOAPSHOOTER_API AFlankingNavModifier : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlankingNavModifier();

    UPROPERTY(EditAnywhere)
    UBoxComponent* BoxCollider;

    UPROPERTY(EditAnywhere)
    UNavModifierComponent* NavModifier;

};
