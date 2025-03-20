#include "Controllers/GoapShooterAIConfiguration.h"
#include "Controllers/GoapShooterAIControllerBase.h"
#include "AI/GOAP/Planner/GoapPlanner.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "AI/GOAP/Goals/GoapGoal.h"
#include "AI/GOAP/Goals/FindEnemyGoal.h"
#include "AI/GOAP/Actions/GoToRandomLocationAction.h"
#include "AI/Execution/GoToRandomLocationExecutable.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"
#include "AI/GOAP/Actions/ShootAtEnemyAction.h"
#include "AI/Execution/ShootAtEnemyExecutable.h"
#include "AI/GOAP/Actions/WaitForPeekAction.h"
#include "AI/GOAP/Goals/KillEnemyGoal.h"
#include "AI/GOAP/Actions/GoToStimulusAction.h"
#include "AI/Execution/GoToStimulusExecutable.h"
#include "AI/GOAP/Actions/TurnToStimulusAction.h"
#include "AI/Execution/TurnToStimulusExecutable.h"

UGoapShooterAIConfiguration::UGoapShooterAIConfiguration()
{
    //
}

void UGoapShooterAIConfiguration::InitializeGoapPlanner(UGoapPlanner* GoapPlanner)
{   
    // Find Enemy
    UFindEnemyGoal* FindEnemyGoal = NewObject<UFindEnemyGoal>(this);
    GoapPlanner->AddGoal(FindEnemyGoal);
    
    UGoToRandomLocationAction* GoToRandomLocationAction = NewObject<UGoToRandomLocationAction>(this);
    GoapPlanner->AddAction(GoToRandomLocationAction);

    UWaitForPeekAction* WaitForPeekAction = NewObject<UWaitForPeekAction>(this);
    GoapPlanner->AddAction(WaitForPeekAction);
    
    // TODO: implement and test this later for audio stimuli
    //UGoToStimulusAction* GoToStimulusAction = NewObject<UGoToStimulusAction>(this);
    //GoapPlanner->AddAction(GoToStimulusAction);
    
    UTurnToStimulusAction* TurnToStimulusAction = NewObject<UTurnToStimulusAction>(this);
    GoapPlanner->AddAction(TurnToStimulusAction);

    // Kill Enemy
    UKillEnemyGoal* KillEnemyGoal = NewObject<UKillEnemyGoal>(this);
    GoapPlanner->AddGoal(KillEnemyGoal);

    UShootAtEnemyAction* ShootAtEnemyAction = NewObject<UShootAtEnemyAction>(this);
    GoapPlanner->AddAction(ShootAtEnemyAction);

}

UExecutableAction* UGoapShooterAIConfiguration::GetExecutableAction(UGoapAction* GoapAction)
{
    if (!GoapAction)
    {
        return nullptr;
    }
    
    if (GoapAction->IsA(UGoToRandomLocationAction::StaticClass()))
    {
        return NewObject<UGoToRandomLocationExecutable>(this);
    }
    
    if (GoapAction->IsA(UShootAtEnemyAction::StaticClass()))
    {
        return NewObject<UShootAtEnemyExecutable>(this);
    }

    if (GoapAction->IsA(UTurnToStimulusAction::StaticClass()))
    {
        return NewObject<UTurnToStimulusExecutable>(this);
    }

    if (GoapAction->IsA(UGoToStimulusAction::StaticClass()))
    {
        return NewObject<UGoToStimulusExecutable>(this);
    }
    
    return nullptr;
}
