#include "Controllers/GoapMovementPlannerConfiguration.h"
#include "AI/GOAP/Planner/GoapPlanner.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "AI/GOAP/Goals/GoapGoal.h"
#include "AI/GOAP/Goals/FindEnemyGoal.h"
#include "AI/GOAP/Actions/GoToRandomLocationAction.h"
#include "AI/Execution/GoToRandomLocationExecutable.h"
#include "AI/GOAP/Actions/WaitForPeekAction.h"
#include "AI/GOAP/Actions/GoToStimulusAction.h"
#include "AI/Execution/GoToStimulusExecutable.h"
#include "AI/GOAP/Actions/TurnToStimulusAction.h"
#include "AI/Execution/TurnToStimulusExecutable.h"
#include "AI/GOAP/Actions/GoToCoverAction.h"
#include "AI/Execution/GoToCoverExecutable.h"
#include "AI/GOAP/Goals/SurviveGoal.h"
#include "AI/GOAP/Actions/LeftRightDodgeAction.h"
#include "AI/Execution/LeftRightDodgeExecutableAction.h"
#include "AI/GOAP/Actions/PeekAction.h"
#include "AI/Execution/PeekExecutable.h"
#include "AI/GOAP/Actions/FlankAction.h"
#include "AI/Execution/FlankExecutableAction.h"
#include "AI/Execution/WaitForPeekExecutable.h"

UGoapMovementPlannerConfiguration::UGoapMovementPlannerConfiguration()
{
    //
}

void UGoapMovementPlannerConfiguration::InitializeGoapPlanner(UGoapPlanner* GoapPlanner)
{
    // Survive
    USurviveGoal* SurviveGoal = NewObject<USurviveGoal>(this);
    GoapPlanner->AddGoal(SurviveGoal);

    UGoToCoverAction* GoToCoverAction = NewObject<UGoToCoverAction>(this);
    GoapPlanner->AddAction(GoToCoverAction);

    ULeftRightDodgeAction* LeftRightDodgeAction = NewObject<ULeftRightDodgeAction>(this);
    GoapPlanner->AddAction(LeftRightDodgeAction);

    UFlankAction* FlankAction = NewObject<UFlankAction>(this);
    GoapPlanner->AddAction(FlankAction);

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

    UPeekAction* PeekAction = NewObject<UPeekAction>(this);
    GoapPlanner->AddAction(PeekAction);
    
}

UExecutableAction* UGoapMovementPlannerConfiguration::GetExecutableAction(UGoapAction* GoapAction)
{
    if (!GoapAction)
    {
        return nullptr;
    }
    
    if (GoapAction->IsA(UGoToRandomLocationAction::StaticClass()))
    {
        return NewObject<UGoToRandomLocationExecutable>(this);
    }
    else if (GoapAction->IsA(UTurnToStimulusAction::StaticClass()))
    {
        return NewObject<UTurnToStimulusExecutable>(this);
    }
    else if (GoapAction->IsA(UGoToStimulusAction::StaticClass()))
    {
        return NewObject<UGoToStimulusExecutable>(this);
    }
    else if (GoapAction->IsA(UGoToCoverAction::StaticClass()))
    {
        return NewObject<UGoToCoverExecutable>(this);
    }
    else if (GoapAction->IsA(ULeftRightDodgeAction::StaticClass()))
    {
        return NewObject<ULeftRightDodgeExecutableAction>(this);
    }
    else if (GoapAction->IsA(UPeekAction::StaticClass()))
    {
        return NewObject<UPeekExecutable>(this);
    }
    else if (GoapAction->IsA(UFlankAction::StaticClass()))
    {
        return NewObject<UFlankExecutableAction>(this);
    }
    else if (GoapAction->IsA(UWaitForPeekAction::StaticClass()))
    {
        return NewObject<UWaitForPeekExecutable>(this);
    }
    
    return nullptr;
}
