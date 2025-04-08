#include "AI/Navigation/FlankingNavModifier.h"
#include "AI/Navigation/NavArea_FlankingBlock.h"

AFlankingNavModifier::AFlankingNavModifier()
{
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
    RootComponent = BoxCollider;
    NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));

    NavModifier->AreaClass = UNavArea_FlankingBlock::StaticClass();
}
