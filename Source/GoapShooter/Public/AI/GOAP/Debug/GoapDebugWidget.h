#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AI/GOAP/Debug/GoapDebugHelper.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "GoapDebugWidget.generated.h"

/**
 * Simple widget for displaying GOAP debug information
 */
UCLASS()
class GOAPSHOOTER_API UGoapDebugWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    /** Set the debug helper to display information from */
    UFUNCTION(BlueprintCallable, Category = "GOAP|Debug")
    void SetDebugHelper(UGoapDebugHelper* InDebugHelper);
    
    /** Get the debug helper */
    UFUNCTION(BlueprintPure, Category = "GOAP|Debug")
    UGoapDebugHelper* GetDebugHelper() const { return DebugHelper; }
    
    UFUNCTION(BlueprintNativeEvent, Category = "GOAP|Debug")
    void UpdateInfo();

    UPROPERTY(BlueprintReadWrite)
    FString DebugText;
    void UpdateDebugText();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    
private:
    UPROPERTY()
    UGoapDebugHelper* DebugHelper;
};
