#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Distance.h"
#include "EnvQuery_FindFlankTargetLocation.generated.h"

UCLASS()
class GOAPSHOOTER_API UEnvQuery_FindFlankTargetLocation : public UEnvQuery
{
	GENERATED_BODY()

	UEnvQuery_FindFlankTargetLocation(const FObjectInitializer& ObjectInitializer);
	
	UEnvQueryTest_Distance* CreateDistanceTest(UObject* Outer);
};
