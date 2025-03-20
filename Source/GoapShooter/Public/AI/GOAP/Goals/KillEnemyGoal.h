#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Goals/GoapGoal.h"
#include "KillEnemyGoal.generated.h"

/**
 * Goal to kill an enemy in the game world
 */
UCLASS()
class GOAPSHOOTER_API UKillEnemyGoal : public UGoapGoal
{
	GENERATED_BODY()
	
public:
	UKillEnemyGoal();

	virtual int32 GetPriority(const TMap<FString, FGoapValue>& WorldState) override;
	virtual bool ShallGoalBeConsidered(const TMap<FString, FGoapValue>& WorldState) override;
};
