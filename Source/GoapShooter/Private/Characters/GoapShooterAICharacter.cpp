#include "Characters/GoapShooterAICharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Controllers/GoapShooterAIController.h"

AGoapShooterAICharacter::AGoapShooterAICharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;

    AIControllerClass = AGoapShooterAIController::StaticClass();

    GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AGoapShooterAICharacter::BeginPlay()
{
    Super::BeginPlay();
}

void AGoapShooterAICharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
