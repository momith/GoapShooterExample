#pragma once

#include "CoreMinimal.h"
#include "GoapPlannerConfiguration.generated.h"

/**
 * Configuration for a GoapPlanner.
 */
UCLASS(BlueprintType, Blueprintable)
class GOAPSHOOTER_API UGoapPlannerConfiguration : public UObject
{
    GENERATED_BODY()
    
public:
    UGoapPlannerConfiguration();

    /**
     * Initialize the GOAP planner with actions and goals
     * @param GoapPlanner - The GOAP planner to initialize
     * @param Controller - The AI controller that owns the planner
     */
    virtual void InitializeGoapPlanner(class UGoapPlanner* GoapPlanner);
    
    /**
     * Get the executable action for a GOAP action
     * @param GoapAction - The GOAP action to get the executable for
     * @return The executable action, or nullptr if no matching executable was found
     */
    virtual class UExecutableAction* GetExecutableAction(class UGoapAction* GoapAction);
};
