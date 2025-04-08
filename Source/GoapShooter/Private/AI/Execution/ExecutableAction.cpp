#include "AI/Execution/ExecutableAction.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "Components/PerceptionMemoryComponent.h"
#include "Controllers/GoapShooterAIController.h"

UExecutableAction::UExecutableAction()
{
    OwnerController = nullptr;
    GoapAction = nullptr;
    bIsExecuting = false;
    bIsComplete = false;
    bWasSuccessful = false;
    ExecutionTime = 0.0f;

    MostInterestingActorWhenActionStarted = nullptr;
}

void UExecutableAction::Initialize(AGoapShooterAIController* InController, UGoapAction* InGoapAction)
{
    OwnerController = InController;
    GoapAction = InGoapAction;
    bIsExecuting = false;
    bIsComplete = false;
    bWasSuccessful = false;
    ExecutionTime = 0.0f;

    MostInterestingActorWhenActionStarted = InController->GetPerceptionMemoryComponent()->GetMostInterestingPerceivedEnemy();
}

bool UExecutableAction::StartExecution()
{   
    bIsExecuting = true;
    bIsComplete = false;
    bWasSuccessful = false;
    ExecutionTime = 0.0f;
    
    UE_LOG(LogTemp, Display, TEXT("Starting execution of action: %s"), *GetActionName());
    
    return true;
}

void UExecutableAction::TickAction(float DeltaTime)
{
    if (!bIsExecuting || bIsComplete)
    {
        return;
    }
    
    ExecutionTime += DeltaTime;
}

bool UExecutableAction::IsActionComplete() const
{
    return bIsComplete;
}

void UExecutableAction::AbortAction()
{
    if (!bIsExecuting || bIsComplete)
    {
        return;
    }
    
    bIsExecuting = false;
    bIsComplete = true;
    bWasSuccessful = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Aborting action: %s"), *GetActionName());
    
    // Notify listeners that the action was aborted
    OnActionCompleted.Broadcast(this, false);
    Internal_OnActionCompleted(false);
}

void UExecutableAction::CompleteAction()
{
    if (!bIsExecuting || bIsComplete)
    {
        return;
    }
    
    bIsExecuting = false;
    bIsComplete = true;
    bWasSuccessful = true;
    
    UE_LOG(LogTemp, Display, TEXT("Action completed successfully: %s (Time: %.2f)"), *GetActionName(), ExecutionTime);
    
    // Notify listeners that the action was completed successfully
    OnActionCompleted.Broadcast(this, true);
    Internal_OnActionCompleted(true);
}

void UExecutableAction::FailAction()
{
    if (!bIsExecuting || bIsComplete)
    {
        return;
    }
    
    bIsExecuting = false;
    bIsComplete = true;
    bWasSuccessful = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Action failed: %s (Time: %.2f)"), *GetActionName(), ExecutionTime);
    
    // Notify listeners that the action failed
    OnActionCompleted.Broadcast(this, false);
    Internal_OnActionCompleted(false);
}

FString UExecutableAction::GetActionName() const
{
    if (GoapAction)
    {
        return GoapAction->GetName();
    }
    
    return TEXT("Unknown");
}

void UExecutableAction::Internal_OnActionCompleted(bool bSuccess)
{
    //
}
