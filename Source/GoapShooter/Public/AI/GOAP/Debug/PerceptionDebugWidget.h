#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PerceptionDebugWidget.generated.h"

class AGoapShooterAIController;

UCLASS()
class GOAPSHOOTER_API UPerceptionDebugWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    UFUNCTION(BlueprintCallable)
    void SetAIController(AGoapShooterAIController* InAIController);
    
    UFUNCTION(BlueprintPure)
    AGoapShooterAIController* GetAIController() const { return AIController; }
    
    UFUNCTION(BlueprintNativeEvent)
    void UpdateInfo();

    UPROPERTY(BlueprintReadWrite)
    FString DebugText;
    void UpdateDebugText();

protected:
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    
private:
    UPROPERTY()
    AGoapShooterAIController* AIController;
};
