#include "EQS/EnvQueryContext_RandomEnemy.h"
#include "GoapShooterCharacter.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"

void UEnvQueryContext_RandomEnemy::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	UObject* QuerierObject = QueryInstance.Owner.Get();
	if (QuerierObject == nullptr)
	{
		return;
	}
	AActor* QuerierActor = Cast<AActor>(QuerierObject);
	AActor* ResultingActor = NULL;
	ProvideSingleActor(QuerierObject, QuerierActor, ResultingActor);
	if (ResultingActor)
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, ResultingActor);
	}
}

void UEnvQueryContext_RandomEnemy::ProvideSingleActor(UObject* QuerierObject, AActor* QuerierActor, AActor*& ResultingActor) const
{
	TArray<AActor*> ActorSet;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoapShooterCharacter::StaticClass(), ActorSet);
	if (ActorSet.Num() > 0)
	{
		ResultingActor = ActorSet[0];
	}
}

UWorld* UEnvQueryContext_RandomEnemy::GetWorld() const
{
	check(GetOuter() != NULL);
	
	UEnvQueryManager* EnvQueryManager = Cast<UEnvQueryManager>(GetOuter());
	if (EnvQueryManager != NULL)
	{
		return EnvQueryManager->GetWorld();
	}
	
	return NULL;
}
