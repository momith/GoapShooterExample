#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "GoToCoverAction.generated.h"

/**
 * Action to move to a cover point
 */
UCLASS()
class GOAPSHOOTER_API UGoToCoverAction : public UGoapAction
{
	GENERATED_BODY()
	
public:
	UGoToCoverAction();

	virtual float GetCost(const TMap<FString, FGoapValue>& WorldState) override;

    // this is the threshold where going to cover becomes preferable to dodging even when aimed at
    float VeryNearThreshold = 400.0f;
};
