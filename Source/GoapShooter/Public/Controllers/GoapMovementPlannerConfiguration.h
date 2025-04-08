#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Planner/GoapPlannerConfiguration.h"
#include "GoapMovementPlannerConfiguration.generated.h"

/**
 * Configuration for a GoapPlanner that handles movement actions (lower body).
 * Another configuration handles combat actions (upper body).
 */
UCLASS(BlueprintType, Blueprintable)
class GOAPSHOOTER_API UGoapMovementPlannerConfiguration : public UGoapPlannerConfiguration
{
    GENERATED_BODY()
    
public:
    UGoapMovementPlannerConfiguration();

    /**
     * Initialize the GOAP planner with actions and goals
     * @param GoapPlanner - The GOAP planner to initialize
     * @param Controller - The AI controller that owns the planner
     */
    virtual void InitializeGoapPlanner(UGoapPlanner* GoapPlanner) override;
    
    /**
     * Get the executable action for a GOAP action
     * @param GoapAction - The GOAP action to get the executable for
     * @return The executable action, or nullptr if no matching executable was found
     */
    virtual UExecutableAction* GetExecutableAction(UGoapAction* GoapAction) override;
};
