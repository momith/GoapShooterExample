#include "AI/Execution/LeftRightDodgeExecutableAction.h"
#include "Controllers/GoapShooterAIController.h"
#include "DrawDebugHelpers.h"
#include "Characters/GoapShooterCharacter.h"
#include "Components/PerceptionMemoryComponent.h"

ULeftRightDodgeExecutableAction::ULeftRightDodgeExecutableAction()
{
    AcceptanceRadius = 3.0f;
}

FVector ULeftRightDodgeExecutableAction::CalculateTargetLocation()
{   
    // Get the AI's current location
    FVector AILocation = OwnerController->GetPawn()->GetActorLocation();
    
    // Get threat location (enemy, last known position, or stimulus)
    FVector ThreatLocation = AILocation + OwnerController->GetPawn()->GetActorForwardVector() * 100.0f; // Default forward
    
    // Try to get a better threat location
    AGoapShooterAIController* GoapController = Cast<AGoapShooterAIController>(OwnerController);
    if (GoapController)
    {
        // Check for current enemy
        AActor* CurrentEnemy = GoapController->GetPerceptionMemoryComponent()->GetMostInterestingPerceivedEnemy();
        if (CurrentEnemy)
        {
            ThreatLocation = CurrentEnemy->GetActorLocation();
        }
        // If no current enemy but we have a last seen location
        else if (!GoapController->GetPerceptionMemoryComponent()->GetLastSeenEnemyLocation().IsZero())
        {
            ThreatLocation = GoapController->GetPerceptionMemoryComponent()->GetLastSeenEnemyLocation();
        }
    }
    
    // Calculate direction from AI to threat
    FVector DirectionToThreat = (ThreatLocation - AILocation).GetSafeNormal();
    
    // Calculate perpendicular direction (cross product with up vector)
    FVector PerpDirection = FVector::CrossProduct(DirectionToThreat, FVector::UpVector).GetSafeNormal();
    
    // Randomly choose left or right
    if (FMath::RandBool())
    {
        PerpDirection = -PerpDirection; // Flip direction for "left" instead of "right"
    }
    
    // Determine dodge distance (random between min and max)
    float DodgeDistance = FMath::RandRange(MinDodgeDistance, MaxDodgeDistance);
    
    // Calculate target location
    FVector DodgeTargetLocation = AILocation + (PerpDirection * DodgeDistance);
    
    return DodgeTargetLocation;
}
