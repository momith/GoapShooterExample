#include "AI/GOAP/Goals/Combat/KillEnemyGoal.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"

UKillEnemyGoal::UKillEnemyGoal()
{
    Name = TEXT("KillEnemy");
    Priority = 100;

    AddDesiredState(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::AllEnemiesInSightAreKilled), FGoapValue(true));
}

int32 UKillEnemyGoal::GetPriority(const TMap<FString, FGoapValue>& WorldState)
{
    // Base priority
    int32 CurrentPriority = Priority;
    
    // TODO: Consider health or something (fight or flight) ?
    
    return CurrentPriority;
}

bool UKillEnemyGoal::ShallGoalBeConsidered(const TMap<FString, FGoapValue>& WorldState)
{
    // Only consider this goal if we can see an enemy
    FString CanSeeEnemyKey = FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy);
    if (WorldState.Contains(CanSeeEnemyKey) && !WorldState[CanSeeEnemyKey].BoolValue)
    {
        // We don't see an enemy, can't kill what we can't see
        return false;
    }
    
    return true;
}
