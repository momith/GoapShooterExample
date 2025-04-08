#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "NavigationSystem.h"
#include "EnvQueryTest_PathNearActor.generated.h"

/**
 * EQS Test that checks if the path to a location passes near a specified actor.
 * Uses a context actor (typically provided by EnvQueryContext_FocusActor) and checks
 * if the navigation path from the querier to the test location passes within a certain distance of the actor.
 */
UCLASS()
class GOAPSHOOTER_API UEnvQueryTest_PathNearActor : public UEnvQueryTest
{
	GENERATED_BODY()
	
public:
	UEnvQueryTest_PathNearActor();
	
	/** Context for the actor that we want to check proximity to */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	TSubclassOf<UEnvQueryContext> ActorContext;
	
	/** Distance threshold for considering a path as "near" the actor */
	UPROPERTY(EditDefaultsOnly, Category = "Test", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ProximityThreshold;
	
	/** Whether we want paths that are near (true) or far (false) from the actor */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	bool bWantPathsNearActor;
	
	/** Whether to use 2D (ignore Z) or 3D distance calculations */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	bool bUse2DDistance;
	
	/** Maximum number of path points to check (for performance) */
	UPROPERTY(EditDefaultsOnly, Category = "Test", meta = (ClampMin = "1", UIMin = "1"))
	int32 MaxPathPointsToCheck;
	
	/** Whether to use simple straight-line distance checks or actual navigation path */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	bool bUseSimpleDistanceCheck;
	
protected:
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
	
	/** Helper function to check if a path passes near an actor */
	bool DoesPathPassNearActor(const TArray<FNavPathPoint>& PathPoints, const FVector& ActorLocation, float Threshold, bool bUse2D) const;
};
