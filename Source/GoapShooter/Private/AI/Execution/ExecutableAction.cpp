#include "AI/Execution/ExecutableAction.h"
#include "Controllers/GoapShooterAIControllerBase.h"
#include "AI/GOAP/Actions/GoapAction.h"

UExecutableAction::UExecutableAction()
{
    OwnerController = nullptr;
    GoapAction = nullptr;
    bIsExecuting = false;
    bIsComplete = false;
    bWasSuccessful = false;
    ExecutionTime = 0.0f;
}

void UExecutableAction::Initialize(AGoapShooterAIControllerBase* InController, UGoapAction* InGoapAction)
{
    OwnerController = InController;
    GoapAction = InGoapAction;
    bIsExecuting = false;
    bIsComplete = false;
    bWasSuccessful = false;
    ExecutionTime = 0.0f;
}

bool UExecutableAction::StartExecution()
{
    if (!OwnerController || !GoapAction)
    {
        UE_LOG(LogTemp, Error, TEXT("ExecutableAction::StartExecution: Missing controller or GOAP action"));
        return false;
    }
    
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
}

FString UExecutableAction::GetActionName() const
{
    if (GoapAction)
    {
        return GoapAction->GetName();
    }
    
    return TEXT("Unknown");
}
