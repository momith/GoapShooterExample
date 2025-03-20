#include "AI/Execution/GoToRandomLocationExecutable.h"
#include "Controllers/GoapShooterAIControllerBase.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "NavigationSystem.h"
#include "AIController.h"

UGoToRandomLocationExecutable::UGoToRandomLocationExecutable()
{
    // Set default values
    MaxSearchRadius = 2000.0f;
    MinSearchRadius = 500.0f;
    MaxLocationFindingAttempts = 5;
    AcceptanceRadius = 50.0f;
    bHasStarted = false;
}

bool UGoToRandomLocationExecutable::StartExecution()
{
    Super::StartExecution();

    if (!FindRandomLocation(TargetLocation))
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Failed to find a random location"), *GetActionName());
        FailAction();
        return false;
    }
    
    MoveToLocation(TargetLocation);
    
    bHasStarted = true;
    
    return true;
}

void UGoToRandomLocationExecutable::TickAction(float DeltaTime)
{
    if (!bHasStarted && !Super::bIsComplete)
    {
        StartExecution();
    }
}

bool UGoToRandomLocationExecutable::IsActionComplete() const
{
    return Super::bIsComplete;
}

void UGoToRandomLocationExecutable::AbortAction()
{
    AGoapShooterAIControllerBase* AIController = Cast<AGoapShooterAIControllerBase>(OwnerController);
    if (AIController)
    {
        AIController->StopMovement();
    }
    
    Super::AbortAction();
}

bool UGoToRandomLocationExecutable::FindRandomLocation(FVector& OutLocation)
{
    AGoapShooterAIControllerBase* AIController = Cast<AGoapShooterAIControllerBase>(OwnerController);
    APawn* ControlledPawn = AIController->GetPawn();
    
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(ControlledPawn->GetWorld());
    if (!NavSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Invalid navigation system"), *GetActionName());
        return false;
    }
    
    // Try to find a random point
    FNavLocation RandomLocation;
    bool bFound = false;
    
    // Try multiple times to find a valid location
    for (int32 Attempt = 0; Attempt < MaxLocationFindingAttempts; ++Attempt)
    {
        // Generate a random radius between min and max
        float RandomRadius = FMath::RandRange(MinSearchRadius, MaxSearchRadius);
        
        // Find a random point in navigable space
        bFound = NavSystem->GetRandomReachablePointInRadius(
            ControlledPawn->GetActorLocation(),
            RandomRadius,
            RandomLocation);
        
        if (bFound)
        {
            // Found a valid location
            OutLocation = RandomLocation.Location;
            return true;
        }
    }
    
    // Failed to find a valid location after all attempts
    UE_LOG(LogTemp, Warning, TEXT("%s: Failed to find a random location after %d attempts"), 
        *GetActionName(), MaxLocationFindingAttempts);
    return false;
}

void UGoToRandomLocationExecutable::MoveToLocation(const FVector& Location)
{
    // Get the AI controller
    AGoapShooterAIControllerBase* AIController = Cast<AGoapShooterAIControllerBase>(OwnerController);
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Invalid AI controller"), *GetActionName());
        FailAction();
        return;
    }
    
    // Set up the move request
    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(Location);
    MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
    MoveRequest.SetReachTestIncludesAgentRadius(true);
    
    // Start the move
    FPathFollowingRequestResult Result = AIController->MoveTo(MoveRequest);
    
    if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
    {
        // Store the request ID for later reference
        CurrentMoveRequestID = Result.MoveId;
        
        // Register for move completion notification
        AIController->ReceiveMoveCompleted.AddDynamic(this, &UGoToRandomLocationExecutable::OnMoveCompleted);
    }
    else
    {
        // Failed to start movement
        UE_LOG(LogTemp, Warning, TEXT("%s: Failed to start movement to location %s"), 
            *GetActionName(), *Location.ToString());
        FailAction();
    }
}

void UGoToRandomLocationExecutable::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
    // Check if this is the completion of our current request
    if (RequestID != CurrentMoveRequestID)
    {
        return;
    }
    
    // Get the AI controller
    AGoapShooterAIControllerBase* AIController = Cast<AGoapShooterAIControllerBase>(OwnerController);
    if (AIController)
    {
        // Unregister from move completion notification
        AIController->ReceiveMoveCompleted.RemoveDynamic(this, &UGoToRandomLocationExecutable::OnMoveCompleted);
    }
    
    // Check the result
    if (Result == EPathFollowingResult::Success)
    {
        // Successfully reached the target location
        CompleteAction();
    }
    else
    {
        // Failed to reach the target location
        UE_LOG(LogTemp, Warning, TEXT("%s: Failed to reach target location, result: %d"), 
            *GetActionName(), static_cast<int32>(Result));
        FailAction();
    }
}
