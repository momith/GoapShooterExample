#include "EQS/EnvQueryContext_MostPerceivedEnemy.h"
#include "GoapShooterAIController.h"
#include "GoapShooterCharacter.h"
#include "Components/PerceptionMemoryComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_MostPerceivedEnemy::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
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

void UEnvQueryContext_MostPerceivedEnemy::ProvideSingleActor(UObject* QuerierObject, AActor* QuerierActor, AActor*& ResultingActor) const
{
	AGoapShooterCharacter* QuerierChar = Cast<AGoapShooterCharacter>(QuerierActor);
	if (QuerierChar)
	{
		if (QuerierChar->GetController())
		{
			AGoapShooterAIController* AIController = Cast<AGoapShooterAIController>(QuerierChar->GetController());
			if (AIController)
			{
				ResultingActor = AIController->GetPerceptionMemoryComponent()->GetMostInterestingPerceivedEnemy();
			}
		}
	}
}

UWorld* UEnvQueryContext_MostPerceivedEnemy::GetWorld() const
{
	check(GetOuter() != NULL);
	
	UEnvQueryManager* EnvQueryManager = Cast<UEnvQueryManager>(GetOuter());
	if (EnvQueryManager != NULL)
	{
		return EnvQueryManager->GetWorld();
	}
	
	return NULL;
}
