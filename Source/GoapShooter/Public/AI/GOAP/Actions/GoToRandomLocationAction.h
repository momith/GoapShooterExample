#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "GoToRandomLocationAction.generated.h"

UCLASS()
class GOAPSHOOTER_API UGoToRandomLocationAction : public UGoapAction
{
	GENERATED_BODY()
	
public:
	UGoToRandomLocationAction();
	
	virtual float GetCost(const TMap<FString, FGoapValue>& WorldState) override;
	virtual bool ShallActionBeConsidered(const TMap<FString, FGoapValue>& WorldState) override;
};
