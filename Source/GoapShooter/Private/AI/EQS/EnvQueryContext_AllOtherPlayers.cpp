#include "EQS/EnvQueryContext_AllOtherPlayers.h"
#include "GoapShooterCharacter.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"

void UEnvQueryContext_AllOtherPlayers::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	UObject* QuerierObject = QueryInstance.Owner.Get();
	if (QuerierObject == nullptr)
	{
		return;
	}
	AActor* QuerierActor = Cast<AActor>(QuerierObject);
	TArray<AActor*> ActorSet;
	ProvideActorsSet(QuerierObject, QuerierActor, ActorSet);
	ActorSet.Remove(nullptr);
	if (ActorSet.Num())
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, ActorSet);
	}
}

void UEnvQueryContext_AllOtherPlayers::ProvideActorsSet(UObject* QuerierObject, AActor* QuerierActor, TArray<AActor*>& ResultingActorsSet) const
{
	// Find all other player characters
	TArray<AActor*> ActorSet;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoapShooterCharacter::StaticClass(), ActorSet);
	ActorSet.Remove(QuerierActor);
	for (AActor* ActorToAdd : ActorSet)
	{
		ResultingActorsSet.Add(ActorToAdd);
	} 
}

UWorld* UEnvQueryContext_AllOtherPlayers::GetWorld() const
{
	check(GetOuter() != NULL);
	
	UEnvQueryManager* EnvQueryManager = Cast<UEnvQueryManager>(GetOuter());
	if (EnvQueryManager != NULL)
	{
		return EnvQueryManager->GetWorld();
	}
	
	return NULL;
}
