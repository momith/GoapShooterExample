#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AI/GOAP/Utils/GoapTypes.h"
#include "GoapWorldStateProviderInterface.generated.h"

/**
 * Interface for AI controllers that can provide a GOAP world state
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UGoapWorldStateProviderInterface : public UInterface
{
    GENERATED_BODY()
};

class GOAPSHOOTER_API IGoapWorldStateProviderInterface
{
    GENERATED_BODY()

public:
    /**
     * Calculate and return the current world state
     * @return Map of world state values
     */
    UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
    TMap<FString, FGoapValue> CalculateWorldState();
};
