#include "AI/GOAP/Actions/LeftRightDodgeAction.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"
#include "AI/Utils/PerceptionTypes.h"
#include "Controllers/GoapShooterAIController.h"
#include "Kismet/GameplayStatics.h"

ULeftRightDodgeAction::ULeftRightDodgeAction()
{
    Cost = 5.0f;
    Name = "LeftRightDodge";
    
    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsAimedAt), FGoapValue(true));
    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsFocusedOnStrongPerceivedStimulus), FGoapValue(true));
    
    //AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsAimedAt), FGoapValue(true));
    AddEffect(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsInDanger), FGoapValue(false));
}

float ULeftRightDodgeAction::GetCost(const TMap<FString, FGoapValue>& WorldState)
{
    return Cost;
}