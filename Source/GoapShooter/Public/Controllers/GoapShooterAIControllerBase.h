#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "AI/GOAP/Goals/GoapGoal.h"
#include "AI/GOAP/Planner/GoapPlanner.h"
#include "AI/GOAP/Debug/GoapDebugHelper.h"
#include "AI/GOAP/Debug/GoapDebugWidget.h"
#include "Controllers/GoapShooterAIConfiguration.h"
#include "AI/Execution/ExecutableAction.h"
#include "GoapShooterAIControllerBase.generated.h"

/**
 * AI Controller that uses GOAP for decision making
 */
UCLASS()
class GOAPSHOOTER_API AGoapShooterAIControllerBase : public AAIController
{
    GENERATED_BODY()

public:
    AGoapShooterAIControllerBase(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** GOAP configuration for decision making */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP")
    UGoapShooterAIConfiguration* GoapConfig;

    /** GOAP planner for decision making */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP")
    UGoapPlanner* GoapPlanner;

    /** Currently executing action */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP")
    UGoapAction* CurrentAction;

    /** Current goal being pursued */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP")
    UGoapGoal* CurrentGoal;
    
    /** Currently executing executable action */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP")
    UExecutableAction* CurrentExecutableAction;

    /** Time between replanning attempts */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GOAP", meta = (ClampMin = "0.1"))
    float ReplanInterval;

    /** Current cooldown until next replan */
    float ReplanCooldown;

    /** Initialize the GOAP planner with actions and goals */
    void InitializeGoapPlanner();

    /** Calculate the current world state */
    UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
    TMap<FString, FGoapValue> CalculateWorldState();
    virtual TMap<FString, FGoapValue> CalculateWorldState_Implementation();

    /** Execute the current action */
    void ExecuteCurrentAction(float DeltaTime);

    /** Check if the current action is complete */
    bool IsIdle();
    
    /** Create an executable action for the given GOAP action */
    UExecutableAction* CreateExecutableAction(UGoapAction* GoapAction);
    
    /** Abort the current executable action if one is running */
    UFUNCTION(BlueprintCallable, Category = "GOAP")
    bool AbortCurrentAction();
    
    /** Handle action completion */
    UFUNCTION()
    void OnActionCompleted(UExecutableAction* Action, bool Success);
    
    /** Flag to enable/disable GOAP debugging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Debug")
    bool bDebugGoapPlanning;
    
    /** Debug helper for GOAP planning */
    UPROPERTY()
    UGoapDebugHelper* DebugHelper;
    
    /** Get the current action */
    UFUNCTION(BlueprintCallable, Category = "GOAP|Actions")
    UGoapAction* GetCurrentAction() const { return CurrentAction; }
    
    /** Get the current goal */
    UFUNCTION(BlueprintCallable, Category = "GOAP|Goals")
    UGoapGoal* GetCurrentGoal() const { return CurrentGoal; }
    
};
