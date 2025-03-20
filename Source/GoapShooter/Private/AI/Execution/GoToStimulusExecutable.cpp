#include "AI/Execution/GoToStimulusExecutable.h"
#include "Controllers/GoapShooterAIController.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"

UGoToStimulusExecutable::UGoToStimulusExecutable()
{
    TargetLocation = FVector::ZeroVector;
    bHasReachedTarget = false;
    WaitTime = 0.0f;
}

bool UGoToStimulusExecutable::StartExecution()
{
    if (!Super::StartExecution())
    {
        return false;
    }
    
    AGoapShooterAIController* AIController = Cast<AGoapShooterAIController>(OwnerController);
    if (AIController)
    {
        MostInterestingPerception = AIController->GetMostInterestingPerception();
        TargetLocation = MostInterestingPerception.LastPerceivedLocation;
    }
    
    if (MostInterestingPerception.LastPerceivedLocation.IsZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("GoToStimulusExecutable: No valid target location provided"));
        FailAction();
        return false;
    }
    
    // Reset state
    bHasReachedTarget = false;
    WaitTime = 0.0f;
    
    // Start moving to the target
    MoveToTarget();
    
    return true;
}

void UGoToStimulusExecutable::TickAction(float DeltaTime)
{
    Super::TickAction(DeltaTime);
    
    // TODO: this is wrong implemented by LLM ... needs to be fully reworked by me

    if (!OwnerController || !OwnerController->GetPawn())
    {
        FailAction();
        return;
    }
    
    // If we haven't reached the target yet
    if (!bHasReachedTarget)
    {
        // Check if we've reached the target
        if (HasReachedTarget())
        {
            bHasReachedTarget = true;
            WaitTime = 0.0f;
            
            // Markiere den Stimulus als untersucht
            AGoapShooterAIController* AIController = Cast<AGoapShooterAIController>(OwnerController);
            if (AIController)
            {
                AIController->OnStimulusInvestigated(MostInterestingPerception);
            }
        }
    }
    else
    {
        // We've reached the target, now wait for the specified duration
        WaitTime += DeltaTime;
        
        if (WaitTime >= WaitDuration)
        {
            // Action is complete after waiting for the required duration
            CompleteAction();
        }
    }
}

void UGoToStimulusExecutable::MoveToTarget()
{
    AAIController* AIController = Cast<AAIController>(OwnerController);
    if (AIController)
    {
        EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
            TargetLocation,
            AcceptanceRadius,
            true,  // Use pathfinding
            true,  // Project destination to navigation
            false, // Can strafe
            false, // Use block detection
            nullptr,  // Navigation filter class (TSubclassOf<UNavigationQueryFilter>)
            true   // Allow partial path
        );
        
        if (MoveResult == EPathFollowingRequestResult::Failed)
        {
            UE_LOG(LogTemp, Warning, TEXT("GoToStimulusExecutable: Failed to start movement to target"));
            FailAction();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GoToStimulusExecutable: Owner controller is not an AIController"));
        FailAction();
    }
}

bool UGoToStimulusExecutable::HasReachedTarget() const
{
    if (!OwnerController || !OwnerController->GetPawn())
    {
        return false;
    }
    
    // Get the current location
    FVector CurrentLocation = OwnerController->GetPawn()->GetActorLocation();
    
    // Check if we're close enough to the target location (ignoring height)
    FVector CurrentLocationXY = CurrentLocation;
    FVector TargetLocationXY = TargetLocation;
    CurrentLocationXY.Z = 0.0f;
    TargetLocationXY.Z = 0.0f;
    
    float DistanceSquared = FVector::DistSquared(CurrentLocationXY, TargetLocationXY);
    return DistanceSquared <= FMath::Square(AcceptanceRadius);
}
