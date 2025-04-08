#include "AI/Execution/PeekExecutable.h"
#include "Controllers/GoapShooterAIController.h"
#include "DrawDebugHelpers.h"
#include "Components/PerceptionMemoryComponent.h"

UPeekExecutable::UPeekExecutable()
{
    AcceptanceRadius = 20.0f;
}

FVector UPeekExecutable::CalculateTargetLocation()
{   
    // Get the AI's current location
    FVector AILocation = OwnerController->GetPawn()->GetActorLocation();
    
    // Get the last known enemy location
    FVector LastKnownEnemyLocation = AILocation + OwnerController->GetPawn()->GetActorForwardVector() * 100.0f; // Default forward
    
    // Try to get a better enemy location
    AGoapShooterAIController* GoapController = Cast<AGoapShooterAIController>(OwnerController);
    if (GoapController)
    {
        // Get the last seen enemy location
        LastKnownEnemyLocation = GoapController->GetPerceptionMemoryComponent()->GetLastSeenEnemyLocation();
        
        // If we have a valid last seen location
        if (!LastKnownEnemyLocation.IsZero())
        {
            // Get the AI's current position
            FVector CurrentAIPosition = AILocation;
            
            // Calculate direction from AI to last known enemy location
            FVector DirectionToEnemy = (LastKnownEnemyLocation - CurrentAIPosition).GetSafeNormal();
            
            // Calculate perpendicular direction (cross product with up vector)
            FVector PerpDirection = FVector::CrossProduct(DirectionToEnemy, FVector::UpVector).GetSafeNormal();
            
            // Determine which side to peek from based on the AI's current position and cover
            // We need to add a new variable to track the AI's position when it last saw the enemy
            
            // For now, we'll need to modify the GoapShooterAIController to store this information
            // But let's implement a simple version first that picks a side based on the environment
            
            // Cast a ray in both perpendicular directions to see which one has more clearance
            FHitResult HitResultLeft;
            FHitResult HitResultRight;
            
            float RayLength = 300.0f;
            FVector LeftRayEnd = CurrentAIPosition + (-PerpDirection * RayLength);
            FVector RightRayEnd = CurrentAIPosition + (PerpDirection * RayLength);
            
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(OwnerController->GetPawn());
            
            bool bHitLeft = GetWorld()->LineTraceSingleByChannel(
                HitResultLeft, 
                CurrentAIPosition, 
                LeftRayEnd, 
                ECC_Visibility, 
                QueryParams
            );
            
            bool bHitRight = GetWorld()->LineTraceSingleByChannel(
                HitResultRight, 
                CurrentAIPosition, 
                RightRayEnd, 
                ECC_Visibility, 
                QueryParams
            );
            
            // Choose the direction with more clearance
            bool bChooseLeft = false;
            
            if (bHitLeft && bHitRight)
            {
                // Both directions hit something, choose the one with more distance
                bChooseLeft = HitResultLeft.Distance > HitResultRight.Distance;
            }
            else if (bHitLeft)
            {
                // Only left hit something, choose right
                bChooseLeft = false;
            }
            else if (bHitRight)
            {
                // Only right hit something, choose left
                bChooseLeft = true;
            }
            else
            {
                // Neither hit anything, choose randomly
                bChooseLeft = FMath::RandBool();
            }
            
            // Set the peek direction
            FVector PeekDirection = bChooseLeft ? -PerpDirection : PerpDirection;
            
            // Determine peek distance (random between min and max)
            float PeekDistance = FMath::RandRange(MinPeekDistance, MaxPeekDistance);
            
            // Calculate target location
            FVector PeekTargetLocation = CurrentAIPosition + (PeekDirection * PeekDistance);
            
            // Draw debug visuals if needed
            if (GoapController && GoapController->GetPerceptionMemoryComponent()->bDebugDrawPerception)
            {
                DrawDebugLine(
                    GetWorld(),
                    CurrentAIPosition,
                    PeekTargetLocation,
                    FColor::Cyan,
                    false,
                    5.0f,
                    0,
                    3.0f
                );
                
                DrawDebugSphere(
                    GetWorld(),
                    PeekTargetLocation,
                    20.0f,
                    12,
                    FColor::Cyan,
                    false,
                    5.0f
                );
            }
            
            return PeekTargetLocation;
        }
    }
    
    // Fallback if we couldn't determine a good peek location
    return AILocation;
}
