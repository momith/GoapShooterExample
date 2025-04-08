#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "LeftRightDodgeAction.generated.h"

/**
 * GOAP Action for dodging left or right.
 */
UCLASS()
class GOAPSHOOTER_API ULeftRightDodgeAction : public UGoapAction
{
    GENERATED_BODY()
    
public:
    ULeftRightDodgeAction();
    
    virtual float GetCost(const TMap<FString, FGoapValue>& WorldState) override;
};
