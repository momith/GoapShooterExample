#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "PeekAction.generated.h"

/**
 * GOAP Action for peeking from cover to regain sight of an enemy.
 * The AI will move orthogonally to the enemy direction to peek around cover.
 */
UCLASS()
class GOAPSHOOTER_API UPeekAction : public UGoapAction
{
    GENERATED_BODY()
    
public:
    UPeekAction();
    
    virtual float GetCost(const TMap<FString, FGoapValue>& WorldState) override;
};
