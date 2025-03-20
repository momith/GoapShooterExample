#pragma once

#include "CoreMinimal.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Controllers/GoapShooterAIControllerBase.h"
#include "AI/Utils/PerceptionTypes.h"
#include "GoapShooterAIController.generated.h"

class AGoapShooterCharacter;

UCLASS()
class GOAPSHOOTER_API AGoapShooterAIController : public AGoapShooterAIControllerBase
{
    GENERATED_BODY()

public:
    AGoapShooterAIController(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual TMap<FString, FGoapValue> CalculateWorldState_Implementation() override;
    
    /****** Perception/Memory Functions ****** */

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FieldOfViewAngle = 140.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    bool bVisualizePerception = true;
    void DrawPerceptionVisualization();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PerceptionDataLifetimeInSeconds = 10.0f;

    UPROPERTY()
    UAISenseConfig_Sight* SightConfig;

    UFUNCTION()
    void TargetActorsPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus);

    void AddOrUpdatePerceptionData(AActor* PerceivedActor, FAIStimulus Stimulus);
    void RemovePerceptionData(AActor* UnPerceivedActor, FAIStimulus Stimulus);
    void CleanUpPerceptionMemory();
    void ChooseMostInterestingPerceptionData();

    EPerceptionType DeterminePerceptionType(const FAIStimulus& Stimulus) const;
    void MarkStimulusAsInvestigated();

    FVector GetCurrentStimulusLocation() const;
    bool HasInvestigatedStimulus() const;

    /** Map of perception data for all perceived actors */
    UPROPERTY()
    TMap<AActor*, FPerceptionData> PerceptionMemory;
    
    /** Die aktuell interessanteste Wahrnehmung */
    UPROPERTY()
    FPerceptionData MostInterestingPerception;
    FPerceptionData GetMostInterestingPerception() const { return MostInterestingPerception; }

    UPROPERTY()
    AActor* CurrentEnemy;

    void UpdateCurrentEnemy();
    bool IsActorInFieldOfView(AActor* TargetActor);
    void PickNewEnemyFromMemory();
    bool IsCurrentEnemyStillInClearSight();

    void RotatePawnTowardsEnemy(float DeltaTime);

    bool CanSeeEnemy() const;

    /** Last known location of the enemy. A Zero vector indicates that we dont have a last known location. */
    FVector LastSeenEnemyLocation = FVector::ZeroVector;
    float LastSeenEnemyWorldTime = -FLT_MAX;

    bool bLastEnemyInSightKilled = false;

    AGoapShooterCharacter* GetCurrentEnemy() const;

    void OnCurrentEnemyKilled();
    void OnStimulusInvestigated(const FPerceptionData& Stimulus);
};
