#include "AI/GOAP/Planner/GoapNode.h"

FGoapNode::FGoapNode()
    : Action(nullptr)
    , GCost(0.0f)
    , HCost(0.0f)
{
}

FGoapNode::FGoapNode(const TMap<FString, FGoapValue>& InState, 
    const TSharedPtr<FGoapNode>& InParent, 
    UGoapAction* InAction, 
    float InGCost, 
    float InHCost)
    : State(InState)
    , Parent(InParent)
    , Action(InAction)
    , GCost(InGCost)
    , HCost(InHCost)
{
}

float FGoapNode::GetFCost() const
{
    return GCost + HCost;
}

// Helper function to combine hashes
inline uint32 CombineHashes(uint32 A, uint32 B)
{
    return A ^ (B + 0x9e3779b9 + (A << 6) + (A >> 2));
}

uint32 FGoapNode::GetStateHash() const
{
    uint32 Hash = 0;
    
    // Sort keys for consistent hashing
    TArray<FString> Keys;
    State.GetKeys(Keys);
    Keys.Sort();
    
    for (const FString& Key : Keys)
    {
        // Combine key hash with current hash
        Hash = CombineHashes(Hash, GetTypeHash(Key));
        
        // Get value and add to hash based on type
        const FGoapValue& Value = State[Key];
        
        // This is a simplification - in a real implementation you'd need to handle
        // the different value types properly
        switch (Value.ValueType)
        {
        case EGoapValueType::Boolean:
            Hash = CombineHashes(Hash, GetTypeHash(Value.BoolValue));
            break;
        case EGoapValueType::Integer:
            Hash = CombineHashes(Hash, GetTypeHash(Value.IntValue));
            break;
        case EGoapValueType::Float:
            Hash = CombineHashes(Hash, GetTypeHash(Value.FloatValue));
            break;
        case EGoapValueType::String:
            Hash = CombineHashes(Hash, GetTypeHash(Value.StringValue));
            break;
        case EGoapValueType::Object:
            Hash = CombineHashes(Hash, GetTypeHash(Value.ObjectValue));
            break;
        }
    }
    
    return Hash;
}

bool FGoapNode::HasSameState(const FGoapNode& Other) const
{
    if (State.Num() != Other.State.Num())
    {
        return false;
    }
    
    for (const auto& Pair : State)
    {
        const FString& Key = Pair.Key;
        const FGoapValue& Value = Pair.Value;
        
        if (!Other.State.Contains(Key))
        {
            return false;
        }
        
        const FGoapValue& OtherValue = Other.State[Key];
        if (Value != OtherValue)
        {
            return false;
        }
    }
    
    return true;
}
