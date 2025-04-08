#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlankingComponent.generated.h"

/**
 * This component of the AI controller is responsible for logic related to flanking.
 * Finding a good flank path is expensive. Performance should be monitored.
 * The current implementation could be improved, but performance should be considered.
 * E.g. the current implementation does not check if the path has cover.
 *
 * Note: The flanking does not work properly if RuntimeGeneration=Dynamic is set for the navigation system in the project settings.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GOAPSHOOTER_API UFlankingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlankingComponent();

protected:
	virtual void BeginPlay() override;

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDebugDrawEachPossibleFlankingPath = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDebugDrawExecutingFlankingPath = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDebugDrawFlankAvoidanceBox = false;

	/** EQS query providing an interesting navigable target location for the flank path (somewhere around the enemy) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UEnvQuery* EqsQueryForFlankLocations; // BP asset will be retrieved automatically in the constructor
	/** Cooldown after flank to avoid AI always flanking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlankCoolDown = 15.0f;
	/** At which distance to the enemy shall a flank be considered at all? **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinFlankDistance = 500.0f;
	/** At which distance to the enemy shall a flank be considered at all? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxFlankDistance = 1000.0f;

	/** Get a path the bot could currently take to flank the enemy. Empty if no path exists.  */
	TArray<FVector> GetFlankPath() const { return FlankPath; }

	/** To be called when flank execution is started */
	void OnFlankStarted();
	/** To be called when flank execution is ended */
	void OnFlankEnded();

	/** Shall flanking be a valid option right now (considering cooldown etc.)?  */
	bool IsFlankTargetLocationAvailable();

	bool IsInitialized() const { return bIsInitialized; }
	
private:
	UPROPERTY()
	class AGoapShooterAIController* AIController;

	bool bIsInitialized = false;
	
	/**
	 * Recurrently the system shall try to find a flank target location and then find a valid path towards the target
	 * location while avoiding the direct area between bot and enemy.
	 */
	UPROPERTY()
	FTimerHandle FlankUpdateTimerHandle;
	UFUNCTION()
	void UpdateCurrentBestFlankTargetLocation();
	bool bIsWaitingForFlankQueryToFinish = false;
	void OnFlankLocationsQueryFinished(TSharedPtr<struct FEnvQueryResult> Result);
	bool bIsWaitingForAsyncFlankCheckToFinish = false;
	
	/**
	 * In order to track if the pawn is currently executing a flank.
	 * Can be used to avoid firing unnecessary flank target location requests during this time.
	 */
	bool bIsFlanking = false;

	/** A path the bot could currently take to flank the enemy. Empty if no path exists. */
	TArray<FVector> FlankPath;

	/** The last time when the bot executed a flank */
	float LastExecutedFlankTime = -FLT_MAX; // by default: infinite in the past

	/** A nav modifier collision box which is blocking navigation area between the pawn and the enemy. */
	UPROPERTY()
	AActor* FlankBlockingVolume; // no direct use, use GetOrSpawnFlankBlockingVolume() instead!

	/**
	 * Gets a single reusable instance of a navigation blocking volume/box. 
	 * @return The actor instance.
	 */
	AActor* GetOrSpawnFlankBlockingVolume();

	/**
	 * Finds a flanking path towards the target location while avoiding the center area between the pawn and the enemy.
	 * Needs to be called on the game thread because of using UNavigationSystemV1::FindPathToLocationSynchronously.
	 * @param PathEndLocation The target location (preferably a location next to the enemy calculated by an EQS query).
	 * @param ActualEnemyLocation The enemy location
	 * @return The path as array of locations
	 */
	TArray<FVector> FindFlankPathToLocation(FVector PathEndLocation, FVector ActualEnemyLocation);

	/**
	 * Places the flank blocking volume (FlankBlockingVolume) between the pawn and the enemy.
	 * @param NavModifierSourceLocation The pawn location
	 * @param NavModifierTargetLocation The enemy location
	 */
	void PlaceAndScaleBigNavModifierInCenter(FVector NavModifierSourceLocation, FVector NavModifierTargetLocation);
	
	void DrawDebugPath(TArray<FVector> Path, float Duration);
};
