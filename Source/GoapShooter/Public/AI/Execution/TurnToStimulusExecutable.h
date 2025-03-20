#pragma once

#include "CoreMinimal.h"
#include "AI/Execution/ExecutableAction.h"
#include "TurnToStimulusExecutable.generated.h"

/**
 * Executable action for turning towards a perceived stimulus.
 * This action rotates the AI character to face the location of the stimulus.
 */
UCLASS()
class GOAPSHOOTER_API UTurnToStimulusExecutable : public UExecutableAction
{
    GENERATED_BODY()
    
public:
    UTurnToStimulusExecutable();
    
    // UExecutableAction interface
    virtual bool StartExecution() override;
    virtual void TickAction(float DeltaTime) override;
    // End of UExecutableAction interface
    
    UPROPERTY(EditDefaultsOnly, Category = "GOAP")
    float TurnSpeed = 180.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "GOAP")
    float FocusDuration = 0.2f;
    
private:
	bool bHasStarted;

    /** Target location to turn towards */
    FVector TargetLocation;
    
    /** Whether we have reached the target rotation */
    bool bHasReachedTarget;
    
    /** Time spent focusing on the target */
    float FocusTime;
    
    /** Calculate the rotation needed to face the target */
    FRotator CalculateTargetRotation() const;
    
    /** Check if we've reached the target rotation */
    bool HasReachedTargetRotation() const;
};
