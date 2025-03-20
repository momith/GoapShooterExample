#include "AI/GOAP/Goals/FindEnemyGoal.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"

UFindEnemyGoal::UFindEnemyGoal()
{
    Name = TEXT("FindEnemy");
    Priority = 50;
    AddDesiredState(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy), FGoapValue(true));
}

int32 UFindEnemyGoal::GetPriority(const TMap<FString, FGoapValue>& WorldState)
{
    return Priority;
}

bool UFindEnemyGoal::ShallGoalBeConsidered(const TMap<FString, FGoapValue>& WorldState)
{
    //// Only consider this goal if we don't already see an enemy
    //FString CanSeeEnemyKey = FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy);
    //if (WorldState.Contains(CanSeeEnemyKey) && WorldState[CanSeeEnemyKey].BoolValue)
    //{
    //    // We already see an enemy, no need to find one
    //    return false;
    //}
    
    return true;
}
