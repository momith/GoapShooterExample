#include "AI/EQS/EnvQueryTest_IsBehindActor.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"

UEnvQueryTest_IsBehindActor::UEnvQueryTest_IsBehindActor()
{
    // Set default values
    BehindAngleThreshold = 135.0f;  // Default: consider anything beyond 135 degrees as "behind"
    MinimumDistanceBehind = 100.0f; // Minimum distance to be considered "behind"
    bUse2DDistance = true;          // Default to 2D distance (ignore height differences)
    bExpected = true;

    // Set the default actor context to the querier (can be changed in editor)
    ActorContext = UEnvQueryContext_Querier::StaticClass();
    
    // Set test purpose and filtering type
    TestPurpose = EEnvTestPurpose::Filter;
    FilterType = EEnvTestFilterType::Match;  // Match = keep items that pass
    
    // Define what kind of data this test operates on
    ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
    
    // Set category for the test
    SetWorkOnFloatValues(false);
}

void UEnvQueryTest_IsBehindActor::RunTest(FEnvQueryInstance& QueryInstance) const
{
    UObject* QueryOwner = QueryInstance.Owner.Get();
    if (!QueryOwner)
    {
        return;
    }
    
    // Get the actor we want to be behind
    TArray<AActor*> ContextActors;
    QueryInstance.PrepareContext(ActorContext, ContextActors);
    
    if (ContextActors.Num() == 0)
    {
        return;
    }
    
    // We'll use the first actor from the context
    AActor* ContextActor = ContextActors[0];
    if (!ContextActor)
    {
        return;
    }
    
    // Get the forward vector of the context actor
    const FVector ContextForward = bInvertDirection ? -ContextActor->GetActorForwardVector() : ContextActor->GetActorForwardVector();
    const FVector ContextLocation = ContextActor->GetActorLocation();
    
    // Normalize the cosine of our angle threshold for comparison
    const float CosineThreshold = FMath::Cos(FMath::DegreesToRadians(BehindAngleThreshold));
    
    // Go through all items in the query
    for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
    {
        // Get the location we're testing
        const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
        
        // Calculate direction from context actor to test location
        FVector DirectionToItem = ItemLocation - ContextLocation;
        
        // If using 2D distance, zero out the Z component
        if (bUse2DDistance)
        {
            DirectionToItem.Z = 0.0f;
        }
        
        // Skip if the item is too close
        const float Distance = DirectionToItem.Size();
        if (Distance < KINDA_SMALL_NUMBER)
        {
            It.SetScore(TestPurpose, FilterType, false, bExpected);
            continue;
        }
        
        // Normalize the direction
        DirectionToItem /= Distance;
        
        // Calculate the dot product between the actor's forward vector and direction to item
        // Dot product < 0 means the angle is > 90 degrees (behind in a basic sense)
        // We use the cosine threshold to determine more precisely what "behind" means
        float DotProduct = FVector::DotProduct(ContextForward, DirectionToItem);
        
        // Check if the item is behind the actor based on our angle threshold
        // Lower dot product = larger angle = more behind
        bool bIsBehind = (DotProduct <= -CosineThreshold) && (Distance >= MinimumDistanceBehind);
        
        // Calculate a normalized score (0.0 to 1.0) based on how "behind" the location is
        // -1.0 is directly behind, so we remap from [-1,1] to [0,1] where 1 is most behind
        //float Score = bIsBehind ? FMath::Abs((DotProduct + 1.0f) * 0.5f) : 0.0f;
        
        // Set the score for this item
        It.SetScore(TestPurpose, FilterType, bIsBehind, bExpected);
    }
}

FText UEnvQueryTest_IsBehindActor::GetDescriptionTitle() const
{
    return FText::Format(FText::FromString("Is Behind {0}"), FText::FromString(ActorContext ? ActorContext->GetName() : TEXT("NONE")));
}

FText UEnvQueryTest_IsBehindActor::GetDescriptionDetails() const
{
    return FText::Format(FText::FromString("Angle Threshold: {0}°, Min Distance: {1}, Using {2}D"),
        FText::AsNumber(BehindAngleThreshold),
        FText::AsNumber(MinimumDistanceBehind),
        FText::FromString(bUse2DDistance ? TEXT("2") : TEXT("3")));
}
