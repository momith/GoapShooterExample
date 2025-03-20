#include "AI/GOAP/Actions/WaitForPeekAction.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"
#include "Kismet/GameplayStatics.h"

UWaitForPeekAction::UWaitForPeekAction()
{
    Name = TEXT("WaitForPeekAction");
    Cost = BaseCost;
    
    AddPrecondition(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy), FGoapValue(false));
    
    AddEffect(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy), FGoapValue(true));
    
    // Initialize threshold values
    bThresholdInitialized = false;
    ThresholdTime = 0.0f;
}

float UWaitForPeekAction::GetCost(const TMap<FString, FGoapValue>& WorldState)
{
    // Base cost for waiting
    float ActionCost = BaseCost;
    
    // Get the world time when enemy was last seen
    float WorldTimeLastEnemySeen = -FLT_MAX;
    FString WorldTimeLastEnemySeenKey = FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::WorldTimeLastEnemySeen);
    if (WorldState.Contains(WorldTimeLastEnemySeenKey))
    {
        WorldTimeLastEnemySeen = WorldState[WorldTimeLastEnemySeenKey].FloatValue;
    }
    
    // Get current world time
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return ActionCost;
    }
    
    float CurrentWorldTime = World->GetTimeSeconds();
    
    // Calculate time since last enemy sighting
    float TimeSinceLastSighting = CurrentWorldTime - WorldTimeLastEnemySeen;
    
    // Initialize random threshold if not already done
    if (!bThresholdInitialized)
    {
        // Set a random seed based on the instance pointer and provided seed
        FRandomStream RandomStream(GetUniqueID() + RandomSeed);
        
        // Generate a random threshold between MinThresholdTime and MaxThresholdTime
        ThresholdTime = RandomStream.FRandRange(MinThresholdTime, MaxThresholdTime);
        bThresholdInitialized = true;
        
        UE_LOG(LogTemp, Display, TEXT("WaitForPeekAction initialized with threshold time: %f seconds"), ThresholdTime);
    }
    
    // Increase cost based on time since last sighting
    if (TimeSinceLastSighting > 0.0f)
    {
        if (TimeSinceLastSighting <= ThresholdTime)
        {
            // Normal linear cost increase before threshold
            ActionCost += TimeSinceLastSighting * TimeSinceSightingCostMultiplier;
        }
        else
        {
            // Base cost up to threshold
            float ThresholdCost = ThresholdTime * TimeSinceSightingCostMultiplier;
            
            // Accelerated cost after threshold (still linear, but steeper slope)
            float ExcessTime = TimeSinceLastSighting - ThresholdTime;
            float AcceleratedCost = ExcessTime * AcceleratedCostMultiplier;
            
            // Combine costs
            ActionCost += ThresholdCost + AcceleratedCost;
        }
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("WaitForPeekAction GetCost: %f (Time since last sight: %f, Threshold: %f)"), 
           ActionCost, TimeSinceLastSighting, ThresholdTime);

    return ActionCost;
}
