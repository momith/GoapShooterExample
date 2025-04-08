#pragma once

#include "CoreMinimal.h"
#include "AI/Execution/ExecutableAction.h"
#include "Navigation/PathFollowingComponent.h"
#include "FlankExecutableAction.generated.h"

/**
 * Executable action for flanking an enemy by moving to a position that avoids
 * the enemy's line of sight while providing a tactical advantage.
 */
UCLASS()
class GOAPSHOOTER_API UFlankExecutableAction : public UExecutableAction
{
    GENERATED_BODY()
    
public:
    UFlankExecutableAction();

	virtual bool StartExecution() override;
	virtual void TickAction(float DeltaTime) override;
	virtual void AbortAction() override;

protected:
	virtual void Internal_OnActionCompleted(bool bSuccess) override;

private:

    FVector CalculateTargetLocation();

	UFUNCTION()
	void TellControllerThatFlankHasEnded();
	
	UPROPERTY()
	FVector TargetLocation;
	UPROPERTY()
	bool bHasStarted;

	TArray<FVector> Path;
	
	// Current path point index
	UPROPERTY()
	int32 CurrentPathIndex;
	
	// Move to the next point in the path
	void MoveToNextPathPoint();
	
	// Called when movement to a path point is completed
	UFUNCTION()
	void OnPathPointReached(FAIRequestID RequestID, EPathFollowingResult::Type Result);

};
