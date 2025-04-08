#include "AI/Execution/GoToExecutableAction.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "GoapShooterAIController.h"

UGoToExecutableAction::UGoToExecutableAction()
{
    AcceptanceRadius = 5.0f;
    bHasStarted = false;
}

bool UGoToExecutableAction::StartExecution()
{
    Super::StartExecution();

    TargetLocation = CalculateTargetLocation();
    if (TargetLocation.IsZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Failed to calculate target location"), *GetActionName());
        FailAction();
        return false;
    }
    
    MoveToLocation(TargetLocation);
    
    bHasStarted = true;
    
    return true;
}

FVector UGoToExecutableAction::CalculateTargetLocation()
{
    return FVector::ZeroVector;
}

void UGoToExecutableAction::TickAction(float DeltaTime)
{
    if (!bHasStarted && !Super::bIsComplete)
    {
        StartExecution();
    }
}

void UGoToExecutableAction::AbortAction()
{
    if (OwnerController)
    {
        OwnerController->StopMovement();
    }
    
    Super::AbortAction();
}

FAIMoveRequest UGoToExecutableAction::PrepareMoveRequest(const FVector& LocationToMoveTo)
{
    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(LocationToMoveTo);
    MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
    MoveRequest.SetReachTestIncludesAgentRadius(true);
    //MoveRequest.SetReachTestIncludesAgentRadius(true);
    //MoveRequest.SetCanStrafe(true);
    return MoveRequest;
}

void UGoToExecutableAction::MoveToLocation(const FVector& Location)
{
    if (!OwnerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Invalid AI controller"), *GetActionName());
        FailAction();
        return;
    }
    
    // Project the target location onto the navigation mesh to ensure it's reachable
    FVector ProjectedLocation = Location;
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(OwnerController->GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(Location, NavLocation, FVector(500.0f, 500.0f, 500.0f)))
        {
            ProjectedLocation = NavLocation.Location;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("%s: Failed to project location %s to navigation mesh"), 
                *GetActionName(), *Location.ToString());
        }
    }

    FAIMoveRequest MoveRequest = PrepareMoveRequest(ProjectedLocation);

    FPathFollowingRequestResult Result = OwnerController->MoveTo(MoveRequest);
    
    if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
    {
        CurrentMoveRequestID = Result.MoveId;
        
        OwnerController->ReceiveMoveCompleted.AddDynamic(this, &UGoToExecutableAction::OnMoveCompleted);
    }
    else if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        // Already at the goal, no action needed
        CompleteAction();

        // debug draw
        if (OwnerController && OwnerController->GetWorld())
        {
            DrawDebugSphere(
                OwnerController->GetWorld(),
                ProjectedLocation,
                10.0f,
                16,
                FColor::Yellow,
                false,
                5.0f
            );
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Failed to start movement to location %s with result code %d"), 
            *GetActionName(), *ProjectedLocation.ToString(), (int32)Result.Code);
        FailAction();

        // debug draw
        if (OwnerController && OwnerController->GetWorld())
        {
            DrawDebugSphere(
                OwnerController->GetWorld(),
                ProjectedLocation,
                10.0f,
                16,
                FColor::Red,
                false,
                5.0f
            );
        }
    }
}

void UGoToExecutableAction::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
    // Check if this is the completion of our current request
    if (RequestID != CurrentMoveRequestID)
    {
        return;
    }
    
    // Get the AI controller
    if (OwnerController)
    {
        // Unregister from move completion notification
        OwnerController->ReceiveMoveCompleted.RemoveDynamic(this, &UGoToExecutableAction::OnMoveCompleted);
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
