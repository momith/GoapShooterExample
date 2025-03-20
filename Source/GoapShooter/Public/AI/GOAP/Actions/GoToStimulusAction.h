#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "GoToStimulusAction.generated.h"

/**
 * GOAP Action for investigating a perceived stimulus.
 * This action is triggered when the AI detects a stimulus (visual, audio, etc.)
 * and needs to investigate it by turning toward the source and potentially
 * moving closer to confirm what was detected.
 */
UCLASS()
class GOAPSHOOTER_API UGoToStimulusAction : public UGoapAction
{
    GENERATED_BODY()
    
public:
    UGoToStimulusAction();
    
    /** How long to spend investigating (in seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
    float InvestigationTime = 1.5f;
    
    /** Maximum distance to investigate from (if further, AI will move closer) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
    float MaxInvestigationDistance = 1000.0f;
    
    /** Acceptable radius to consider the target reached */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
    float AcceptanceRadius = 100.0f;
    
    /** How long to wait at the target location */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
    float WaitDuration = 0.5f;
    
    /** Get the cost of this action */
    virtual float GetCost(const TMap<FString, FGoapValue>& WorldState) override;
    
};
