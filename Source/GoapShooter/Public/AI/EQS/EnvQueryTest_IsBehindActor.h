#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_IsBehindActor.generated.h"

/**
 * EQS Test that checks if a location is behind a specified actor.
 * Uses a context actor (typically provided by EnvQueryContext_FocusActor) and checks
 * if test locations are positioned behind this actor based on its forward vector.
 */
UCLASS()
class GOAPSHOOTER_API UEnvQueryTest_IsBehindActor : public UEnvQueryTest
{
	GENERATED_BODY()
	
public:
	UEnvQueryTest_IsBehindActor();
	
	/** Context for the actor that we want to be behind */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	TSubclassOf<UEnvQueryContext> ActorContext;
	
	/** Angle in degrees that defines what "behind" means (180 means directly behind) */
	UPROPERTY(EditDefaultsOnly, Category = "Test", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
	float BehindAngleThreshold;
	
	/** Distance threshold for considering a point as "behind" */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	float MinimumDistanceBehind;
	
	/** Whether to use 2D (ignore Z) or 3D distance calculations */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	bool bUse2DDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Test")
	bool bExpected;

	UPROPERTY(EditDefaultsOnly, Category = "Test")
	bool bInvertDirection;

protected:
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};
