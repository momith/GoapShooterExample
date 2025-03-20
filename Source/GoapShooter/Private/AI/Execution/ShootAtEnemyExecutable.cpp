#include "AI/Execution/ShootAtEnemyExecutable.h"
#include "Controllers/GoapShooterAIControllerBase.h"
#include "Controllers/GoapShooterAIController.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "Characters/GoapShooterCharacter.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"

UShootAtEnemyExecutable::UShootAtEnemyExecutable()
{
    // Set default values
    MaxShootSway = 3.0f;
    ShootInterval = 1.0f;
    bHasStarted = false;
    bEnemyKilled = false;
}

bool UShootAtEnemyExecutable::StartExecution()
{
    Super::StartExecution();
    
    AGoapShooterAIController* AIController = Cast<AGoapShooterAIController>(OwnerController);
    
    // Get the current enemy from the controller
    CurrentEnemy = AIController->GetCurrentEnemy();
    if (!CurrentEnemy)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: No enemy to shoot at"), *GetActionName());
        FailAction();
        return false;
    }
    
    // Check if the enemy is already dead
    if (CurrentEnemy->IsDead())
    {
        UE_LOG(LogTemp, Display, TEXT("%s: Enemy is already dead"), *GetActionName());
        CompleteAction();
        return true;
    }
    
    // Set a timer to shoot after a short delay
    UE_LOG(LogTemp, Display, TEXT("%s: Setting timer to shoot in %f seconds"), *GetActionName(), ShootInterval);
    GetWorld()->GetTimerManager().SetTimer(
        ShootTimerHandle,
        this,
        &UShootAtEnemyExecutable::ShootNow,
        ShootInterval,
        false
    );
    
    // Mark as started
    bHasStarted = true;
    
    return true;
}

void UShootAtEnemyExecutable::TickAction(float DeltaTime)
{
    // If we haven't started yet, try to start
    if (!bHasStarted && !Super::bIsComplete)
    {
        StartExecution();
        return;
    }
    
    // Check if the enemy is still valid
    if (CurrentEnemy && CurrentEnemy->IsDead() && !Super::bIsComplete)
    {
        UE_LOG(LogTemp, Display, TEXT("%s: Enemy died during execution"), *GetActionName());
        if (GetWorld()->GetTimerManager().IsTimerActive(ShootTimerHandle))
        {
            GetWorld()->GetTimerManager().ClearTimer(ShootTimerHandle);
        }
        CompleteAction();
    }
}

bool UShootAtEnemyExecutable::IsActionComplete() const
{
    return Super::bIsComplete;
}

void UShootAtEnemyExecutable::AbortAction()
{
    // Clear the timer if it's active
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(ShootTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(ShootTimerHandle);
    }
    
    // Call the base class implementation
    Super::AbortAction();
}

void UShootAtEnemyExecutable::ShootNow()
{
    UE_LOG(LogTemp, Display, TEXT("%s: Shooting now"), *GetActionName());
    
    // Make sure we have a current enemy and a controlled pawn
    if (!CurrentEnemy || !OwnerController || !OwnerController->GetPawn())
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Cannot shoot - Missing enemy or controller"), *GetActionName());
        FailAction();
        return;
    }
    
    // Get our character
    AGoapShooterCharacter* OurCharacter = Cast<AGoapShooterCharacter>(OwnerController->GetPawn());
    if (!OurCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Cannot shoot - Invalid character"), *GetActionName());
        FailAction();
        return;
    }
    
    // Try to shoot at the enemy with random sway
    UE_LOG(LogTemp, Display, TEXT("%s: Attempting to shoot at enemy %s"), *GetActionName(), *CurrentEnemy->GetName());
    bool bEnemyDead = OurCharacter->ShootAt(CurrentEnemy, MaxShootSway);
    UE_LOG(LogTemp, Display, TEXT("%s: Shot result - Enemy dead: %s"), *GetActionName(), bEnemyDead ? TEXT("Yes") : TEXT("No"));
    
    // Update the controller's knowledge of enemy status
    AGoapShooterAIController* AIController = Cast<AGoapShooterAIController>(OwnerController);
    if (AIController && bEnemyDead)
    {
        AIController->OnCurrentEnemyKilled();
    }
    
    // Complete the action regardless of whether we killed the enemy
    // The action is considered successful if we took the shot
    CompleteAction();
}
