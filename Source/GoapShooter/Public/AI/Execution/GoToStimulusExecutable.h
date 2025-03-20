#pragma once

#include "CoreMinimal.h"
#include "AI/Execution/ExecutableAction.h"
#include "AI/Utils/PerceptionTypes.h"
#include "GoToStimulusExecutable.generated.h"

/**
 * Executable action for moving towards a perceived stimulus.
 * This action moves the AI character to the location of the stimulus.
 */
UCLASS()
class GOAPSHOOTER_API UGoToStimulusExecutable : public UExecutableAction
{
    GENERATED_BODY()
    
public:
    UGoToStimulusExecutable();
    
    // UExecutableAction interface
    virtual bool StartExecution() override;
    virtual void TickAction(float DeltaTime) override;
    // End of UExecutableAction interface
    
    /** Akzeptanzradius, in dem das Ziel als erreicht gilt */
    UPROPERTY(EditDefaultsOnly, Category = "GOAP")
    float AcceptanceRadius = 100.0f;
    
    /** Wie lange die KI am Ziel warten soll (in Sekunden) */
    UPROPERTY(EditDefaultsOnly, Category = "GOAP")
    float WaitDuration = 0.5f;
    
private:
    /** Target location to move towards */
    FVector TargetLocation;
    
    /** Whether we have reached the target location */
    bool bHasReachedTarget;
    
    /** Time spent waiting at the target location */
    float WaitTime;
    
    /** Move the AI to the target location using the AI controller */
    void MoveToTarget();
    
    /** Check if we've reached the target location */
    bool HasReachedTarget() const;

    UPROPERTY()
    FPerceptionData MostInterestingPerception;
};
