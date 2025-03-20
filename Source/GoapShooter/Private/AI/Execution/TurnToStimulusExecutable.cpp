#include "AI/Execution/TurnToStimulusExecutable.h"
#include "Controllers/GoapShooterAIController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

UTurnToStimulusExecutable::UTurnToStimulusExecutable()
{
    TargetLocation = FVector::ZeroVector;
    bHasReachedTarget = false;
    FocusTime = 0.0f;
    bHasStarted = false;
}

bool UTurnToStimulusExecutable::StartExecution()
{
    if (!Super::StartExecution())
    {
        return false;
    }
    
    AGoapShooterAIController* AIController = Cast<AGoapShooterAIController>(OwnerController);
    if (AIController)
    {
        FPerceptionData MostInterestingPerception = AIController->GetMostInterestingPerception();
        TargetLocation = MostInterestingPerception.LastPerceivedLocation;

        UE_LOG(LogTemp, Log, TEXT("TurnToStimulusExecutable: TargetLocation=%s"), *TargetLocation.ToString());
    }
    
    if (TargetLocation.IsZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("TurnToStimulusExecutable: No valid target location provided"));
        FailAction();
        return false;
    }
    
    bHasStarted = true;
    
    return true;
}

void UTurnToStimulusExecutable::TickAction(float DeltaTime)
{
    Super::TickAction(DeltaTime);
    
    if (!bHasStarted && !Super::bIsComplete)
    {
        StartExecution();
    }

    // If we haven't reached the target rotation yet
    if (!bHasReachedTarget)
    {
        // Calculate the target rotation
        FRotator TargetRotation = CalculateTargetRotation();
        
        // Get the current rotation
        FRotator CurrentRotation = OwnerController->GetPawn()->GetActorRotation();
        
        // Interpolate towards the target rotation
        FRotator NewRotation = FMath::RInterpTo(
            CurrentRotation,
            TargetRotation,
            DeltaTime,
            TurnSpeed / 10.0f // Adjust speed for smoother rotation
        );
        
        // Apply the new rotation
        OwnerController->GetPawn()->SetActorRotation(NewRotation);
        
        // Check if we've reached the target rotation
        if (HasReachedTargetRotation())
        {
            bHasReachedTarget = true;
            FocusTime = 0.0f;
        }
    }
    else
    {
        // We've reached the target rotation, now maintain focus for the specified duration
        FocusTime += DeltaTime;
        
        if (FocusTime >= FocusDuration)
        {
            // Action is complete after focusing for the required duration
            CompleteAction();
        }
    }
}

FRotator UTurnToStimulusExecutable::CalculateTargetRotation() const
{   
    // Get the pawn and its location
    APawn* OwnerPawn = OwnerController->GetPawn();
    FVector PawnLocation = OwnerPawn->GetActorLocation();
    
    // Get the direction vector from pawn to target
    FVector DirectionToTarget = TargetLocation - PawnLocation;
    DirectionToTarget.Z = 0.0f; // Ignore height difference
    DirectionToTarget.Normalize(); // Normalize to get unit vector
    
    // Get the pawn's forward vector (the direction it's currently facing)
    FVector PawnForward = OwnerPawn->GetActorForwardVector();
    PawnForward.Z = 0.0f; // Ignore height component
    PawnForward.Normalize();
    
    // Calculate the rotation that aligns the forward vector with the target direction
    FRotator TargetRotation = DirectionToTarget.Rotation();
    
    // Keep current pitch and roll, only change yaw
    FRotator CurrentRotation = OwnerPawn->GetActorRotation();
    
    return FRotator(CurrentRotation.Pitch, TargetRotation.Yaw, CurrentRotation.Roll);
}

bool UTurnToStimulusExecutable::HasReachedTargetRotation() const
{
    // Get the current rotation
    FRotator CurrentRotation = OwnerController->GetPawn()->GetActorRotation();
    
    // Calculate the target rotation
    FRotator TargetRotation = CalculateTargetRotation();
    
    // Check if we're close enough to the target rotation
    float AngleDifference = FMath::Abs(CurrentRotation.Yaw - TargetRotation.Yaw);
    if (AngleDifference > 180.0f)
    {
        AngleDifference = 360.0f - AngleDifference;
    }
    
    // Consider the rotation reached if we're within 5 degrees
    return AngleDifference < 5.0f;
}
