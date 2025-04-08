#include "AI/GOAP/Goals/SurviveGoal.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"

USurviveGoal::USurviveGoal()
{
    Name = TEXT("Survive");
    Priority = 60;
    AddDesiredState(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsInDanger), FGoapValue(false));
}

int32 USurviveGoal::GetPriority(const TMap<FString, FGoapValue>& WorldState)
{
    int32 Prio = Priority;
    if (WorldState.Contains(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsInDanger)) 
        && WorldState[FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsInDanger)] == FGoapValue(false))
    {
        Prio -= 20.0f;
    }
    return Prio;
}

bool USurviveGoal::ShallGoalBeConsidered(const TMap<FString, FGoapValue>& WorldState)
{
    return true;
}