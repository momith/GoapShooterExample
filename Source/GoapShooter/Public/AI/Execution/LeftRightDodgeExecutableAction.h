#pragma once

#include "CoreMinimal.h"
#include "AI/Execution/GoToExecutableAction.h"
#include "LeftRightDodgeExecutableAction.generated.h"

/**
 * Executable action for left-right dodging
 * Moves the AI perpendicular to the line between AI and threat
 */
UCLASS()
class GOAPSHOOTER_API ULeftRightDodgeExecutableAction : public UGoToExecutableAction
{
	GENERATED_BODY()
	
public:
	ULeftRightDodgeExecutableAction();

	virtual FVector CalculateTargetLocation() override;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float MinDodgeDistance = 30.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float MaxDodgeDistance = 100.0f;
	
};
