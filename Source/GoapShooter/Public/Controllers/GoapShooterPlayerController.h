#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AI/GOAP/Debug/GoapDebugWidget.h"
#include "GoapShooterPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UPerceptionDebugWidget;
class AGoapShooterAIController;

/**
 * Player controller for the GoapShooter game
 */
UCLASS()
class GOAPSHOOTER_API AGoapShooterPlayerController : public APlayerController
{
    GENERATED_BODY()
    
public:
    AGoapShooterPlayerController();
    
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    
    void TogglePerceptionDebugWidget();
    void ToggleMovementGoapDebugWidget();
    void ToggleCombatGoapDebugWidget();

    void ToggleGoapDebugWidget(bool bDebugMovementGoap);
    void ShowWidget(UUserWidget* Widget);
    void HideWidget(UUserWidget* Widget);
    
protected:
    UPROPERTY(EditDefaultsOnly, Category = "GOAP|Debug")
    TSubclassOf<UGoapDebugWidget> GoapDebugWidgetClass;
    UPROPERTY(EditDefaultsOnly, Category = "GOAP|Debug")
    TSubclassOf<UPerceptionDebugWidget> PerceptionDebugWidgetClass;
    
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;
    
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* MovementDebugKeyAction;
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* CombatDebugKeyAction;
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* PerceptionDebugKeyAction;

    UPROPERTY()
    UGoapDebugWidget* MovementGoapDebugWidget;
    UPROPERTY()
    UGoapDebugWidget* CombatGoapDebugWidget;

    UPROPERTY()
    UPerceptionDebugWidget* PerceptionDebugWidget;

    UPROPERTY()
    AGoapShooterAIController* ClosestAIController;
    /** Find the closest AI controller to debug */
    class AGoapShooterAIController* FindClosestAIController();
};
