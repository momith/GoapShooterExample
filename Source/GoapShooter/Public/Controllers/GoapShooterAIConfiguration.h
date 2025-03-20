#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AI/GOAP/Utils/GoapTypes.h"
#include "GoapShooterAIConfiguration.generated.h"

class UGoapPlanner;
class UGoapAction;
class UGoapGoal;
class AGoapShooterAIControllerBase;
class UExecutableAction;

/**
 * Configuration class for GOAP AI Controller
 * Handles setup of actions, goals, and other AI parameters
 */
UCLASS(BlueprintType, Blueprintable)
class GOAPSHOOTER_API UGoapShooterAIConfiguration : public UObject
{
    GENERATED_BODY()
    
public:
    UGoapShooterAIConfiguration();

    /**
     * Initialize the GOAP planner with actions and goals
     * @param GoapPlanner - The GOAP planner to initialize
     * @param Controller - The AI controller that owns the planner
     */
    virtual void InitializeGoapPlanner(UGoapPlanner* GoapPlanner);
    
    /**
     * Get the executable action for a GOAP action
     * @param GoapAction - The GOAP action to get the executable for
     * @return The executable action, or nullptr if no matching executable was found
     */
    virtual UExecutableAction* GetExecutableAction(UGoapAction* GoapAction);
};
