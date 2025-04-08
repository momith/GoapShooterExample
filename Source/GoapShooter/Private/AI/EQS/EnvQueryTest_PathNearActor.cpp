#include "AI/EQS/EnvQueryTest_PathNearActor.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AIController.h"

UEnvQueryTest_PathNearActor::UEnvQueryTest_PathNearActor()
{
    // Set default values
    ProximityThreshold = 300.0f;     // Default: consider paths within 300 units as "near"
    bWantPathsNearActor = false;     // Default: we want paths that are NOT near the actor (avoid)
    bUse2DDistance = true;           // Default to 2D distance (ignore height differences)
    MaxPathPointsToCheck = 20;       // Limit the number of path points to check for performance
    bUseSimpleDistanceCheck = false; // Default to using actual navigation path
    
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

void UEnvQueryTest_PathNearActor::RunTest(FEnvQueryInstance& QueryInstance) const
{
    UObject* QueryOwner = QueryInstance.Owner.Get();
    if (!QueryOwner)
    {
        return;
    }
    
    // Get the actor we want to check proximity to
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
        UE_LOG(LogTemp, Error, TEXT("Context actor is null. Cannot check proximity."));
        return;
    }
    
    // Get the context actor's location
    const FVector ContextLocation = ContextActor->GetActorLocation();
    
    // Get the querier (AI controller or pawn)
    AActor* QuerierActor = Cast<AActor>(QueryOwner);
    if (!QuerierActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Querier actor is null. Cannot check proximity."));
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
            UE_LOG(LogTemp, Error, TEXT("Querier pawn is null. Cannot check proximity."));
            return;
        }
    }
    else
    {
        // If not an AI controller, try casting directly to pawn
        QuerierPawn = Cast<APawn>(QuerierActor);
        if (!QuerierPawn)
        {
            UE_LOG(LogTemp, Error, TEXT("Querier pawn is null. Cannot check proximity."));
            return;
        }
    }
    
    // Get the navigation system
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(QuerierPawn->GetWorld());
    if (!NavSys)
    {
        UE_LOG(LogTemp, Error, TEXT("Navigation system is null. Cannot check proximity."));
        return;
    }
    
    // Get the querier's location
    const FVector QuerierLocation = QuerierPawn->GetActorLocation();
    
    // Go through all items in the query
    for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
    {
        // Get the location we're testing
        const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
        
        bool bPathPassesNearActor = false;
        
        if (bUseSimpleDistanceCheck)
        {
            // Simple straight-line check
            // Calculate the closest point on the line from querier to item
            const FVector QuerierToItem = ItemLocation - QuerierLocation;
            const float QuerierToItemLength = QuerierToItem.Size();
            
            if (QuerierToItemLength > KINDA_SMALL_NUMBER)
            {
                const FVector QuerierToItemDir = QuerierToItem / QuerierToItemLength;
                const FVector QuerierToContext = ContextLocation - QuerierLocation;
                
                // Project the vector from querier to context onto the path direction
                const float Projection = FVector::DotProduct(QuerierToContext, QuerierToItemDir);
                
                // Clamp the projection to the path length
                const float ClampedProjection = FMath::Clamp(Projection, 0.0f, QuerierToItemLength);
                
                // Calculate the closest point on the path to the context actor
                const FVector ClosestPointOnPath = QuerierLocation + QuerierToItemDir * ClampedProjection;
                
                // Calculate the distance from the context actor to the closest point
                FVector DistanceVector = ContextLocation - ClosestPointOnPath;
                if (bUse2DDistance)
                {
                    DistanceVector.Z = 0.0f;
                }
                
                const float Distance = DistanceVector.Size();
                
                // Check if the distance is below the threshold
                bPathPassesNearActor = Distance <= ProximityThreshold;
            }
        }
        else
        {
            // Use actual navigation path
            
            // Get the nav data
            ANavigationData* NavData = NavSys->GetNavDataForProps(QuerierPawn->GetNavAgentPropertiesRef());
            if (NavData)
            {
                // Create a path finding query with the correct parameters
                FPathFindingQuery PathFindingQuery(QuerierPawn, *NavData, QuerierLocation, ItemLocation);
                
                // Find the path
                FPathFindingResult PathResult = NavSys->FindPathSync(PathFindingQuery);
                if (PathResult.IsSuccessful() && PathResult.Path.IsValid())
                {
                    // Check if the path passes near the actor
                    bPathPassesNearActor = DoesPathPassNearActor(PathResult.Path->GetPathPoints(), ContextLocation, ProximityThreshold, bUse2DDistance);
                }
            }
        }
        
        // Determine if the item passes the test based on whether we want paths near or far from the actor
        const bool bPassesTest = (bWantPathsNearActor == bPathPassesNearActor);
        
        //UE_LOG(LogTemp, Log, TEXT("Path passes near actor: %s"), bPathPassesNearActor ? TEXT("true") : TEXT("false"));

        // Set the score for this item
        It.SetScore(TestPurpose, FilterType, bPassesTest, true); // bPassesTest ? 1.0f : 0.0f
    }
}

bool UEnvQueryTest_PathNearActor::DoesPathPassNearActor(const TArray<FNavPathPoint>& PathPoints, const FVector& ActorLocation, float Threshold, bool bUse2D) const
{
    const int32 NumPoints = FMath::Min(PathPoints.Num(), MaxPathPointsToCheck);
    
    // Check each path segment
    for (int32 i = 0; i < NumPoints - 1; ++i)
    {
        const FVector PointA = PathPoints[i].Location;
        const FVector PointB = PathPoints[i + 1].Location;
        
        // Calculate the closest point on the line segment to the actor
        const FVector SegmentVector = PointB - PointA;
        const float SegmentLength = SegmentVector.Size();
        
        if (SegmentLength > KINDA_SMALL_NUMBER)
        {
            const FVector SegmentDirection = SegmentVector / SegmentLength;
            const FVector PointAToActor = ActorLocation - PointA;
            
            // Project the vector from point A to actor onto the segment direction
            const float Projection = FVector::DotProduct(PointAToActor, SegmentDirection);
            
            // Clamp the projection to the segment length
            const float ClampedProjection = FMath::Clamp(Projection, 0.0f, SegmentLength);
            
            // Calculate the closest point on the segment to the actor
            const FVector ClosestPointOnSegment = PointA + SegmentDirection * ClampedProjection;
            
            // Calculate the distance from the actor to the closest point
            FVector DistanceVector = ActorLocation - ClosestPointOnSegment;
            if (bUse2D)
            {
                DistanceVector.Z = 0.0f;
            }
            
            const float Distance = DistanceVector.Size();
            
            // If any segment is close enough, the path passes near the actor
            if (Distance <= Threshold)
            {
                return true;
            }
        }
    }
    
    // No segment was close enough
    return false;
}

FText UEnvQueryTest_PathNearActor::GetDescriptionTitle() const
{
    return FText::Format(FText::FromString("Path {0} {1}"), 
        FText::FromString(bWantPathsNearActor ? TEXT("Near") : TEXT("Away From")),
        FText::FromString(ActorContext ? ActorContext->GetName() : TEXT("NONE")));
}

FText UEnvQueryTest_PathNearActor::GetDescriptionDetails() const
{
    return FText::Format(FText::FromString("Proximity: {0}, Using {1}D, Method: {2}"),
        FText::AsNumber(ProximityThreshold),
        FText::FromString(bUse2DDistance ? TEXT("2") : TEXT("3")),
        FText::FromString(bUseSimpleDistanceCheck ? TEXT("Simple Line") : TEXT("Nav Path")));
}
