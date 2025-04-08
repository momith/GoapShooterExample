#pragma once

#include "CoreMinimal.h"
#include "Characters/GoapShooterCharacter.h"
#include "InputActionValue.h"
#include "GoapShooterPlayerCharacter.generated.h"

/**
 * Player-controlled character class with first-person camera setup
 */
UCLASS(config=Game)
class GOAPSHOOTER_API AGoapShooterPlayerCharacter : public AGoapShooterCharacter
{
    GENERATED_BODY()

public:
    AGoapShooterPlayerCharacter();
    
    virtual void Tick(float DeltaTime) override;
    
    void Shoot();
    
    virtual void Die(AActor* DamageCauser) override;
    
    /** Called for movement input */
    void Move(const FInputActionValue& Value);

    /** Called for looking input */
    void Look(const FInputActionValue& Value);

    /** MappingContext */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputMappingContext* DefaultMappingContext;

    /** Jump Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* JumpAction;

    /** Move Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* MoveAction;

    /** Look Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* LookAction;

    /** Shoot Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* ShootAction;

protected:
    virtual void BeginPlay() override;

    /** First person camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    class UCameraComponent* FirstPersonCameraComponent;

    /** Called to bind functionality to input */
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
