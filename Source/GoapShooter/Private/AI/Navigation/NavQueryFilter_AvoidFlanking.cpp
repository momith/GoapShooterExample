#include "AI/Navigation/NavQueryFilter_AvoidFlanking.h"
#include "AI/Navigation/NavArea_FlankingBlock.h"
#include "NavFilters/NavigationQueryFilter.h"

UNavQueryFilter_AvoidFlanking::UNavQueryFilter_AvoidFlanking()
{
    AddTravelCostOverride(UNavArea_FlankingBlock::StaticClass(), FLT_MAX);

    FNavigationFilterArea FilterArea;
    FilterArea.AreaClass = UNavArea_FlankingBlock::StaticClass();
    FilterArea.bIsExcluded = true; // Explicitly exclude the area from pathfinding
    
    Areas.Add(FilterArea);
}
