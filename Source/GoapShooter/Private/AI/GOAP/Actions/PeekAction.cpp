#include "AI/GOAP/Actions/PeekAction.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"
#include "Controllers/GoapShooterAIController.h"
#include "Kismet/GameplayStatics.h"

UPeekAction::UPeekAction()
{
    Name = TEXT("PeekAction");
    Cost = 3.0f;
    
    // Preconditions: AI cannot see enemy, is in cover, and knows enemy's last location
    //AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsInCover), FGoapValue(true));
    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsFocusedOnStrongPerceivedStimulus), FGoapValue(true));
    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy), FGoapValue(false));
    //AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::HasLastSeenEnemyLocation), FGoapValue(true));
    
    // Effects: AI can see enemy again
    AddEffect(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy), FGoapValue(true));
}

float UPeekAction::GetCost(const TMap<FString, FGoapValue>& WorldState)
{
    float ActionCost = Cost;
    
    // peeking makes sense, if we are at a cover point
    bool bIsInCover = WorldState[FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsInCover)].BoolValue;
    if (bIsInCover)
    {
        ActionCost *= 0.5f;
    }

    // Get the world time when enemy was last seen
    float WorldTimeLastEnemySeen = -FLT_MAX;
    FString WorldTimeLastEnemySeenKey = FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::WorldTimeLastEnemySeen);
    if (WorldState.Contains(WorldTimeLastEnemySeenKey))
    {
        WorldTimeLastEnemySeen = WorldState[WorldTimeLastEnemySeenKey].FloatValue;
    }
    
    // Get current world time
    float CurrentWorldTime = UGameplayStatics::GetTimeSeconds(GetWorld());
    
    // Calculate time since enemy was last seen
    float TimeSinceLastSeen = CurrentWorldTime - WorldTimeLastEnemySeen;
    
    // Adjust cost based on time since last seen
    // The more recent we saw the enemy, the lower the cost (more likely to peek)
    if (TimeSinceLastSeen < 5.0f)
    {
        // Recently seen enemy - reduce cost to encourage peeking
        ActionCost *= 0.7f;
    }
    else if (TimeSinceLastSeen > 10.0f)
    {
        // Enemy was seen a while ago - increase cost
        ActionCost *= 1.5f;
    }
    
    return ActionCost;
}