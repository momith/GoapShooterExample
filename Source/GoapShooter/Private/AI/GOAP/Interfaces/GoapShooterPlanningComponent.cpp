#include "AI/GOAP/Interfaces/GoapShooterPlanningComponent.h"
#include "AI/GOAP/Planner/GoapPlannerConfiguration.h"
#include "AI/Execution/ExecutableAction.h"
#include "AI/GOAP/Debug/GoapDebugHelper.h"
#include "AI/GOAP/Interfaces/GoapWorldStateProviderInterface.h"
#include "Controllers/GoapShooterAIController.h"

UGoapShooterPlanningComponent::UGoapShooterPlanningComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bDebugGoapPlanning = true;

    GoapConfig = CreateDefaultSubobject<UGoapPlannerConfiguration>(TEXT("GoapConfig"));
    GoapPlanner = CreateDefaultSubobject<UGoapPlanner>(TEXT("GoapPlanner"));
    GoapDebugHelper = CreateDefaultSubobject<UGoapDebugHelper>(TEXT("GoapDebugHelper"));

    CurrentAction = nullptr;
    CurrentGoal = nullptr;
    ReplanInterval = 30.0f; // this could be dynamic (e.g. according to how far away the AI is from other players)
    ReplanCooldown = 0.0f;

    bIsReplanningForcedForNextTick = false;
}

void UGoapShooterPlanningComponent::OnSignificantWorldStateChange()
{
    // force a replanning
    //ReplanCooldown = 0.0f;
    bIsReplanningForcedForNextTick = true;
}

void UGoapShooterPlanningComponent::BeginPlay()
{
    Super::BeginPlay();
    
    Controller = Cast<AGoapShooterAIController>(GetOwner());
    if (!Controller)
    {
        UE_LOG(LogTemp, Error, TEXT("GoapShooterPlanningComponent: Controller is null"));
    }

    GoapPlanner = NewObject<UGoapPlanner>(this);
    if (ConfigClassToUse)
    {
        GoapConfig = NewObject<UGoapPlannerConfiguration>(this, ConfigClassToUse);
        GoapConfig->InitializeGoapPlanner(GoapPlanner);
    }
    GoapDebugHelper = NewObject<UGoapDebugHelper>(this);
    if (bDebugGoapPlanning)
    {
        GoapDebugHelper->Initialize(GoapPlanner);
    }

    SetComponentTickEnabled(true);
}

void UGoapShooterPlanningComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsValid(Controller) || !IsValid(Controller->GetPawn())) return;
    
    if (!bActivated)
    {
        return;
    }

    if (bIsReplanningForcedForNextTick)
    {
        bIsReplanningForcedForNextTick = false;
        UE_LOG(LogTemp, Error, TEXT("GoapShooterPlanningComponent: Abort current action because of forced replanning"));
        AbortCurrentAction();
    }

    //// Decrease replan cooldown
    //ReplanCooldown -= DeltaTime; // TODO to be refactored: we dont need to use replan interval or cooldown anymore, we react to event OnSignificantWorldStateChange
    
    // Check if we need to replan
    if (CurrentExecutableAction == nullptr) // CurrentAction == nullptr || CurrentExecutableAction == nullptr || ReplanCooldown <= 0.0f
    {
        UE_LOG(LogTemp, Log, TEXT("GoapShooterPlanningComponent: Replanning because CurrentExecutableAction is null. CurrentAction: %s"), 
            CurrentAction ? *CurrentAction->GetName() : TEXT("None"));
        
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
            
            if (NewAction)
            {
                //UE_LOG(LogTemp, Log, TEXT("GoapShooterPlanningComponent: Planned action %s"), *NewAction->GetName());
            }
            else
            {
                //UE_LOG(LogTemp, Error, TEXT("GoapShooterPlanningComponent: No action found fitting the goal %s"), *NewGoal->GetName());
            }

            //if (NewAction != CurrentAction && CurrentExecutableAction)
            //{
            //    AbortCurrentAction();
            //    CurrentExecutableAction = nullptr;
            //}
            
            CurrentAction = NewAction;
        }
        //else
        //{
        //    if (CurrentAction && CurrentExecutableAction)
        //    {
        //        AbortCurrentAction();
        //        CurrentExecutableAction = nullptr;
        //    }
        //    
        //    CurrentAction = nullptr;
        //}
        //
        //// Reset replan cooldown
        //ReplanCooldown = ReplanInterval;
    }
    
    // Execute current action if available
    if (CurrentAction)
    {
        ExecuteCurrentAction(DeltaTime);
    } 
    else 
    {
        // TODO choose some default action and execute it
        
        //UE_LOG(LogTemp, Warning, TEXT("No action to execute"));
    }

    // Update debug info if debugging is enabled
    if (bDebugGoapPlanning && GoapDebugHelper)
    {
        GoapDebugHelper->UpdateCurrentActionAndCurrentGoal(CurrentAction, CurrentGoal);
        GoapDebugHelper->UpdateActionDebugInfo(CalculateWorldState());
    }
}

