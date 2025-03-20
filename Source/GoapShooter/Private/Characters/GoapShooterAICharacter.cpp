#include "Characters/GoapShooterAICharacter.h"

AGoapShooterAICharacter::AGoapShooterAICharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
}

void AGoapShooterAICharacter::BeginPlay()
{
    Super::BeginPlay();
}

void AGoapShooterAICharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
