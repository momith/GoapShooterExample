#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AI/GOAP/Utils/GoapTypes.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "AI/GOAP/Goals/GoapGoal.h"
#include "AI/GOAP/Planner/GoapNode.h"
#include "GoapPlanner.generated.h"

/**
 * GOAP AI Planner class for GoapShooter game.
 * Handles planning and execution of GOAP actions to achieve goals.
 */
UCLASS(Blueprintable)
class GOAPSHOOTER_API UGoapPlanner : public UObject
{
	GENERATED_BODY()
	
public:
	UGoapPlanner();

	/**
	 * Add an action to the planner
	 * @param Action - The action to add
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void AddAction(UGoapAction* Action);

	/**
	 * Add a goal to the planner
	 * @param Goal - The goal to add
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void AddGoal(UGoapGoal* Goal);

	/**
	 * Update a value in the world state
	 * @param Key - The key to update
	 * @param Value - The new value
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void UpdateWorldState(const FString& Key, const FGoapValue& Value);
	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void FullUpdateWorldState(TMap<FString, FGoapValue> WorldStateToSet);

	/**
	 * Select the best goal (goal with highest priority considering the current world state)
	 * @return The best goal to achieve
	 */
	UGoapGoal* SelectBestGoal();

	/**
	 * Plan the next action to take
	 * @return The next action to take, or nullptr if no action is available
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP")
	UGoapAction* PlanNextAction(UGoapGoal* Goal);

	/**
	 * Get all available actions
	 * @return Array of all actions in the planner
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP")
	const TArray<UGoapAction*>& GetActions() const { return Actions; }

private:

	/** List of available actions */
	UPROPERTY()
	TArray<UGoapAction*> Actions;

	/** List of goals */
	UPROPERTY()
	TArray<UGoapGoal*> Goals;

	/** Current world state */
	UPROPERTY()
	TMap<FString, FGoapValue> WorldState;

	/**
	 * A* planning algorithm to find a sequence of actions to achieve a goal
	 * @param StartState - The starting world state
	 * @param Goal - The goal to achieve
	 * @return The first action in the plan, or nullptr if no plan is found
	 */
	UGoapAction* AStarPlanning(const TMap<FString, FGoapValue>& StartState, UGoapGoal* Goal);

	/**
	 * Extract the first action from a plan
	 * @param Node - The goal node of the plan
	 * @return The first action in the plan
	 */
	UGoapAction* ExtractFirstAction(const TSharedPtr<FGoapNode>& Node);

	/**
	 * Calculate the heuristic cost from a state to a goal
	 * @param State - The current state
	 * @param Goal - The goal to achieve
	 * @return The heuristic cost
	 */
	float Heuristic(const TMap<FString, FGoapValue>& State, UGoapGoal* Goal);
};
