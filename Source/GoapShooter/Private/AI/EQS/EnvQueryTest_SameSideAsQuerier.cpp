#include "AI/EQS/EnvQueryTest_SameSideAsQuerier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "AIController.h"

UEnvQueryTest_SameSideAsQuerier::UEnvQueryTest_SameSideAsQuerier()
{
    // Set default values
    bWantSameSide = true;       // Default: we want locations on the same side as the querier
    bUse2DCalculation = true;   // Default to 2D calculations (ignore height differences)
    bUseRightVector = true;    // Default to using right vector
    
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

void UEnvQueryTest_SameSideAsQuerier::RunTest(FEnvQueryInstance& QueryInstance) const
{
    UObject* QueryOwner = QueryInstance.Owner.Get();
    if (!QueryOwner)
    {
        UE_LOG(LogTemp, Error, TEXT("Query owner is null. Cannot check side."));
        return;
    }
    
    // Get the actor that defines the dividing plane
    TArray<AActor*> ContextActors;
    QueryInstance.PrepareContext(ActorContext, ContextActors);
    
    if (ContextActors.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No context actors found. Cannot check side."));
        return;
    }
    
    // We'll use the first actor from the context
    AActor* ContextActor = ContextActors[0];
    if (!ContextActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Context actor is null. Cannot check side."));
        return;
    }
    
    // Get the context actor's location and forward vector
    const FVector ContextLocation = ContextActor->GetActorLocation();
    
    // Determine which vector to use for the plane normal
    FVector PlaneNormal;
    if (bUseRightVector)
    {
        PlaneNormal = ContextActor->GetActorRightVector();
    }
    else
    {
        PlaneNormal = ContextActor->GetActorForwardVector();
    }
    
    // If using 2D, zero out the Z component
    if (bUse2DCalculation)
    {
        PlaneNormal.Z = 0.0f;
        
        // Ensure the vector is still normalized after zeroing Z
        if (!PlaneNormal.IsNearlyZero())
        {
            PlaneNormal.Normalize();
        }
        else
        {
            // If the vector becomes zero after removing Z, use X axis as fallback
            PlaneNormal = FVector(1.0f, 0.0f, 0.0f);
        }
    }
    
    // Get the querier (AI controller or pawn)
    AActor* QuerierActor = Cast<AActor>(QueryOwner);
    if (!QuerierActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Querier actor is null. Cannot check side."));
        return;
    }
    
    // If the querier is an AI controller, get its pawn
    APawn* QuerierPawn = nullptr;
    AAIController* AIController = Cast<AAIController>(QuerierActor);
    if (AIController)
    {
        QuerierPawn = AIController->GetPawn();
        if (!QuerierPawn)
        {
            UE_LOG(LogTemp, Error, TEXT("Querier pawn is null. Cannot check side."));
            return;
        }
    }
    else
    {
        // If not an AI controller, try casting directly to pawn
        QuerierPawn = Cast<APawn>(QuerierActor);
        if (!QuerierPawn)
        {
            UE_LOG(LogTemp, Error, TEXT("Querier pawn is null. Cannot check side."));
            return;
        }
    }
    
    // Get the querier's location
    const FVector QuerierLocation = QuerierPawn->GetActorLocation();
    
    // Go through all items in the query
    for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
    {
        // Get the location we're testing
        const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
        
        // Check if the querier and the test location are on the same side
        const bool bOnSameSide = ArePointsOnSameSide(ContextLocation, PlaneNormal, QuerierLocation, ItemLocation, bUse2DCalculation);
        
        // Determine if the item passes the test based on whether we want same side or opposite side
        const bool bPassesTest = (bWantSameSide == bOnSameSide);
        
        //UE_LOG(LogTemp, Verbose, TEXT("Location test: On same side: %s, Passes: %s"), 
        //    bOnSameSide ? TEXT("true") : TEXT("false"),
        //    bPassesTest ? TEXT("true") : TEXT("false"));
        
        // Set the score for this item
        It.SetScore(TestPurpose, FilterType, bPassesTest, true);
    }
}

bool UEnvQueryTest_SameSideAsQuerier::ArePointsOnSameSide(const FVector& PlaneOrigin, const FVector& PlaneNormal, 
                                                        const FVector& Point1, const FVector& Point2, bool bUse2D) const
{
    // Calculate vectors from plane origin to points
    FVector ToPoint1 = Point1 - PlaneOrigin;
    FVector ToPoint2 = Point2 - PlaneOrigin;
    
    // If using 2D, zero out the Z components
    if (bUse2D)
    {
        ToPoint1.Z = 0.0f;
        ToPoint2.Z = 0.0f;
    }
    
    // Calculate dot products to determine which side of the plane each point is on
    const float DotProduct1 = FVector::DotProduct(PlaneNormal, ToPoint1);
    const float DotProduct2 = FVector::DotProduct(PlaneNormal, ToPoint2);
    
    // Points are on the same side if both dot products have the same sign
    // (both positive or both negative)
    return (DotProduct1 * DotProduct2) > 0.0f;
}

FText UEnvQueryTest_SameSideAsQuerier::GetDescriptionTitle() const
{
    return FText::Format(FText::FromString("{0} Side As Querier Relative To {1}"), 
        FText::FromString(bWantSameSide ? TEXT("Same") : TEXT("Opposite")),
        FText::FromString(ActorContext ? ActorContext->GetName() : TEXT("NONE")));
}

FText UEnvQueryTest_SameSideAsQuerier::GetDescriptionDetails() const
{
    return FText::Format(FText::FromString("Using {0} Vector, {1}D Calculation"),
        FText::FromString(bUseRightVector ? TEXT("Right") : TEXT("Forward")),
        FText::FromString(bUse2DCalculation ? TEXT("2") : TEXT("3")));
}
