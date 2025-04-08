#include "AI/Execution/GoToCoverExecutable.h"
#include "Components/CoverComponent.h"
#include "Controllers/GoapShooterAIController.h"

UGoToCoverExecutable::UGoToCoverExecutable()
{
    AcceptanceRadius = 3.0f;
}

FVector UGoToCoverExecutable::CalculateTargetLocation()
{
    AGoapShooterAIController* AIController = Cast<AGoapShooterAIController>(OwnerController);
    if (AIController)
    {
        return AIController->GetCoverComponent()->GetBestCoverLocation();
    }
    return FVector::ZeroVector;
}