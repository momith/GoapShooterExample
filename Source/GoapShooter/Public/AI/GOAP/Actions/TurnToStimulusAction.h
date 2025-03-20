#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "TurnToStimulusAction.generated.h"

/**
 * GOAP Action for turning towards a perceived stimulus.
 * This action is triggered when the AI detects a stimulus (visual, audio, etc.)
 * and needs to turn toward the source to face it without moving.
 * This is useful for quick reactions to stimuli without committing to movement.
 */
UCLASS()
class GOAPSHOOTER_API UTurnToStimulusAction : public UGoapAction
{
    GENERATED_BODY()
    
public:
    UTurnToStimulusAction();
    
    /** 
     * Calculate the cost of this action based on the current world state.
     * Cost is adjusted based on the type of stimulus (visual, audio, damage).
     */
    virtual float GetCost(const TMap<FString, FGoapValue>& WorldState) override;
    
    /** How quickly to turn towards the stimulus (degrees per second) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
    float TurnSpeed = 180.0f;
    
    /** How long to maintain focus on the stimulus location (in seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
    float FocusDuration = 1.0f;
};
