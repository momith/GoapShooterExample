#pragma once

#include "CoreMinimal.h"
#include "AI/Execution/ExecutableAction.h"
#include "Navigation/PathFollowingComponent.h"
#include "GoToExecutableAction.generated.h"

/**
 * Executable action that moves the AI to a location
 * Uses Unreal's navigation system and movement completion notifications
 */
UCLASS()
class GOAPSHOOTER_API UGoToExecutableAction : public UExecutableAction
{
	GENERATED_BODY()
	
public:
	UGoToExecutableAction();
	
	virtual bool StartExecution() override;
	virtual void TickAction(float DeltaTime) override;
	virtual void AbortAction() override;
	
	/**
     * To be overwritten by child: 
     * Calculate the target location to move to. 
     * Zero vector is considered as null. 
     */
	virtual FVector CalculateTargetLocation();

	/** 
	 * Prepare a move request for the given target location. 
	 * Can be overwritten by child for custom movement behavior.
	 */
	virtual FAIMoveRequest PrepareMoveRequest(const FVector& LocationToMoveTo);

	/** Acceptance radius for reaching the target location */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation")
	float AcceptanceRadius;
	
private:

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
