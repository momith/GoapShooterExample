#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AISightTargetInterface.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "GoapShooterCharacter.generated.h"

UCLASS()
class GOAPSHOOTER_API AGoapShooterCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    /** Constructor with optional object initializer */
    explicit AGoapShooterCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    /** Returns Mesh1P subobject **/
    UFUNCTION(BlueprintCallable, Category = "Character")
    USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

    UPROPERTY()
    UAIPerceptionStimuliSourceComponent* StimuliSourceComponent;
    
    /** Attempts to shoot at a target actor with random accuracy , result returns if enemy is dead */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShootAt(AGoapShooterCharacter* Target, float MaxSway = 5.0f);
    
    /** Check if the character is dead */
    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsDead() const { return Health <= 0.0f; }
    
    /** Apply damage to the character */
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
    virtual void BeginPlay() override;
    
    /** Handle death effects and cleanup */
    virtual void Die();

    /** First person mesh (arms), visible only to the owning player */
    UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
    USkeletalMeshComponent* Mesh1P;
    
    /** Character's health points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0"))
    float Health = 100.0f;
    
    /** Maximum health points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0"))
    float MaxHealth = 100.0f;
    
    /** Base damage inflicted by this character's rifle */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RifleDamage = 20.0f;
    
    /** Maximum distance for rifle shots */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxShootDistance = 5000.0f;
    
    /** How long to display debug lines for shots */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugDrawTime = 2.0f;
};
