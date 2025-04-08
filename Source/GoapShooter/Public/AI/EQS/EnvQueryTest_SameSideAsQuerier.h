#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_SameSideAsQuerier.generated.h"

/**
 * EQS Test that checks if a location is on the same side of a plane defined by an actor's forward vector as the querier.
 * Uses a context actor (typically provided by EnvQueryContext_FocusActor) and its forward vector to split the world into two sides.
 * Tests if the querier and the test location are on the same side of this plane.
 */
UCLASS()
class GOAPSHOOTER_API UEnvQueryTest_SameSideAsQuerier : public UEnvQueryTest
{
	GENERATED_BODY()
	
public:
	UEnvQueryTest_SameSideAsQuerier();
	
	/** Context for the actor that defines the dividing plane */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	TSubclassOf<UEnvQueryContext> ActorContext;
	
	/** Whether we want locations on the same side (true) or opposite side (false) as the querier */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	bool bWantSameSide;
	
	/** Whether to use 2D (ignore Z) or 3D calculations */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	bool bUse2DCalculation;
	
	/** Whether to use the actor's right vector instead of forward vector to define the plane */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	bool bUseRightVector;
	
protected:
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
	
	/** Helper function to determine if two points are on the same side of a plane */
	bool ArePointsOnSameSide(const FVector& PlaneOrigin, const FVector& PlaneNormal, 
	                         const FVector& Point1, const FVector& Point2, bool bUse2D) const;
};
