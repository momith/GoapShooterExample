#include "AI/GOAP/Actions/GoToCoverAction.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"

UGoToCoverAction::UGoToCoverAction()
{
    Name = TEXT("GoToCoverAction");
    Cost = 4.0f;

    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsInCover), FGoapValue(false));
    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsCoverNearby), FGoapValue(true));
    //AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsFocusedOnStrongPerceivedStimulus), FGoapValue(true)); // rethink that

    AddEffect(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsInDanger), FGoapValue(false));
}

float UGoToCoverAction::GetCost(const TMap<FString, FGoapValue>& WorldState)
{
    float FinalCost = Cost;
    
    // Get AI position and best cover location
    FVector AIPosition = FVector::ZeroVector;
    FVector CoverLocation = FVector::ZeroVector;
    
    if (WorldState.Contains(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::AIPosition)))
    {
        AIPosition = WorldState[FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::AIPosition)].VectorValue;
    }
    
    if (WorldState.Contains(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::BestCoverLocation)))
    {
        CoverLocation = WorldState[FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::BestCoverLocation)].VectorValue;
    }
    
    // Calculate distance to cover
    float DistanceToCover = (CoverLocation - AIPosition).Size();
    
    // Base cost increases with distance to cover (normalized by a max expected distance)
    const float MaxExpectedDistance = 2000.0f; // Adjust based on your game scale
    float DistanceFactor = FMath::Clamp(DistanceToCover / MaxExpectedDistance, 0.0f, 1.0f);
    
    // Increase cost based on distance (up to 2x the base cost at max distance)
    FinalCost = Cost * (1.0f + DistanceFactor);
    
    // Check if being aimed at
    bool bIsAimedAt = false;
    if (WorldState.Contains(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsAimedAt)) 
        && WorldState[FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsAimedAt)] == FGoapValue(true))
    {
        bIsAimedAt = true;
    }
    
    // Always make cover more attractive when it's close, regardless of being aimed at
    if (DistanceToCover <= VeryNearThreshold)
    {
        // Cover is very close, make this action more attractive
        // The closer the cover, the more we reduce the cost
        float ProximityFactor = DistanceToCover / VeryNearThreshold;
        float ReductionMultiplier = bIsAimedAt ? 0.6f : 0.8f; // More reduction when aimed at
        FinalCost *= (ReductionMultiplier + ((1.0f - ReductionMultiplier) * ProximityFactor));
    }
    else if (bIsAimedAt)
    {
        // Being aimed at and cover is far away
        // Make dodging more attractive by increasing the cost
        float DistanceMultiplier = 1.5f + (DistanceFactor * 1.0f); // Up to 2.5x at max distance
        FinalCost *= DistanceMultiplier;
    }
    
    // TODO: consider also?
    // - low health
    // - no enemy in sight but stimuli exist and im not already near cover
    // - if current enemy is in a better position (at cover)
    // - stimuli exist and i habe cover nearby (low costs, high reward)

    return FinalCost;
}
