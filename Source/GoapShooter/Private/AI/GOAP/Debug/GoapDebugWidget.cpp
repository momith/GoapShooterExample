#include "AI/GOAP/Debug/GoapDebugWidget.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "AI/GOAP/Goals/GoapGoal.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/BorderSlot.h"
#include "Blueprint/WidgetTree.h"

void UGoapDebugWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UGoapDebugWidget::SetDebugHelper(UGoapDebugHelper* InDebugHelper)
{
    DebugHelper = InDebugHelper;
}

void UGoapDebugWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    if (DebugHelper)
    {
        UpdateDebugText();
        UpdateInfo();
    }
}

void UGoapDebugWidget::UpdateInfo_Implementation()
{
    //
}

void UGoapDebugWidget::UpdateDebugText()
{
    FString DebugString;
    
    // Add current action and goal
    DebugString += TEXT("Current Action: ");
    DebugString += DebugHelper->GetTrackedCurrentAction() ? DebugHelper->GetTrackedCurrentAction()->GetName() : TEXT("None");
    DebugString += TEXT("\nCurrent Goal: ");
    DebugString += DebugHelper->GetTrackedCurrentGoal() ? DebugHelper->GetTrackedCurrentGoal()->GetName() : TEXT("None");
    DebugString += TEXT("\n\n");
    
    // Add debug helper information if available
    if (DebugHelper)
    {
        // Add header
        DebugString += TEXT("===== GOAP ACTION DEBUG INFO =====\n");
        
        // Add action evaluations
        DebugString += TEXT("\nAction Evaluations:\n");
        
        const TArray<FGoapActionDebugInfo>& ActionDebugInfo = DebugHelper->GetActionDebugInfo();
        for (const FGoapActionDebugInfo& DebugInfo : ActionDebugInfo)
        {
            DebugString += FString::Printf(TEXT("  %s:\n"), *DebugInfo.ActionName);
            //DebugString += FString::Printf(TEXT("    Should Be Considered: %s\n"), 
            //    DebugInfo.bShouldBeConsidered ? TEXT("true") : TEXT("false"));
            DebugString += FString::Printf(TEXT("    Preconditions Satisfied: %s\n"), 
                DebugInfo.bPreconditionsSatisfied ? TEXT("true") : TEXT("false"));
            DebugString += FString::Printf(TEXT("    Cost: %f\n"), DebugInfo.Cost);
        }
        
        DebugString += TEXT("==================================\n");
    }
    else
    {
        // No debug helper available
        DebugString += TEXT("===== NO GOAP DEBUG HELPER AVAILABLE =====\n");
        DebugString += TEXT("\nPossible reasons:\n");
        DebugString += TEXT("  1. No AI controllers in the scene\n");
        DebugString += TEXT("  2. AI controllers not properly initialized\n");
        DebugString += TEXT("  3. Debug helper not created in AI controller\n");
        DebugString += TEXT("\nCheck the output log for more information.\n");
        DebugString += TEXT("==================================\n");
    }
    
    DebugText = DebugString;
}
