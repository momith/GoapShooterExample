#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Utils/GoapTypes.h"
#include "GoapAction.generated.h"

/**
 * Base class for GOAP actions in the GoapShooter game.
 * Represents an action that can be performed by AI characters using GOAP planning.
 *
 * Note: This class shall not be respnsible for executing the action. It shall only be respnsible for planning purposes.
 */
UCLASS(Blueprintable, Abstract)
class GOAPSHOOTER_API UGoapAction : public UObject
{
	GENERATED_BODY()
	
public:
	UGoapAction();

	/**
	 * Add a precondition to this action
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void AddPrecondition(const FString& Key, const FGoapValue& Value);

	/**
	 * Add an effect to this action
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void AddEffect(const FString& Key, const FGoapValue& Value);

	/**
	 * Check if this action is valid given the current world state
	 * @param WorldState - The current world state
	 * @return True if all preconditions are satisfied
	 */
	bool ArePreconditionsSatisfied(const TMap<FString, FGoapValue>& WorldState) const;

	/**
	 * Apply the effects of this action to the world state
	 * @param WorldState - The current world state
	 * @return A new world state with the effects applied
	 */
	TMap<FString, FGoapValue> ApplyEffects(const TMap<FString, FGoapValue>& WorldState) const;

	/**
	 * Check if this action should be considered for planning
	 * Can be used for completely deactivating the action for the planning for whatever reasons, e.g. debugging.
	 * But in general the decision if action shall be considered shall be driven by the costs and the preconditions (and the effects).
	 * 
	 * @return True if the action is valid and should be considered. 
	 */
	virtual bool ShallActionBeConsidered(const TMap<FString, FGoapValue>& WorldState);

	/**
	 * Get the cost of performing this action
	 * @return The cost of the action
	 */
	virtual float GetCost(const TMap<FString, FGoapValue>& WorldState);

	/**
	 * Get the name of this action
	 * @return The name of the action
	 */
	FString GetName() const { return Name; }

protected:

	/** Name of this action */
	UPROPERTY()
	FString Name;

	/** Cost of performing this action */
	UPROPERTY()
	float Cost;

	/** Preconditions that must be true for this action to be valid */
	UPROPERTY()
	TMap<FString, FGoapValue> Preconditions;

	/** Effects that will become true after this action is performed */
	UPROPERTY()
	TMap<FString, FGoapValue> Effects;
};
