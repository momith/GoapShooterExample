#include "AI/GOAP/Actions/ShootAtEnemyAction.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"

UShootAtEnemyAction::UShootAtEnemyAction()
{
    Name = TEXT("ShootAtEnemyAction");
    Cost = 10.0f; // Lower cost than movement actions
    
    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy), FGoapValue(true));
    
    AddEffect(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::AllEnemiesInSightAreKilled), FGoapValue(true));
}

float UShootAtEnemyAction::GetCost(const TMap<FString, FGoapValue>& WorldState)
{
    float CurrentCost = Cost;
    
    // TODO: Check distance to enemy to adjust cost ?
    // TODO: Check ammo ?
    
    return CurrentCost;
}
