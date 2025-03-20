#include "Controllers/GoapShooterAIControllerBase.h"
#include "Characters/GoapShooterAICharacter.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"
#include "AI/GOAP/Goals/FindEnemyGoal.h"
#include "AI/GOAP/Actions/GoToRandomLocationAction.h"
#include "AI/Execution/ExecutableAction.h"
#include "AI/Execution/GoToRandomLocationExecutable.h"

AGoapShooterAIControllerBase::AGoapShooterAIControllerBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
    PrimaryActorTick.bCanEverTick = true;
    
    GoapConfig = CreateDefaultSubobject<UGoapShooterAIConfiguration>(TEXT("GoapConfig"));
    GoapPlanner = CreateDefaultSubobject<UGoapPlanner>(TEXT("GoapPlanner"));
    DebugHelper = CreateDefaultSubobject<UGoapDebugHelper>(TEXT("DebugHelper"));

    CurrentAction = nullptr;
    CurrentGoal = nullptr;
    ReplanInterval = 1.0f; // this could be dynamic (e.g. according to how far away the AI is from other players)
    ReplanCooldown = 0.0f;

    bDebugGoapPlanning = true;
}

void AGoapShooterAIControllerBase::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the GOAP planner with actions and goals
    GoapConfig->InitializeGoapPlanner(GoapPlanner);
    
    // Initialize the debug helper
    if (DebugHelper)
    {
        DebugHelper->Initialize(GoapPlanner);
    }
}

TMap<FString, FGoapValue> AGoapShooterAIControllerBase::CalculateWorldState_Implementation()
{
    // to be implemented in sub class
    return TMap<FString, FGoapValue>();
}

UExecutableAction* AGoapShooterAIControllerBase::CreateExecutableAction(UGoapAction* GoapAction)
{
    if (!GoapAction)
    {
        return nullptr;
    }
    
    UExecutableAction* ExecutableAction = GoapConfig->GetExecutableAction(GoapAction);
    
    if (ExecutableAction)
    {
        UE_LOG(LogTemp, Display, TEXT("Creating executable action: %s"), *GoapAction->GetName());
        ExecutableAction->Initialize(this, GoapAction);
        ExecutableAction->OnActionCompleted.AddDynamic(this, &AGoapShooterAIControllerBase::OnActionCompleted);
    }
    
    return ExecutableAction;
}

void AGoapShooterAIControllerBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Decrease replan cooldown
    ReplanCooldown -= DeltaTime;
    
    // Check if we need to replan
    if (CurrentAction == nullptr || IsIdle() || ReplanCooldown <= 0.0f)
    {
        // Calculate current world state
        GoapPlanner->FullUpdateWorldState(CalculateWorldState());
        
        // Select the best goal based on current world state
        UGoapGoal* NewGoal = GoapPlanner->SelectBestGoal();
        
        // Update current goal
        CurrentGoal = NewGoal;
        
        // Plan the next action
        if (CurrentGoal)
        {
            UGoapAction* NewAction = GoapPlanner->PlanNextAction(NewGoal);
            
            if (NewAction != CurrentAction && CurrentExecutableAction)
            {
                AbortCurrentAction();
                CurrentExecutableAction = nullptr;
            }
            
            CurrentAction = NewAction;
        }
        else
        {
            if (CurrentAction && CurrentExecutableAction)
            {
                AbortCurrentAction();
                CurrentExecutableAction = nullptr;
            }
            
            CurrentAction = nullptr;
        }
        
        // Reset replan cooldown
        ReplanCooldown = ReplanInterval;
    }
    
    // Execute current action if available
    if (CurrentAction)
    {
        ExecuteCurrentAction(DeltaTime);
    } 
    else 
    {
        // TODO choose some default action and execute it
        UE_LOG(LogTemp, Warning, TEXT("No action to execute"));
    }

    // Update debug info if debugging is enabled
    if (bDebugGoapPlanning && DebugHelper)
    {
        DebugHelper->UpdateCurrentActionAndCurrentGoal(CurrentAction, CurrentGoal);
        DebugHelper->UpdateActionDebugInfo(CalculateWorldState());
    }
}

bool AGoapShooterAIControllerBase::IsIdle()
{
    return CurrentExecutableAction == nullptr;
}

void AGoapShooterAIControllerBase::ExecuteCurrentAction(float DeltaTime)
{
    if (!CurrentAction)
    {
        UE_LOG(LogTemp, Warning, TEXT("No action to execute"));
        return;
    }
    
    if (!CurrentExecutableAction)
    {
        CurrentExecutableAction = CreateExecutableAction(CurrentAction);
    }
    else 
    {
        // the current action has its own tick that we call manually in the controller tick
        // TODO: maybe refactor the ExecutableAction concept to improve readability
        CurrentExecutableAction->TickAction(DeltaTime);
    }
}

bool AGoapShooterAIControllerBase::AbortCurrentAction()
{
    // Check if we have a current executable action
    if (!CurrentExecutableAction)
    {
        UE_LOG(LogTemp, Warning, TEXT("No executable action to abort"));
        return false;
    }
    
    // Check if the action is already complete
    if (CurrentExecutableAction->IsActionComplete())
    {
        UE_LOG(LogTemp, Warning, TEXT("Action is already complete, no need to abort"));
        return false;
    }
    
    // Abort the action - this will trigger OnActionCompleted delegate
    UE_LOG(LogTemp, Display, TEXT("Aborting action: %s"), *CurrentExecutableAction->GetActionName());
    CurrentExecutableAction->AbortAction();
    
    // No need to manually clear references or force replanning here
    // The OnActionCompleted callback will handle that
    
    return true;
}

void AGoapShooterAIControllerBase::OnActionCompleted(UExecutableAction* Action, bool Success)
{
    if (!Action)
    {
        return;
    }
    
    //UE_LOG(LogTemp, Display, TEXT("Action completed: %s, Success: %s"), 
    //    *Action->GetActionName(), Success ? TEXT("True") : TEXT("False"));
        
    // Unbind from the action's completion delegate
    CurrentExecutableAction->OnActionCompleted.RemoveDynamic(this, &AGoapShooterAIControllerBase::OnActionCompleted);

    // Clear the current executable action if it's the one that completed
    if (CurrentExecutableAction == Action)
    {   
        CurrentExecutableAction = nullptr;
        CurrentAction = nullptr;
    }

    // Force a replan on the next tick
    ReplanCooldown = 0.0f;
}