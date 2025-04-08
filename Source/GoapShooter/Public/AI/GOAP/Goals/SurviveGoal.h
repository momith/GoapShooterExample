#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Goals/GoapGoal.h"
#include "SurviveGoal.generated.h"

/**
 * Goal to survive in the game world
 */
UCLASS()
class GOAPSHOOTER_API USurviveGoal : public UGoapGoal
{
	GENERATED_BODY()
	
public:
	USurviveGoal();

	virtual int32 GetPriority(const TMap<FString, FGoapValue>& WorldState) override;
	virtual bool ShallGoalBeConsidered(const TMap<FString, FGoapValue>& WorldState) override;
};
