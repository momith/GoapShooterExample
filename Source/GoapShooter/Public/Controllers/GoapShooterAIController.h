#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/GOAP/Interfaces/GoapWorldStateProviderInterface.h"
#include "GoapShooterAIController.generated.h"

/**
 * AI Controller that uses GOAP for decision making. One GOAP planner for actions controlling the lower body
 * (movement) and one GOAP planner for actions controlling the upper body (hands).
 */
UCLASS()
class GOAPSHOOTER_API AGoapShooterAIController : public AAIController, public IGoapWorldStateProviderInterface
{
    GENERATED_BODY()

public:
    AGoapShooterAIController(const FObjectInitializer& ObjectInitializer);

// Attributes
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FieldOfViewAngle = 140.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinSecondsRotateTowardsLastSeenLocation = 8.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSecondsRotateTowardsLastSeenLocation = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RandSecondsRotateTowardsLastSeenLocation = 10.0f;

    UFUNCTION(BlueprintCallable)
    class UAISenseConfig_Sight* GetSightConfig() const { return SightConfig; }

    UFUNCTION(BlueprintCallable)
    class UAISenseConfig_Hearing* GetHearingConfig() const { return HearingConfig; }

    UFUNCTION(BlueprintCallable)
    class UAISenseConfig_Damage* GetDamageConfig() const { return DamageConfig; }
    
// Components

    UFUNCTION(BlueprintCallable)
    class UGoapShooterPlanningComponent* GetMovementPlanningComponent() const { return MovementPlanningComponent; }
    
    UFUNCTION(BlueprintCallable)
    class UGoapShooterPlanningComponent* GetCombatPlanningComponent() const { return CombatPlanningComponent; }

    UFUNCTION(BlueprintCallable)
    class UFlankingComponent* GetFlankingComponent() const { return FlankingComponent; }
    
    UFUNCTION(BlueprintCallable)
    class UCoverComponent* GetCoverComponent() const { return CoverComponent; }
    
    UFUNCTION(BlueprintCallable)
    class UPerceptionMemoryComponent* GetPerceptionMemoryComponent() const { return PerceptionMemoryComponent; }
    
// Methods
    
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    /** Implementation of interface: Provide the current world state for GOAP planning */
    virtual TMap<FString, FGoapValue> CalculateWorldState_Implementation() override;

    /**
     * GOAP actions are usually executed until they are completed and a new GOAP planning is executed.
     * But in case of significant world state changes, it makes sense to force a replanning and abort the current action.
     */
    void OnSignificantWorldStateChange();

    /** Check if any of the current executed actions are related to the currently most interesting actor/enemy */
    bool IsAnyCurrentExecutableActionRelatedToMostInterestingActor();

    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    
private:

// Attributes

    UPROPERTY()
    TObjectPtr<class UAISenseConfig_Sight> SightConfig;
    UPROPERTY()
    TObjectPtr<class UAISenseConfig_Hearing> HearingConfig;
    UPROPERTY()
    TObjectPtr<class UAISenseConfig_Damage> DamageConfig;
    
// Components

    /** Planning for movement (lower body) */
    UPROPERTY()
    class UGoapShooterPlanningComponent* MovementPlanningComponent;
    
    /** Planning for combat (upper body) */
    UPROPERTY()
    class UGoapShooterPlanningComponent* CombatPlanningComponent;

    UPROPERTY()
    class UFlankingComponent* FlankingComponent;
    
    UPROPERTY()
    class UCoverComponent* CoverComponent;
    
    UPROPERTY()
    class UPerceptionMemoryComponent* PerceptionMemoryComponent;
    
// Methods
    
    void RotatePawnTowardsEnemy(float DeltaTime);
    void RotatePawnTowardsLocation(FVector Location, float DeltaTime);
    
};
