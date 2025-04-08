#pragma once

#include "CoreMinimal.h"
#include "AI/Execution/ExecutableAction.h"
#include "WaitForPeekExecutable.generated.h"

/**
 * Executable action that waits for 1 second before completing
 * Used as a delay before peeking
 */
UCLASS()
class GOAPSHOOTER_API UWaitForPeekExecutable : public UExecutableAction
{
    GENERATED_BODY()
    
public:
    UWaitForPeekExecutable();
    
    virtual void TickAction(float DeltaTime) override;
    virtual bool StartExecution() override;
    
private:
    // Timer to track how long we've been waiting
    float WaitTimer;
    
    // Total time to wait before completing
    UPROPERTY(EditDefaultsOnly, Category = "Wait")
    float WaitDuration;
};