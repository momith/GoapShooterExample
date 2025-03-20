#pragma once

#include "CoreMinimal.h"
#include "AI/Execution/ExecutableAction.h"
#include "Navigation/PathFollowingComponent.h"
#include "GoToRandomLocationExecutable.generated.h"

/**
 * Executable action that moves the AI to a random location
 * Uses Unreal's navigation system and movement completion notifications
 */
UCLASS()
class GOAPSHOOTER_API UGoToRandomLocationExecutable : public UExecutableAction
{
	GENERATED_BODY()
	
public:
	UGoToRandomLocationExecutable();
	
	// ExecutableAction interface
	virtual bool StartExecution() override;
	virtual void TickAction(float DeltaTime) override;
	virtual bool IsActionComplete() const override;
	virtual void AbortAction() override;
	// End of ExecutableAction interface
	
	/** Maximum search radius for finding a random location */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation")
	float MaxSearchRadius;
	
	/** Minimum search radius for finding a random location */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation")
	float MinSearchRadius;
	
	/** Maximum number of attempts to find a valid random location */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation")
	int32 MaxLocationFindingAttempts;
	
	/** Acceptance radius for reaching the target location */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation")
	float AcceptanceRadius;
	
private:
	/** Find a random navigable location */
	bool FindRandomLocation(FVector& OutLocation);
	
	/** Move to the target location */
	void MoveToLocation(const FVector& Location);
	
	/** Called when movement is completed */
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
	
	/** The target location to move to */
	UPROPERTY()
	FVector TargetLocation;
	
	/** Whether the action has started */
	UPROPERTY()
	bool bHasStarted;
	
	/** Request ID for the current move request */
	FAIRequestID CurrentMoveRequestID;
};