UExecutableAction* UGoapShooterPlanningComponent::CreateExecutableAction(UGoapAction* GoapAction)
{
    if (!GoapAction)
    {
        UE_LOG(LogTemp, Error, TEXT("GoapShooterPlanningComponent::CreateExecutableAction: GoapAction is null"));
        return nullptr;
    }
    
    UE_LOG(LogTemp, Log, TEXT("GoapShooterPlanningComponent::CreateExecutableAction: Creating executable action for %s"), *GoapAction->GetName());
    
    UExecutableAction* ExecutableAction = GoapConfig->GetExecutableAction(GoapAction);
    
    if (ExecutableAction)
    {
        ExecutableAction->Initialize(Controller, GoapAction);
        ExecutableAction->OnActionCompleted.AddDynamic(this, &UGoapShooterPlanningComponent::OnActionCompleted);
        
        UE_LOG(LogTemp, Log, TEXT("GoapShooterPlanningComponent::CreateExecutableAction: Created executable action %s"), *ExecutableAction->GetActionName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GoapShooterPlanningComponent::CreateExecutableAction: Failed to create executable action for %s"), *GoapAction->GetName());
    }
    
    return ExecutableAction;
}

bool UGoapShooterPlanningComponent::IsIdle()
{
    return CurrentExecutableAction == nullptr;
}

void UGoapShooterPlanningComponent::ExecuteCurrentAction(float DeltaTime)
{
    if (!CurrentAction)
    {
        UE_LOG(LogTemp, Warning, TEXT("No action to execute"));
        return;
    }
    
    if (!CurrentExecutableAction)
    {
        UE_LOG(LogTemp, Log, TEXT("GoapShooterPlanningComponent::ExecuteCurrentAction: Creating executable action for %s"), *CurrentAction->GetName());
        CurrentExecutableAction = CreateExecutableAction(CurrentAction);
    }
    else 
    {
        // the current action has its own tick that we call manually in the component tick
        // TODO: maybe refactor the ExecutableAction concept to improve readability
        CurrentExecutableAction->TickAction(DeltaTime);
    }
}

bool UGoapShooterPlanningComponent::AbortCurrentAction()
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

void UGoapShooterPlanningComponent::OnActionCompleted(UExecutableAction* Action, bool Success)
{
    if (!Action)
    {
        UE_LOG(LogTemp, Error, TEXT("GoapShooterPlanningComponent::OnActionCompleted: Action is null"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("GoapShooterPlanningComponent::OnActionCompleted: Action %s completed with success=%d"), 
        *Action->GetActionName(), Success);

    Action->OnActionCompleted.RemoveDynamic(this, &UGoapShooterPlanningComponent::OnActionCompleted);

    if (CurrentExecutableAction != Action)
    {   
        UE_LOG(LogTemp, Error, TEXT("Action completed, but is not the current one: %s. Current action is: %s"), 
            *Action->GetActionName(), 
            CurrentExecutableAction ? *CurrentExecutableAction->GetActionName() : TEXT("None"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("GoapShooterPlanningComponent::OnActionCompleted: Clearing current action and executable action"));
    CurrentExecutableAction = nullptr;
    CurrentAction = nullptr;

    //// Force a replan on the next tick
    //ReplanCooldown = 0.0f;
}

TMap<FString, FGoapValue> UGoapShooterPlanningComponent::CalculateWorldState()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("GoapShooterPlanningComponent has no owner"));
        return TMap<FString, FGoapValue>();
    }
    
    if (!Owner->Implements<UGoapWorldStateProviderInterface>())
    {
        UE_LOG(LogTemp, Error, TEXT("Owner does not implement IGoapWorldStateProviderInterface"));
        return TMap<FString, FGoapValue>();
    }
    
    return IGoapWorldStateProviderInterface::Execute_CalculateWorldState(Owner);
}
