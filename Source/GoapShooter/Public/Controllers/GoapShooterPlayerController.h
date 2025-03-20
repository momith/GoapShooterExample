#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AI/GOAP/Debug/GoapDebugWidget.h"
#include "InputActionValue.h"
#include "GoapShooterPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

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
    
    void ToggleGoapDebugWidget();
    
    void ShowGoapDebugWidget();
    
    void HideGoapDebugWidget();
    
protected:
    /** Class to use for the GOAP debug widget */
    UPROPERTY(EditDefaultsOnly, Category = "GOAP|Debug")
    TSubclassOf<UGoapDebugWidget> GoapDebugWidgetClass;
    
    /** The GOAP debug widget instance */
    UPROPERTY()
    UGoapDebugWidget* GoapDebugWidget;
    
    /** Input Mapping Context */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;
    
    /** O Key Input Action */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* OKeyAction;
    
    /** Handler for O key press */
    void OnOKeyPressed();
    
    /** Find the closest AI controller to debug */
    class AGoapShooterAIControllerBase* FindClosestAIController();
};
