#pragma once

#include "CoreMinimal.h"
#include "GoapShooterCharacter.h"
#include "Components/ActorComponent.h"
#include "Utils/PerceptionTypes.h"
#include "PerceptionMemoryComponent.generated.h"

/**
 * The most important component of the AI controller representing the perception memory.
 * Everything related to perception shall be done here.
 * Except for the construction/configuration of PerceptionComponent, this is done by the AI controller itself.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GOAPSHOOTER_API UPerceptionMemoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPerceptionMemoryComponent();

// Attributes

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDebugDrawPerception = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StrongHearingRange = 700.0f;

	/** After perceivement stopped: How long to keep perception data before removing it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SecondsUntilRemovalOfPerceptionData = 10.0f;
	
	UFUNCTION(BlueprintCallable)
	bool IsInitialized() const { return bIsInitialized; }

	/** Gets last known location of the enemy. A Zero vector indicates that we don't have a last known location. */
	UFUNCTION(BlueprintCallable)
	FVector GetLastSeenEnemyLocation() const { return LastSeenEnemyLocation; }
	
	UFUNCTION(BlueprintCallable)
	float GetLastSeenEnemyWorldTime() const { return LastSeenEnemyWorldTime; }
	
// Methods
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable)
	AGoapShooterCharacter* GetMostInterestingPerceivedEnemy();

	UFUNCTION(BlueprintCallable)
	FVector GetEstimatedEnemyLocation();
	
	UFUNCTION(BlueprintCallable)
	FPerceptionData GetMostInterestingPerception();

	void OnStimulusInvestigated(const ::FPerceptionData& Stimulus);
	
// Methods (primarily for GOAP world state retrieval)
	
	bool IsAimedAt();
	bool IsLookingAtStimulus();
	bool CanSeeEnemy();
	bool IsMinThresholdFulfilledForEnemyPerception();
	bool AreAllEnemiesInSightKilled();
	bool HasPerceivedStimulus();
	bool HasInvestigatedStimulus();
	FString GetPerceivedStimulusType();
	FVector GetPerceivedStimulusLocation();
	bool HasHalfStrongEnemyPerception();

	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
private:
	
// Attributes

	/** Map of perception data for all perceived actors */
	UPROPERTY()
	TMap<AActor*, FPerceptionData> PerceptionMemory;

	UPROPERTY()
	AActor* MostInterestingActor;
	
	/** Last known location of the enemy. A Zero vector indicates that we dont have a last known location. */
	FVector LastSeenEnemyLocation = FVector::ZeroVector;
	float LastSeenEnemyWorldTime = -FLT_MAX;
	FVector AIPositionWhenLastSawEnemy = FVector::ZeroVector;

	bool bLastEnemyInSightKilled = false;

	bool bLastTickStrongPerceptionOfMostInterestingActor = false;
	
	bool bIsInitialized = false;
	
	UPROPERTY()
	class AGoapShooterAIController* AIController;
	
// Methods
	
	UFUNCTION()
	void TargetActorsPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus);

	void AddOrUpdatePerceptionData(AActor* PerceivedActor, FAIStimulus Stimulus);
	void StartPerceptionDataExpiration(AActor* UnPerceivedActor, FAIStimulus Stimulus);

	void CleanUpPerceptionMemory();

	void UpdateMostInterestingActor();
	void ChooseMostInterestingPerceptionData();
	
	enum EPerceptionType DeterminePerceptionType(const FAIStimulus& Stimulus);
	
	FInterpCurveFloat GetCurveForHearingAttenuation(FName NoiseTag);

	bool HasStrongEnemyPerception();
	
	UFUNCTION()
	void OnPlayerOrBotKilled(APlayerState* Killer, APlayerState* Victim);
	
	bool HasLineOfSightToActor(AActor* TargetActor);
	bool IsActorInFieldOfView(AActor* TargetActor);
	bool IsControlledPawnInFieldOfViewOfActor(AActor* Actor);
	
	void DrawPerceptionVisualization();
	void DrawStrongHearingRange();

	APawn* GetPawn();
	
};
