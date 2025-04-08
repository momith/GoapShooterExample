#pragma once

#include "CoreMinimal.h"
#include "AI/Execution/GoToExecutableAction.h"
#include "PeekExecutable.generated.h"

/**
 * Executable action for peeking
 * Moves the AI perpendicular to the line between AI and threat
 */
UCLASS()
class GOAPSHOOTER_API UPeekExecutable : public UGoToExecutableAction
{
	GENERATED_BODY()
	
public:
	UPeekExecutable();

	float MinPeekDistance = 100.0f;
	float MaxPeekDistance = 300.0f;

	virtual FVector CalculateTargetLocation() override;
};
