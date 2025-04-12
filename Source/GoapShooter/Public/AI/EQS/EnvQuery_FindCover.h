#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Distance.h"
#include "EnvQuery_FindCover.generated.h"

UCLASS()
class GOAPSHOOTER_API UEnvQuery_FindCover : public UEnvQuery
{
	GENERATED_BODY()

	UEnvQuery_FindCover(const FObjectInitializer& ObjectInitializer);
	
	UEnvQueryTest_Distance* CreateDistanceTest(UObject* Outer);
};
