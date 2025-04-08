#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/Actions/GoapAction.h"
#include "ShootAtEnemyAction.generated.h"

/**
 * Action to shoot at a visible enemy
 */
UCLASS()
class GOAPSHOOTER_API UShootAtEnemyAction : public UGoapAction
{
	GENERATED_BODY()
	
public:
	UShootAtEnemyAction();

	virtual float GetCost(const TMap<FString, FGoapValue>& WorldState) override;
};
