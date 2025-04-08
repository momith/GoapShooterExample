#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CoverComponent.generated.h"

/**
 * This component of the AI controller is responsible for logic related to finding cover.
 * Currently, it provides only the best scored cover point.
 * It might make sense to track multiple cover points instead, e.g. for scoring flank paths.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GOAPSHOOTER_API UCoverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCoverComponent();

protected:
	virtual void BeginPlay() override;

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDebugDrawCoverPoints = false;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UEnvQuery* EqsQueryForCoverLocations;

	/** Radius for considering cover points nearby */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoverRadiusConsideredAsNearby = 400.0f;

	/** Radius for considering cover points in cover */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoverRadiusConsideredAsInCover = 40.0f;
    
	/** Search radius for finding cover points around the AI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoverSearchRadius = 1000.0f;

	/** Minimum score for a cover point to be considered viable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinimumCoverScore = 0.5f;

	/** Returns the current best cover location */
	FVector GetBestCoverLocation();

	/** Visualize the current best cover location */
	void VisualizeBestCover();

	/** Returns true if the any cover is nearby */
	bool IsCoverNearby();
	/** Returns true if we are in cover location */
	bool IsInCover();

	bool IsInitialized() const { return bIsInitialized; }
	
private:
	UPROPERTY()
	class AGoapShooterAIController* AIController;

	bool bIsInitialized = false;
	
	TArray<FVector> CurrentCoverLocations;
	FVector CachedBestCoverLocation = FVector::ZeroVector;

	UPROPERTY()
	FTimerHandle CoverUpdateTimerHandle;
	UFUNCTION()
	void UpdateCurrentBestCover();

	bool bIsWaitingForQueryToFinish = false;

	void OnCoverLocationsQueryFinished(TSharedPtr<struct FEnvQueryResult> Result);
	
};
