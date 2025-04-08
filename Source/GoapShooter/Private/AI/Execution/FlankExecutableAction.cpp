#include "AI/Execution/FlankExecutableAction.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Controllers/GoapShooterAIController.h"
#include "DrawDebugHelpers.h"
#include "AI/Navigation/NavigationTypes.h"
#include "Components/FlankingComponent.h"


UFlankExecutableAction::UFlankExecutableAction()
{
    bHasStarted = false;
    bIsRelatedToMostInterestingActor = true; // makes it more difficult to abort this action if the related actor is still the most interesting actor
    CurrentPathIndex = -1;
}

void UFlankExecutableAction::TickAction(float DeltaTime)
{
    if (!bHasStarted && !bIsComplete)
    {
        StartExecution();
    }
}

bool UFlankExecutableAction::StartExecution()
{
    Super::StartExecution();

    AGoapShooterAIController* CastedController = Cast<AGoapShooterAIController>(OwnerController);
    
    bHasStarted = true;

    Path = CastedController->GetFlankingComponent()->GetFlankPath();
    
    if (Path.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: No flank path currently available"), *GetActionName());
        FailAction();
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("%s: Found %d path points"), *GetActionName(), Path.Num());
    
    CastedController->GetFlankingComponent()->OnFlankStarted();

    // Start moving along the path
    MoveToNextPathPoint();

    return true;
}

void UFlankExecutableAction::MoveToNextPathPoint()
{
    // Increment the index
    CurrentPathIndex++;

    // Check if we have a valid path and index
    if (Path.Num() == 0 || CurrentPathIndex >= Path.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("Reached end of path, last point was %d"), CurrentPathIndex - 1);
        // We've reached the end of the path
        CompleteAction();
        return;
    }

    // Get the current target point
    FVector TargetPoint = Path[CurrentPathIndex];
    
    // Make sure we're not already registered for move completion
    OwnerController->ReceiveMoveCompleted.RemoveDynamic(this, &UFlankExecutableAction::OnPathPointReached);
    OwnerController->ReceiveMoveCompleted.AddDynamic(this, &UFlankExecutableAction::OnPathPointReached);
    
    // Set up the move request
    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(TargetPoint);
    //MoveRequest.SetAcceptanceRadius(80.0f); // Increased acceptance radius
    //MoveRequest.SetReachTestIncludesAgentRadius(true);
    //MoveRequest.SetUsePathfinding(true);
    //MoveRequest.SetAllowPartialPath(false); // Allow partial paths
    
    // TODO use MoveTo, return a move ID and use it in OnPathPointReached to check if its related

    // Execute the move
    EPathFollowingRequestResult::Type ResultCode = //OwnerController->MoveTo(MoveRequest).Code;
    OwnerController->MoveToLocation(TargetPoint);

    UE_LOG(LogTemp, Log, TEXT("Flank move request for path point %d"), CurrentPathIndex);

    if (ResultCode == EPathFollowingRequestResult::Failed)
    {
        // EPathFollowingRequestResult::AlreadyAtGoal and RequestSuccessful will result in a callback after which we handle the next point.
        // but this request failed completely. we won't get a callback. therefore don't wait and try the next point
        MoveToNextPathPoint();
    }
}

void UFlankExecutableAction::OnPathPointReached(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
    if (Super::bIsComplete)
    {
        UE_LOG(LogTemp, Error, TEXT("FlankExecutableAction::OnPathPointReached: Action already complete"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s: Path point %d reached with result %d"), 
        *GetActionName(), CurrentPathIndex, (int32)Result);
    
    MoveToNextPathPoint();
}

void UFlankExecutableAction::AbortAction()
{
    // be aware that this can be often called due to significant world state changes (e.g. from doesNotSeeEnemy to seeEnemy) during the flank
    if (OwnerController)
    {
        OwnerController->StopMovement();
    }
    Super::AbortAction();
}

void UFlankExecutableAction::Internal_OnActionCompleted(bool bSuccess)
{
    Super::Internal_OnActionCompleted(bSuccess);
    TellControllerThatFlankHasEnded();
    if (OwnerController)
    {
        // Make sure to unregister from the move completed delegate
        OwnerController->ReceiveMoveCompleted.RemoveDynamic(this, &UFlankExecutableAction::OnPathPointReached);
        //OwnerController->StopMovement();
    }
}

void UFlankExecutableAction::TellControllerThatFlankHasEnded()
{
    Cast<AGoapShooterAIController>(OwnerController)->GetFlankingComponent()->OnFlankEnded();
}
