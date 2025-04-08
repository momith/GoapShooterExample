#include "Controllers/GoapCombatPlannerConfiguration.h"
#include "AI/GOAP/Planner/GoapPlanner.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "AI/GOAP/Goals/GoapGoal.h"
#include "AI/GOAP/Goals/Combat/KillEnemyGoal.h"
#include "AI/GOAP/Actions/Combat/ShootAtEnemyAction.h"
#include "AI/Execution/Combat/ShootAtEnemyExecutable.h"

UGoapCombatPlannerConfiguration::UGoapCombatPlannerConfiguration()
{
}

void UGoapCombatPlannerConfiguration::InitializeGoapPlanner(UGoapPlanner* GoapPlanner)
{
    // Kill Enemy
    UKillEnemyGoal* KillEnemyGoal = NewObject<UKillEnemyGoal>(this);
    GoapPlanner->AddGoal(KillEnemyGoal);
    
    UShootAtEnemyAction* ShootAtEnemyAction = NewObject<UShootAtEnemyAction>(this);
    GoapPlanner->AddAction(ShootAtEnemyAction);
}

UExecutableAction* UGoapCombatPlannerConfiguration::GetExecutableAction(UGoapAction* GoapAction)
{
    if (!GoapAction)
    {
        return nullptr;
    }
    
    if (GoapAction->IsA(UShootAtEnemyAction::StaticClass()))
    {
        return NewObject<UShootAtEnemyExecutable>(this);
    }
    
    return nullptr;
}
