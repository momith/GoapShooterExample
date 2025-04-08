#pragma once

#include "CoreMinimal.h"
#include "ExecutableAction.generated.h"

/**
 * Delegate for action completion events
 * @param Action - The action that completed
 * @param Success - Whether the action completed successfully
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionCompletedDelegate, class UExecutableAction*, Action, bool, Success);

/**
 * Base class for executable actions that can be executed by the AI Controller
 * Handles the execution of GOAP actions and notifies the AI Controller upon completion
 *
 * The concept of executable actions is a little bit unintuitive and messy and should be refactored: When an
 * executable action is created for a GOAP action, the method Initialize is called. Afterwards, the TickAction
 * method will be called in the tick of the planner until the planner is notified about the end of the action.
 * These are the only methods you can count on being called by the planning component.
 *
 * TODO refactor the concept of ExecutableAction
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class GOAPSHOOTER_API UExecutableAction : public UObject
{
	GENERATED_BODY()
	
public:
	UExecutableAction();
	
	/**
	 * Initialize the executable action with the AI Controller and GOAP action
	 * @param InController - The AI Controller that owns this action
	 * @param InGoapAction - The GOAP action to execute
	 */
	void Initialize(class AGoapShooterAIController* InController, class UGoapAction* InGoapAction);
	
	/**
	 * Start executing the action
	 * @return True if the action started successfully, false otherwise
	 */
	virtual bool StartExecution();
	
	/**
	 * Tick the action
	 * @param DeltaTime - Time since last tick
	 */
	virtual void TickAction(float DeltaTime);
	
	/**
	 * Check if the action is complete
	 * @return True if the action is complete, false otherwise
	 */
	virtual bool IsActionComplete() const;
	
	/**
	 * Abort the action (called when the AI Controller wants to stop the action)
	 */
	virtual void AbortAction();
	
	/**
	 * Called when the action completes successfully
	 */
	virtual void CompleteAction();
	
	/**
	 * Called when the action fails
	 */
	virtual void FailAction();
	
	/**
	 * Get the name of the action
	 * @return The name of the action
	 */
	FString GetActionName() const;
	
	/**
	 * Event fired when the action completes (successfully or not)
	 */
	UPROPERTY(BlueprintAssignable)
	FOnActionCompletedDelegate OnActionCompleted;
	
	bool GetIsRelatedToMostInterestingActor() const { return bIsRelatedToMostInterestingActor; }
	AActor* GetMostInterestingActorWhenActionStarted() const { return MostInterestingActorWhenActionStarted; }

protected:
	/** The AI Controller that owns this action */
	UPROPERTY()
	class AGoapShooterAIController* OwnerController;
	
	/** The GOAP action being executed */
	UPROPERTY()
	class UGoapAction* GoapAction;
	
	/** Whether the action is currently executing */
	UPROPERTY()
	bool bIsExecuting;
	
	/** Whether the action is complete */
	UPROPERTY()
	bool bIsComplete;
	
	/** Whether the action was successful */
	UPROPERTY()
	bool bWasSuccessful;
	
	/** Time spent executing the action */
	UPROPERTY()
	float ExecutionTime;

	virtual void Internal_OnActionCompleted(bool bSuccess);

	UPROPERTY()
	AActor* MostInterestingActorWhenActionStarted;

	/** Whether the action is related to the most interesting actor. By default true. */
	UPROPERTY()
	bool bIsRelatedToMostInterestingActor = true;

};
