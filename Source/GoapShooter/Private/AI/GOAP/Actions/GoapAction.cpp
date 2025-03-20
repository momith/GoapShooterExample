#include "AI/GOAP/Actions/GoapAction.h"

UGoapAction::UGoapAction()
{
    Name = TEXT("BaseAction");
    Cost = 1.0f;
}

void UGoapAction::AddPrecondition(const FString& Key, const FGoapValue& Value)
{
    Preconditions.Add(Key, Value);
}

void UGoapAction::AddEffect(const FString& Key, const FGoapValue& Value)
{
    Effects.Add(Key, Value);
}

bool UGoapAction::ArePreconditionsSatisfied(const TMap<FString, FGoapValue>& WorldState) const
{
    // Check if all preconditions are satisfied
    for (const auto& Pair : Preconditions)
    {
        const FString& Key = Pair.Key;
        const FGoapValue& RequiredValue = Pair.Value;

        // Check if the key exists in the world state
        if (!WorldState.Contains(Key))
        {
            return false;
        }

        // Check if the value matches
        const FGoapValue& CurrentValue = WorldState[Key];
        if (CurrentValue != RequiredValue)
        {
            return false;
        }
    }

    return true;
}

TMap<FString, FGoapValue> UGoapAction::ApplyEffects(const TMap<FString, FGoapValue>& WorldState) const
{
    // Create a copy of the world state
    TMap<FString, FGoapValue> NewState = WorldState;

    // Apply all effects
    for (const auto& Pair : Effects)
    {
        NewState.Add(Pair.Key, Pair.Value);
    }

    return NewState;
}

bool UGoapAction::ShallActionBeConsidered(const TMap<FString, FGoapValue>& WorldState)
{
    return true;
}

float UGoapAction::GetCost(const TMap<FString, FGoapValue>& WorldState)
{
    return Cost;
}
