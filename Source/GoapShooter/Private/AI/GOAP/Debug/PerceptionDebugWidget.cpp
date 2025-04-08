#include "AI/GOAP/Debug/PerceptionDebugWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PerceptionMemoryComponent.h"
#include "Controllers/GoapShooterAIController.h"

void UPerceptionDebugWidget::SetAIController(AGoapShooterAIController* InAIController)
{
    AIController = InAIController;
}

void UPerceptionDebugWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    if (AIController)
    {
        UpdateDebugText();
        UpdateInfo();
    }
}

void UPerceptionDebugWidget::UpdateInfo_Implementation()
{
    //
}

void UPerceptionDebugWidget::UpdateDebugText()
{
    FString DebugString;
    
    if (AIController)
    {
        DebugString += TEXT("===== PERCEPTION DEBUG INFO =====\n");
        
        DebugString += TEXT("\nPerception Data:\n");
        
        const FPerceptionData& MostInterestingPerception = AIController->GetPerceptionMemoryComponent()->GetMostInterestingPerception();

        if (MostInterestingPerception.PerceivedActor)
        {
            AActor* Actor = MostInterestingPerception.PerceivedActor;
            FVector LastPerceivedLocation = MostInterestingPerception.LastPerceivedLocation;
            float LastPerceivedTime = MostInterestingPerception.LastPerceivedTime;
            FString LastPerceivedTimeString;
            if (LastPerceivedTime == FLT_MAX)
            {
                LastPerceivedTimeString = TEXT("FLT_MAX");
            }
            else if (LastPerceivedTime == -FLT_MAX)
            {
                LastPerceivedTimeString = TEXT("-FLT_MAX");
            }
            else
            {
                LastPerceivedTimeString = FString::Printf(TEXT("%.2f"), LastPerceivedTime);
            }
            EPerceptionType PerceptionType = MostInterestingPerception.PerceptionType;
            float Strength = MostInterestingPerception.GetStrength();
            bool bHasBeenInvestigated = MostInterestingPerception.bHasBeenInvestigated;

            DebugString += FString::Printf(TEXT("  Actor: %s\n"), *Actor->GetName());
            DebugString += FString::Printf(TEXT("  Last Perceived Location: %s\n"), *LastPerceivedLocation.ToString());
            DebugString += FString::Printf(TEXT("  Last Perceived Time: %s\n"), *LastPerceivedTimeString);
            DebugString += FString::Printf(TEXT("  Perception Type: %s\n"), *UEnum::GetValueAsName(PerceptionType).ToString());
            DebugString += FString::Printf(TEXT("  Strength: %f\n"), Strength);
            DebugString += FString::Printf(TEXT("  Has Been Investigated: %s\n"), bHasBeenInvestigated ? TEXT("true") : TEXT("false"));

            DebugString += TEXT("\n");
        }

    }
    
    DebugText = DebugString;
}
