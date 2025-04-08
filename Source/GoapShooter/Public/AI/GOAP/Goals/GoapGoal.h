#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Utils/GoapTypes.h"
#include "GoapGoal.generated.h"

/**
 * Base class for GOAP goals in the GoapShooter game.
 * Represents a goal that AI characters want to achieve using GOAP planning.
 */
UCLASS(Blueprintable, Abstract)
class GOAPSHOOTER_API UGoapGoal : public UObject
{
	GENERATED_BODY()
	
public:
	UGoapGoal();

	/**
	 * Add a desired state value to this goal
	 * @param Key - The key for the desired state
	 * @param Value - The value that would satisfy this goal
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void AddDesiredState(const FString& Key, const FGoapValue& Value);

	/**
	 * Get the priority of this goal (by default a constant value)
	 * @param WorldState - The world state to calculate priority based on, if desired
	 * @return The priority of the goal (higher values = more important)
	 */
	virtual int32 GetPriority(const TMap<FString, FGoapValue>& WorldState);

	/**
	 * Check if this goal should be considered based on the current world state
	 * @param WorldState - The current world state
	 * @return True if the goal should be considered
	 */
	virtual bool ShallGoalBeConsidered(const TMap<FString, FGoapValue>& WorldState);

	/**
	 * Check if this goal is satisfied by the current world state
	 * @param WorldState - The current world state
	 * @return True if all desired states are satisfied
	 */
	bool IsSatisfiedBy(const TMap<FString, FGoapValue>& WorldState);

	/**
	 * Get the name of this goal
	 * @return The name of the goal
	 */
	FString GetName() const { return Name; }

	TMap<FString, FGoapValue> GetDesiredState() const { return DesiredState; }

protected:

	/** Name of this goal */
	UPROPERTY()
	FString Name;

	/** Priority of this goal (higher values = more important) */
	UPROPERTY()
	int32 Priority;

	/** Desired world state (or rather part of it) that would satisfy this goal */
	UPROPERTY()
	TMap<FString, FGoapValue> DesiredState;

};
