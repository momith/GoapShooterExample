#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_MostPerceivedEnemy.generated.h"

UCLASS()
class GOAPSHOOTER_API UEnvQueryContext_MostPerceivedEnemy : public UEnvQueryContext
{
	GENERATED_BODY()

	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

	virtual UWorld* GetWorld() const override;

	void ProvideSingleActor(UObject* QuerierObject, AActor* QuerierActor, AActor*& ResultingActor) const;
};
