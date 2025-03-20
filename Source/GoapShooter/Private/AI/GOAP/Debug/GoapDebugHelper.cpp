#include "AI/GOAP/Debug/GoapDebugHelper.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "AI/GOAP/Goals/GoapGoal.h"

UGoapDebugHelper::UGoapDebugHelper()
{
    Planner = nullptr;
}

void UGoapDebugHelper::Initialize(UGoapPlanner* InPlanner)
{
    Planner = InPlanner;
}

void UGoapDebugHelper::UpdateActionDebugInfo(const TMap<FString, FGoapValue>& CurrentWorldState)
{
    // Clear previous debug info
    ActionDebugInfo.Empty();
    
    if (!Planner)
    {
        UE_LOG(LogTemp, Warning, TEXT("GoapDebugHelper: Planner is null"));
        return;
    }
    
    // Get all actions from the planner
    const TArray<UGoapAction*>& Actions = Planner->GetActions();
    for (UGoapAction* Action : Actions)
    {
        if (Action)
        {
            // Evaluate action for current world state
            bool bShouldBeConsidered = Action->ShallActionBeConsidered(CurrentWorldState);
            bool bPreconditionsSatisfied = Action->ArePreconditionsSatisfied(CurrentWorldState);
            float Cost = Action->GetCost(CurrentWorldState);
            
            // Create debug info
            FGoapActionDebugInfo DebugInfo(
                Action->GetName(),
                bShouldBeConsidered,
                bPreconditionsSatisfied,
                Cost
            );
            
            // Add to array
            ActionDebugInfo.Add(DebugInfo);
        }
    }
}

void UGoapDebugHelper::UpdateCurrentActionAndCurrentGoal(UGoapAction* CurrentAction, UGoapGoal* CurrentGoal)
{
    // Update current action and goal
    TrackedCurrentAction = CurrentAction;
    TrackedCurrentGoal = CurrentGoal;
}

void UGoapDebugHelper::LogActionDebugInfo(UGoapAction* CurrentAction, UGoapGoal* CurrentGoal)
{
    // // Log header
    // UE_LOG(LogTemp, Display, TEXT("===== GOAP ACTION DEBUG INFO ====="));
    
    // // Log current action if any
    // if (CurrentAction)
    // {
    //     UE_LOG(LogTemp, Display, TEXT("Current Action: %s"), *CurrentAction->GetName());
    // }
    // else
    // {
    //     UE_LOG(LogTemp, Display, TEXT("Current Action: None"));
    // }
    
    // // Log current goal if any
    // if (CurrentGoal)
    // {
    //     UE_LOG(LogTemp, Display, TEXT("Current Goal: %s"), *CurrentGoal->GetName());
    // }
    // else
    // {
    //     UE_LOG(LogTemp, Display, TEXT("Current Goal: None"));
    // }
    
    // // Log all action debug info
    // UE_LOG(LogTemp, Display, TEXT("Action Evaluations:"));
    // for (const FGoapActionDebugInfo& DebugInfo : ActionDebugInfo)
    // {
    //     UE_LOG(LogTemp, Display, TEXT("  %s:"), *DebugInfo.ActionName);
    //     UE_LOG(LogTemp, Display, TEXT("    Should Be Considered: %s"), DebugInfo.bShouldBeConsidered ? TEXT("true") : TEXT("false"));
    //     UE_LOG(LogTemp, Display, TEXT("    Preconditions Satisfied: %s"), DebugInfo.bPreconditionsSatisfied ? TEXT("true") : TEXT("false"));
    //     UE_LOG(LogTemp, Display, TEXT("    Cost: %f"), DebugInfo.Cost);
    // }
    
    // UE_LOG(LogTemp, Display, TEXT("=================================="));
}
