#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_RandomEnemy.generated.h"

/**
 * this is for testing
 */
UCLASS()
class GOAPSHOOTER_API UEnvQueryContext_RandomEnemy : public UEnvQueryContext
{
	GENERATED_BODY()

	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

	virtual UWorld* GetWorld() const override;

	void ProvideSingleActor(UObject* QuerierObject, AActor* QuerierActor, AActor*& ResultingActor) const;
};
