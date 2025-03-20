#include "AI/GOAP/Actions/GoToRandomLocationAction.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"

UGoToRandomLocationAction::UGoToRandomLocationAction()
{
    Name = TEXT("GoToRandomLocationAction");
    Cost = 20.0f;


    AddEffect(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy), FGoapValue(true));
}

float UGoToRandomLocationAction::GetCost(const TMap<FString, FGoapValue>& WorldState)
{
    return Cost;
}

bool UGoToRandomLocationAction::ShallActionBeConsidered(const TMap<FString, FGoapValue>& WorldState)
{
    return true;
}
