#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AI/GOAP/Utils/GoapTypes.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "GoapNode.generated.h"

/**
 * Helper class for A* search in GOAP planning.
 * Represents a node in the search graph.
 */
USTRUCT(BlueprintType)
struct GOAPSHOOTER_API FGoapNode
{
	GENERATED_BODY()

public:
	FGoapNode();
	
	FGoapNode(const TMap<FString, FGoapValue>& InState, 
		const TSharedPtr<FGoapNode>& InParent, 
		UGoapAction* InAction, 
		float InGCost, 
		float InHCost);

	/** Current world state at this node */
	TMap<FString, FGoapValue> State;

	/** Parent node in the search path */
	TSharedPtr<FGoapNode> Parent;

	/** Action that led to this node */
	UPROPERTY()
	UGoapAction* Action;

	/** Cost from start to this node */
	float GCost;

	/** Heuristic cost from this node to goal */
	float HCost;

	/** Get the total cost (F = G + H) */
	float GetFCost() const;

	/** Generate a hash for this node based on its state */
	uint32 GetStateHash() const;

	/** Check if two nodes have the same state */
	bool HasSameState(const FGoapNode& Other) const;
};

/**
 * Custom hash set for FGoapNode based on state
 */
struct GOAPSHOOTER_API FGoapNodeHasher
{
	uint32 operator()(const FGoapNode& Node) const
	{
		return Node.GetStateHash();
	}
};

/**
 * Equality predicate for FGoapNode based on state
 */
struct GOAPSHOOTER_API FGoapNodeEqualityPredicate
{
	bool operator()(const FGoapNode& A, const FGoapNode& B) const
	{
		return A.HasSameState(B);
	}
};

/**
 * Custom priority queue comparator for FGoapNode
 */
struct GOAPSHOOTER_API FGoapNodeComparator
{
	bool operator()(const TSharedPtr<FGoapNode>& A, const TSharedPtr<FGoapNode>& B) const
	{
		if (FMath::IsNearlyEqual(A->GetFCost(), B->GetFCost()))
		{
			return A->HCost < B->HCost;
		}
		return A->GetFCost() < B->GetFCost();
	}
};
