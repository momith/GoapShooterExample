#include "AI/GOAP/Actions/GoToStimulusAction.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"
#include "AI/Utils/PerceptionTypes.h"
#include "Controllers/GoapShooterAIController.h"
#include "Kismet/GameplayStatics.h"

UGoToStimulusAction::UGoToStimulusAction()
{
    Cost = 10.0f;

    // Set action name
    Name = "GoToStimulusAction";
    
    // Set preconditions - we need a stimulus that hasn't been investigated yet
    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::HasPerceivedStimulus), FGoapValue(true));
    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::HasInvestigatedStimulus), FGoapValue(false));
    
    // Set effects - after this action, the stimulus will be investigated
    AddEffect(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::HasInvestigatedStimulus), FGoapValue(true));
    
    // This action can potentially lead to seeing an enemy
    AddEffect(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy), FGoapValue(true));
}

float UGoToStimulusAction::GetCost(const TMap<FString, FGoapValue>& WorldState)
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
            ActionCost *= 0.8f;
        }
        // Audio stimuli are less reliable, so higher cost
        else if (PerceptionType == EPerceptionType::Audio)
        {
            ActionCost *= 1.2f;
        }
        // Damage stimuli are very important, so lower cost
        else if (PerceptionType == EPerceptionType::Damage)
        {
            ActionCost *= 0.5f;
        }
    }
    
    // Adjust cost based on distance to stimulus
    FString StimulusLocationKey = FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::PerceivedStimulusLocation);
    FString AIPositionKey = FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::AIPosition);
    
    if (WorldState.Contains(StimulusLocationKey) && WorldState.Contains(AIPositionKey))
    {
        FVector StimulusLocation = WorldState[StimulusLocationKey].VectorValue;
        FVector AILocation = WorldState[AIPositionKey].VectorValue;
        
        float Distance = FVector::Dist(AILocation, StimulusLocation);
        
        float DistanceFactor = FMath::Clamp(Distance / 1000.0f, 0.5f, 2.0f);
        ActionCost *= DistanceFactor;
        
        if (Distance < 200.0f)
        {
            ActionCost *= 0.5f;
        }
    }
    
    return ActionCost;
}