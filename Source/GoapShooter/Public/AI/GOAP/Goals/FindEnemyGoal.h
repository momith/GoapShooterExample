#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Goals/GoapGoal.h"
#include "FindEnemyGoal.generated.h"

/**
 * Goal to find an enemy in the game world
 */
UCLASS()
class GOAPSHOOTER_API UFindEnemyGoal : public UGoapGoal
{
	GENERATED_BODY()
	
public:
	UFindEnemyGoal();

	virtual int32 GetPriority(const TMap<FString, FGoapValue>& WorldState) override;
	virtual bool ShallGoalBeConsidered(const TMap<FString, FGoapValue>& WorldState) override;
};
