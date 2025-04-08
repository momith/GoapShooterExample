#pragma once

#include "CoreMinimal.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "NavQueryFilter_AvoidFlanking.generated.h"

/**
 * Navigation filter that makes AI avoid the Flanking Block area.
 */
UCLASS()
class GOAPSHOOTER_API UNavQueryFilter_AvoidFlanking : public UNavigationQueryFilter
{
    GENERATED_BODY()

public:
    UNavQueryFilter_AvoidFlanking();
};
