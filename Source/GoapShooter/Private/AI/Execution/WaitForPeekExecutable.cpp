#include "AI/Execution/WaitForPeekExecutable.h"

UWaitForPeekExecutable::UWaitForPeekExecutable()
{
    WaitTimer = 0.0f;
    WaitDuration = 1.0f; // Wait for 1 second before completing
}

bool UWaitForPeekExecutable::StartExecution()
{
    Super::StartExecution();
    
    // Reset the timer when we start
    WaitTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("WaitForPeekExecutable: Started waiting for %f seconds"), WaitDuration);
    return true;
}

void UWaitForPeekExecutable::TickAction(float DeltaTime)
{
    if (!bIsExecuting && !bIsComplete)
    {
        StartExecution();
    }

    // Increment the timer
    WaitTimer += DeltaTime;
    
    // Check if we've waited long enough
    if (WaitTimer >= WaitDuration)
    {
        UE_LOG(LogTemp, Log, TEXT("WaitForPeekExecutable: Wait completed after %f seconds"), WaitTimer);
        CompleteAction();
    }
}