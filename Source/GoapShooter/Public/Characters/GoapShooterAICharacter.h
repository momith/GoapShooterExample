#pragma once

#include "CoreMinimal.h"
#include "Characters/GoapShooterCharacter.h"

#include "GoapShooterAICharacter.generated.h"

/**
 * AI-controlled character that uses GOAP for decision making
 */
UCLASS()
class GOAPSHOOTER_API AGoapShooterAICharacter : public AGoapShooterCharacter
{
    GENERATED_BODY()

public:
    AGoapShooterAICharacter(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
};
