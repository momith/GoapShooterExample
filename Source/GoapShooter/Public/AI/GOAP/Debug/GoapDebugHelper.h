#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Utils/GoapTypes.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "AI/GOAP/Goals/GoapGoal.h"
#include "AI/GOAP/Planner/GoapPlanner.h"
#include "GoapDebugHelper.generated.h"

/**
 * Struct to hold debug information for a GOAP action
 */
USTRUCT(BlueprintType)
struct FGoapActionDebugInfo
{
    GENERATED_BODY()
    
    /** Name of the action */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP|Debug")
    FString ActionName;
    
    /** Whether the action should be considered */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP|Debug")
    bool bShouldBeConsidered;
    
    /** Whether the action's preconditions are satisfied */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP|Debug")
    bool bPreconditionsSatisfied;
    
    /** Cost of the action */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP|Debug")
    float Cost;
    
    FGoapActionDebugInfo()
        : ActionName(TEXT("None"))
        , bShouldBeConsidered(false)
        , bPreconditionsSatisfied(false)
        , Cost(0.0f)
    {
    }
    
    FGoapActionDebugInfo(const FString& InActionName, bool InShouldBeConsidered, bool InPreconditionsSatisfied, float InCost)
        : ActionName(InActionName)
        , bShouldBeConsidered(InShouldBeConsidered)
        , bPreconditionsSatisfied(InPreconditionsSatisfied)
        , Cost(InCost)
    {
    }
};

/**
 * Helper class for debugging GOAP planning and execution
 */
UCLASS(Blueprintable)
class GOAPSHOOTER_API UGoapDebugHelper : public UObject
{
    GENERATED_BODY()
    
public:
    UGoapDebugHelper();
    
    /** Initialize the debug helper with a planner */
    void Initialize(UGoapPlanner* InPlanner);
    
    /** Update debug information for all actions */
    void UpdateActionDebugInfo(const TMap<FString, FGoapValue>& CurrentWorldState);
    
    /** Log debug information for all actions */
    void LogActionDebugInfo(UGoapAction* CurrentAction, UGoapGoal* CurrentGoal);
    
    /** Update current action and goal */
    void UpdateCurrentActionAndCurrentGoal(UGoapAction* CurrentAction, UGoapGoal* CurrentGoal);
    
    /** Get the debug information for all actions */
    UFUNCTION(BlueprintCallable, Category = "GOAP|Debug")
    const TArray<FGoapActionDebugInfo>& GetActionDebugInfo() const { return ActionDebugInfo; }
    
    /** Get the current action */
    UFUNCTION(BlueprintCallable, Category = "GOAP|Debug")
    UGoapAction* GetTrackedCurrentAction() const { return TrackedCurrentAction; }
    
    /** Get the current goal */
    UFUNCTION(BlueprintCallable, Category = "GOAP|Debug")
    UGoapGoal* GetTrackedCurrentGoal() const { return TrackedCurrentGoal; }
    
private:
    /** Reference to the GOAP planner */
    UPROPERTY()
    UGoapPlanner* Planner;
    
    /** Debug information for all actions */
    UPROPERTY()
    TArray<FGoapActionDebugInfo> ActionDebugInfo;

UPROPERTY()
    UGoapAction* TrackedCurrentAction; 
    UPROPERTY()
    UGoapGoal* TrackedCurrentGoal;
};
