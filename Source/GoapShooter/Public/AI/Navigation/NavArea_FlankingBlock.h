#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "NavArea_FlankingBlock.generated.h"

/**
 * Custom navigation area to block AI movement for flanking purposes.
 */
UCLASS()
class GOAPSHOOTER_API UNavArea_FlankingBlock : public UNavArea
{
    GENERATED_BODY()

public:
    UNavArea_FlankingBlock(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
