#include "AI/GOAP/Actions/TurnToStimulusAction.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"
#include "AI/Utils/PerceptionTypes.h"
#include "Controllers/GoapShooterAIController.h"
#include "Kismet/GameplayStatics.h"

UTurnToStimulusAction::UTurnToStimulusAction()
{
    Cost = 5.0f;

    // Set action name
    Name = "TurnToStimulus";
    
    // Set preconditions - we need a stimulus that hasn't been investigated yet
    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::HasPerceivedStimulus), FGoapValue(true));
    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::HasInvestigatedStimulus), FGoapValue(false));
    
    // Set effects - after this action, the stimulus will be partially investigated
    // Note: We don't set HasInvestigatedStimulus to true since we're only turning, not fully investigating
    // This allows for a potential follow-up action like GoToStimulusAction
    
    // This action can potentially lead to seeing an enemy if they're in the direction we turn
    AddEffect(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy), FGoapValue(true));
}

float UTurnToStimulusAction::GetCost(const TMap<FString, FGoapValue>& WorldState)
{
    // Base cost
    float ActionCost = Cost;
    
    // Adjust cost based on stimulus type - visual stimuli are more reliable
    FString StimulusTypeKey = FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::PerceivedStimulusType);
    if (WorldState.Contains(StimulusTypeKey))
    {
        FString StimulusType = WorldState[StimulusTypeKey].StringValue;
        EPerceptionType PerceptionType = FPerceptionTypeUtils::FromString(StimulusType);
        
        // Visual stimuli are more reliable, so lower cost
        if (PerceptionType == EPerceptionType::Visual)
        {
            ActionCost *= 0.7f;
        }
        // Audio stimuli are less reliable, but still worth turning towards
        else if (PerceptionType == EPerceptionType::Audio)
        {
            ActionCost *= 0.9f;
        }
        // Damage stimuli are very important, so lower cost
        else if (PerceptionType == EPerceptionType::Damage)
        {
            ActionCost *= 0.3f;
        }
    }
    
    // TODO: consider distance to stimuli ?
    
    return ActionCost;
}
