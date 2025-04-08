#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "FlankAction.generated.h"

/**
 * GOAP Action for flanking an enemy by moving to a position that avoids
 * the enemy's line of sight while providing a tactical advantage.
 */
UCLASS()
class GOAPSHOOTER_API UFlankAction : public UGoapAction
{
    GENERATED_BODY()
    
public:
    UFlankAction();
    
    virtual float GetCost(const TMap<FString, FGoapValue>& WorldState) override;
};
