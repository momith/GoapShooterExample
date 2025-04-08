#include "AI/GOAP/Actions/FlankAction.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"
#include "Controllers/GoapShooterAIController.h"
#include "DrawDebugHelpers.h"

UFlankAction::UFlankAction()
{
    Name = TEXT("FlankAction");
    Cost = 1.5f; // Flanking usually results in a better position, so we score this action high
    
    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsFlankTargetLocationAvailable), FGoapValue(true));

    // Effects: AI is in a better tactical position 
    // TODO >> should have some nice cost calculation effect for other actions (should decrease the costs of killingenemy) <<
    AddEffect(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsInDanger), FGoapValue(false));
    AddEffect(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy), FGoapValue(true));
    AddEffect(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsAimedAt), FGoapValue(false));
}

float UFlankAction::GetCost(const TMap<FString, FGoapValue>& WorldState)
{
    //return 1.0f; // for testing

    float ActionCost = Cost;
    
    // TODO Check for cover in front vs. sides ???

    //if (WorldState.Contains(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy))
    //    && !WorldState[FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy)].BoolValue)
    //{
    //    ActionCost -= 6.0f; // Decrease cost if enemy is not visible
    //}
    
    //else if (WorldState.Contains(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsAimedAt))
    //    && !WorldState[FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsAimedAt)].BoolValue)
    //{
    //    ActionCost -= 4.0f; // Decrease cost if enemy is visible, but enemy does not aim at us
    //}
    
    return ActionCost;
}
