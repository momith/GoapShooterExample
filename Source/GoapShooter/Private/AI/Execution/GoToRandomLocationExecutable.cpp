#include "AI/Execution/GoToRandomLocationExecutable.h"
#include "NavigationSystem.h"
#include "GoapShooterAIController.h"

UGoToRandomLocationExecutable::UGoToRandomLocationExecutable()
{
    MaxSearchRadius = 2000.0f;
    MinSearchRadius = 500.0f;
    MaxLocationFindingAttempts = 5;

    bIsRelatedToMostInterestingActor = false;
}

FVector UGoToRandomLocationExecutable::CalculateTargetLocation()
{
    FVector CalculatedTargetLocation;
    if (FindRandomLocation(CalculatedTargetLocation))
    {
        return CalculatedTargetLocation;
    }
    return FVector::ZeroVector;
}

bool UGoToRandomLocationExecutable::FindRandomLocation(FVector& OutLocation)
{
    APawn* ControlledPawn = OwnerController->GetPawn();
    
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
