#include "Components/CoverComponent.h"
#include "Controllers/GoapShooterAIController.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "Curves/CurveFloat.h"

UCoverComponent::UCoverComponent()
{
	// load BP assets because BP and C++ synchronization is buggy
	static ConstructorHelpers::FObjectFinder<UEnvQuery> EqsQueryForCoverLocationsAssetFinder(
	    TEXT("/Game/AI/EQ_FindCover"));
	if (EqsQueryForCoverLocationsAssetFinder.Succeeded())
	{
		EqsQueryForCoverLocations = EqsQueryForCoverLocationsAssetFinder.Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find EqsQueryForCoverLocations asset."));
	}
}

void UCoverComponent::BeginPlay()
{
	Super::BeginPlay();

    AIController = Cast<AGoapShooterAIController>(GetOwner());
    
	GetWorld()->GetTimerManager().SetTimer(CoverUpdateTimerHandle, this,
		&UCoverComponent::UpdateCurrentBestCover, 1.0f, true);

    bIsInitialized = true;
}


FVector UCoverComponent::GetBestCoverLocation()
{
    return CurrentCoverLocations.Num() > 0 ? CurrentCoverLocations[0] : FVector::ZeroVector;
}

bool UCoverComponent::IsCoverNearby()
{
    if (CurrentCoverLocations.Num() == 0)
    {
        return false;
    }
    for (const FVector& CoverLocation : CurrentCoverLocations)
    {
        if ((CoverLocation - AIController->GetPawn()->GetActorLocation()).Size() < CoverRadiusConsideredAsNearby)
        {
            return true;
        }
    }
    return false;
}

bool UCoverComponent::IsInCover()
{
    if (CurrentCoverLocations.Num() == 0)
    {
        return false;
    }
    for (const FVector& CoverLocation : CurrentCoverLocations)
    {
        if ((CoverLocation - AIController->GetPawn()->GetActorLocation()).Size() < CoverRadiusConsideredAsInCover)
        {
            return true;
        }
    }
    return false;
}

void UCoverComponent::VisualizeBestCover()
{
    if (bDebugDrawCoverPoints && CurrentCoverLocations.Num() > 0 && !CurrentCoverLocations[0].IsZero())
    {
        DrawDebugSphere(GetWorld(), CurrentCoverLocations[0], 50.0f, 8, FColor::Green, false, 2.0f);
    }
}

void UCoverComponent::UpdateCurrentBestCover()
{
    if (!IsValid(AIController) || !AIController->GetPawn())
    {
        return;
    }
    
    // TODO consider only these enemies ? see BP file "EQ_C_AllOtherPlayers"
    // TArray<AActor*> PerceivedEnemies;
    // for (const TPair<AActor*, FPerceptionData>& Pair : PerceptionMemory)
    // {
    //     AActor* Enemy = Pair.Key;
    //     if (Enemy && Pair.Value.PerceptionType == EPerceptionType::Visual)
    //     {
    //         PerceivedEnemies.Add(Enemy);
    //     }
    // }

    if (!EqsQueryForCoverLocations)
    {
        UE_LOG(LogTemp, Error, TEXT("No EQS query for cover locations!"));
        return;
    }
    if (bIsWaitingForQueryToFinish) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Wont start EQS query request because previous one is still pending!"));
        return;
    }

    bIsWaitingForQueryToFinish = true;
    FEnvQueryRequest CoverLocationsQueryRequest = FEnvQueryRequest(EqsQueryForCoverLocations, AIController->GetPawn());
    CoverLocationsQueryRequest.Execute(EEnvQueryRunMode::AllMatching, this, &UCoverComponent::OnCoverLocationsQueryFinished);
}

void UCoverComponent::OnCoverLocationsQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
    if (!IsValid(AIController) || !AIController->GetPawn())
    {
        return;
    }
    
    bIsWaitingForQueryToFinish = false;
    
    if (!Result.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid EQS query result!"));
        //CachedBestCoverLocation = FVector::ZeroVector;
        CurrentCoverLocations.Empty();
        return;
    }
    
    if (Result->IsSuccessful())
    {       
        int32 NumResults = Result->Items.Num();
        if (NumResults == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("No cover locations found!"));
            //CachedBestCoverLocation = FVector::ZeroVector;
            CurrentCoverLocations.Empty();
            return;
        }
        CurrentCoverLocations.Empty();
        for (int32 i = 0; i < NumResults; i++)
        {
            CurrentCoverLocations.Add(Result->GetItemAsLocation(i));
        }
        // get best scored cover location
        //CachedBestCoverLocation = Result->GetItemAsLocation(0);
        //CurrentCoverLocations = Result->Items; // Result->GetAllItemsAsLocation();

        //UE_LOG(LogTemp, Log, TEXT("Best cover location found: %s"), *CachedBestCoverLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EQS query failed!"));
        //CachedBestCoverLocation = FVector::ZeroVector;
        CurrentCoverLocations.Empty();
    }

    VisualizeBestCover();
}


