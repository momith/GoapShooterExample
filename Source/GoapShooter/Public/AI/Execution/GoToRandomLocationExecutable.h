#pragma once

#include "CoreMinimal.h"
#include "AI/Execution/GoToExecutableAction.h"
#include "GoToRandomLocationExecutable.generated.h"

/**
 * Executable action that moves the AI to a random location
 * Uses Unreal's navigation system and movement completion notifications
 */
UCLASS()
class GOAPSHOOTER_API UGoToRandomLocationExecutable : public UGoToExecutableAction
{
	GENERATED_BODY()
	
public:
	UGoToRandomLocationExecutable();
	
	virtual FVector CalculateTargetLocation() override;

	/** Maximum search radius for finding a random location */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation")
	float MaxSearchRadius;
	
	/** Minimum search radius for finding a random location */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation")
	float MinSearchRadius;
	
	/** Maximum number of attempts to find a valid random location */
	UPROPERTY(EditDefaultsOnly, Category = "Navigation")
	int32 MaxLocationFindingAttempts;
	
private:
	/** Find a random navigable location */
	bool FindRandomLocation(FVector& OutLocation);
};
