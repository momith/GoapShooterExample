#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "AI/GOAP/Goals/GoapGoal.h"
#include "AI/GOAP/Planner/GoapPlanner.h"
#include "AI/GOAP/Planner/GoapPlannerConfiguration.h"
#include "AI/GOAP/Debug/GoapDebugHelper.h"
#include "AI/Execution/ExecutableAction.h"
#include "GoapShooterPlanningComponent.generated.h"

class AGoapShooterAIController;

/**
 * Component that handles GOAP planning for AI controllers
 * Can be used for different planning domains (movement, combat, etc.)
 */
UCLASS(ClassGroup = "AI", meta = (BlueprintSpawnableComponent))
class GOAPSHOOTER_API UGoapShooterPlanningComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGoapShooterPlanningComponent();

    UPROPERTY(EditAnywhere)
    bool bActivated = true;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UGoapPlannerConfiguration> ConfigClassToUse;

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** The AI Controller that owns this component */
    UPROPERTY()
    AGoapShooterAIController* Controller;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDebugGoapPlanning;

    /** Time between replanning attempts */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.1"))
    float ReplanInterval;

    /** Current cooldown until next replan */
    float ReplanCooldown;

    bool bIsReplanningForcedForNextTick;

    void OnSignificantWorldStateChange();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UGoapPlannerConfiguration* GoapConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UGoapPlanner* GoapPlanner;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UGoapAction* CurrentAction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UGoapGoal* CurrentGoal;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UExecutableAction* CurrentExecutableAction;

    void ExecuteCurrentAction(float DeltaTime);

    /** Check if no action is currently running */
    bool IsIdle();
    
    UExecutableAction* CreateExecutableAction(UGoapAction* GoapAction);
    
    UFUNCTION(BlueprintCallable)
    bool AbortCurrentAction();
    
    UFUNCTION()
    void OnActionCompleted(UExecutableAction* Action, bool Success);
    
    UPROPERTY()
    UGoapDebugHelper* GoapDebugHelper;
       
    UFUNCTION(BlueprintCallable)
    UGoapAction* GetCurrentAction() const { return CurrentAction; }
    
    UFUNCTION(BlueprintCallable)
    UGoapGoal* GetCurrentGoal() const { return CurrentGoal; }

    TMap<FString, FGoapValue> CalculateWorldState();
};
