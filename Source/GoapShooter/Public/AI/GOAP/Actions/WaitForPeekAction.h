#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "WaitForPeekAction.generated.h"

/**
 * Action to wait at a position to peek and potentially spot enemies
 * This action becomes more interesting when an enemy was recently seen
 * and less interesting as time passes since the last sighting
 */
UCLASS()
class GOAPSHOOTER_API UWaitForPeekAction : public UGoapAction
{
	GENERATED_BODY()
	
public:
	UWaitForPeekAction();

	virtual float GetCost(const TMap<FString, FGoapValue>& WorldState) override;

	/** Maximum time in seconds since last enemy sighting for this action to be considered */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	float MaxTimeSinceLastSighting = 6.0f;
	
	/** Minimum time in seconds since last enemy sighting for this action to be considered */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	float MinTimeSinceLastSighting = 0.5f;
	
	/** Cost multiplier that increases as time since last sighting increases */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	float TimeSinceSightingCostMultiplier = 0.5f;

	/** Minimum threshold time (in seconds) after which costs increase rapidly */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	float MinThresholdTime = 1.0f;
	
	/** Maximum threshold time (in seconds) after which costs increase rapidly */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	float MaxThresholdTime = 4.0f;
	
	/** Accelerated cost multiplier after threshold is reached */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	float AcceleratedCostMultiplier = 10.0f;

	/** Seed for randomization */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	int32 RandomSeed = 0;

private:
	/** Random threshold time for this instance */
	float ThresholdTime;
	
	/** Whether the threshold has been initialized */
	bool bThresholdInitialized;
};
