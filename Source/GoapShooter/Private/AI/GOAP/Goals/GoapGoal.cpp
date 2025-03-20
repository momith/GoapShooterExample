#include "AI/GOAP/Goals/GoapGoal.h"

UGoapGoal::UGoapGoal()
{
    Name = TEXT("BaseGoal");
    Priority = 1;
}

void UGoapGoal::AddDesiredState(const FString& Key, const FGoapValue& Value)
{
    DesiredState.Add(Key, Value);
}

bool UGoapGoal::IsSatisfiedBy(const TMap<FString, FGoapValue>& WorldState)
{
    // Check if all desired states are satisfied
    for (const auto& Pair : DesiredState)
    {
        const FString& Key = Pair.Key;
        const FGoapValue& DesiredValue = Pair.Value;

        // Check if the key exists in the world state
        if (!WorldState.Contains(Key))
        {
            return false;
        }

        // Check if the value matches
        const FGoapValue& CurrentValue = WorldState[Key];
        if (CurrentValue != DesiredValue)
        {
            return false;
        }
    }

    return true;
}

int32 UGoapGoal::GetPriority(const TMap<FString, FGoapValue>& WorldState)
{
    return Priority;
}

bool UGoapGoal::ShallGoalBeConsidered(const TMap<FString, FGoapValue>& WorldState)
{
    return true;
}
