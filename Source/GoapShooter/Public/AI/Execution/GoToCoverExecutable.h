#pragma once

#include "CoreMinimal.h"
#include "AI/Execution/GoToExecutableAction.h"
#include "GoToCoverExecutable.generated.h"

/**
 * Executable action for moving towards a cover.
 */
UCLASS()
class GOAPSHOOTER_API UGoToCoverExecutable : public UGoToExecutableAction
{
    GENERATED_BODY()
    
public:
    UGoToCoverExecutable();
    
    virtual FVector CalculateTargetLocation() override;
};
