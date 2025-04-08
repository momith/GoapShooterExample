#include "Components/FlankingComponent.h"
#include "Controllers/GoapShooterAIController.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "AI/Navigation/FlankingNavModifier.h"
#include "NavigationSystem.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "Curves/CurveFloat.h"
#include "Tasks/Task.h"
#include "DrawDebugHelpers.h"
#include "AI/Navigation/NavigationTypes.h"
#include "AI/Navigation/NavQueryFilter_AvoidFlanking.h"
#include "NavigationPath.h"
#include "Components/PerceptionMemoryComponent.h"

UFlankingComponent::UFlankingComponent()
{
    // load BP assets because BP and C++ synchronization is buggy
    static ConstructorHelpers::FObjectFinder<UEnvQuery> EqsQueryForFlankLocationsAssetFinder(
        TEXT("/Game/AI/EQ_FindFlankTargetLocation"));
    if (EqsQueryForFlankLocationsAssetFinder.Succeeded())
    {
        EqsQueryForFlankLocations = EqsQueryForFlankLocationsAssetFinder.Object;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find EqsQueryForFlankLocations asset for the UFlankingComponent."));
    }
}

void UFlankingComponent::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AGoapShooterAIController>(GetOwner());

    // TODO refactor candidate: we could batch process this for all bots at once, would that make things faster?
    GetWorld()->GetTimerManager().SetTimer(FlankUpdateTimerHandle, this,
        &UFlankingComponent::UpdateCurrentBestFlankTargetLocation, 1.0f, true, 0.5f);

    bIsInitialized = true;
}

bool UFlankingComponent::IsFlankTargetLocationAvailable()
{
    return FlankPath.Num() > 0 
        && (FlankPath[FlankPath.Num() - 1] - AIController->GetPawn()->GetActorLocation()).Size() > MinFlankDistance
        //&& (CurrentBestFlankTargetLocation - GetPawn()->GetActorLocation()).Size() < MaxFlankDistance
        //&& HasStrongEnemyPerception()
        && LastExecutedFlankTime + FlankCoolDown < GetWorld()->GetTimeSeconds()
        && AIController->GetPerceptionMemoryComponent()->GetMostInterestingPerceivedEnemy();
}

void UFlankingComponent::UpdateCurrentBestFlankTargetLocation()
{
    if (!IsValid(AIController) || !AIController->GetPawn())
    {
        return;
    }
    
    if (!EqsQueryForFlankLocations)
    {
        UE_LOG(LogTemp, Error, TEXT("No EQS query for flank locations!"));
        return;
    }
    if (bIsWaitingForFlankQueryToFinish) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Wont start flank EQS query request because previous one is still pending!"));
        return;
    }
    if (bIsWaitingForAsyncFlankCheckToFinish)
    {
        UE_LOG(LogTemp, Warning, TEXT("Wont start flank EQS query request because previous flank check is still pending!"));
        return;
    }
    if (bIsFlanking) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Wont start flank EQS query request because previous flank is still pending!"));
        return;
    }
    //if (LastExecutedFlankTime + FlankCoolDown > GetWorld()->GetTimeSeconds())
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("Wont start flank EQS query request because cooldown is still pending!"));
    //    return;
    //}

    bIsWaitingForFlankQueryToFinish = true;
    FEnvQueryRequest FlankLocationsQueryRequest = FEnvQueryRequest(EqsQueryForFlankLocations, AIController->GetPawn());
    // TODO refactor candidate: we could return best 25 percent and try to score them here with logic that we cannot execute in EQS
    FlankLocationsQueryRequest.Execute(EEnvQueryRunMode::SingleResult, this, &UFlankingComponent::OnFlankLocationsQueryFinished);
}

AActor* UFlankingComponent::GetOrSpawnFlankBlockingVolume()
{
    if (FlankBlockingVolume)
    {
        return FlankBlockingVolume;
    }
    // spawn flank blocking volume
    FActorSpawnParameters SpawnParams;
    FVector SpawnLocation = FVector::ZeroVector;
    SpawnLocation.Z = -5000.0f;
    FlankBlockingVolume = GetWorld()->SpawnActor<AFlankingNavModifier>(AFlankingNavModifier::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    return FlankBlockingVolume;
}

void UFlankingComponent::OnFlankLocationsQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
    if (!IsValid(AIController) || !AIController->GetPawn())
    {
        return;
    }
    
    bIsWaitingForFlankQueryToFinish = false;

    // if (bIsFlanking)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("Async answer to best flank location is thrown away because flank is still pending!"));
    //     CurrentBestFlankTargetLocation = FVector::ZeroVector;
    //     FlankPath.Empty();
    //     return;
    // }

    if (!Result.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid EQS query result!"));
        FlankPath.Empty();
        return;
    }
    
    if (Result->IsSuccessful())
    {       
        int32 NumResults = Result->Items.Num();
        if (NumResults == 0)
        {
            //UE_LOG(LogTemp, Warning, TEXT("No flank locations found!"));
            FlankPath.Empty();
            return;
        }

        // get best scored flank location
        FVector CandidateForCurrentBestFlankTargetLocation = Result->GetItemAsLocation(0);

        // async task: check if flank location(s) are navigatable
        // but first ensure that flank blocking volume actor gets spawned in game thread before we trigger the task
        AActor* SpawnedModifier = GetOrSpawnFlankBlockingVolume(); 
        if (!SpawnedModifier) {
            FlankPath.Empty();
            return;
        }

        bIsWaitingForAsyncFlankCheckToFinish = true;
        UE::Tasks::Launch(TEXT("Flank Evaluation Trigger Task"), [this, CandidateForCurrentBestFlankTargetLocation, SpawnedModifier]()
        {
            if (!IsValid(AIController) || !AIController->GetPawn())
            {
                return;
            }
            
            // why do we directly directing the task to the game thread?
            // the idea is that this async background task is first queued and only executed if resources are available
            // this is then an indicator that also the game thread is not too busy currently.
            // not sure if this works though...
            
            //AsyncTask(ENamedThreads::GameThread, [this, CandidateForCurrentBestFlankTargetLocation, SpawnedModifier]() // asynctask resulted in stutter during testing (multiple queued up and executed at once)
            //{
            FFunctionGraphTask::CreateAndDispatchWhenReady([this, CandidateForCurrentBestFlankTargetLocation, SpawnedModifier]()
            {
                if (!IsValid(AIController) || !AIController->GetPawn())
                {
                    return;
                }
                
                bIsWaitingForAsyncFlankCheckToFinish = false;

                // if (bIsFlanking)
                // {
                //     UE_LOG(LogTemp, Warning, TEXT("Async answer to check best flank location is thrown away because AI is actively flanking!"));
                //     CurrentBestFlankTargetLocation = FVector::ZeroVector;
                //     FlankPath.Empty();
                //     return;
                // }

                TArray<FVector> Path;

                if (AActor* MostInterestingActor = AIController->GetPerceptionMemoryComponent()->GetMostInterestingPerceivedEnemy()) {
                    Path = FindFlankPathToLocation(CandidateForCurrentBestFlankTargetLocation, MostInterestingActor->GetActorLocation());
                }

                //UE_LOG(LogTemp, Log, TEXT("Flank calculation complete: %d path points"), Path.Num());
                
                if (Path.Num() == 0) 
                {
                    FlankPath = Path;
                }
                else 
                {
                    // TODO refactor candidate: we could score the path: is it a good path, e.g. are (enough) cover points on this path (info from CoverComponent)?
                    
                    FlankPath = Path;

                    if (bDebugDrawEachPossibleFlankingPath) DrawDebugPath(FlankPath, 1.0f);
                }
            
            
            //});
            }, TStatId(), nullptr, ENamedThreads::GameThread);

        }, UE::Tasks::ETaskPriority::BackgroundLow); // Ensures execution only when system isn't overloaded
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EQS query for finding flank target location failed!"));
        FlankPath.Empty();
    }
}

void UFlankingComponent::OnFlankStarted()
{
	bIsFlanking = true;
    if (bDebugDrawExecutingFlankingPath)
    {
        DrawDebugPath(FlankPath, 10.0f);
    }
}

void UFlankingComponent::OnFlankEnded()
{
	LastExecutedFlankTime = GetWorld()->GetTimeSeconds();
	bIsFlanking = false;
	UE_LOG(LogTemp, Error, TEXT("Flank ended at %f"), LastExecutedFlankTime);
}

TArray<FVector> UFlankingComponent::FindFlankPathToLocation(FVector PathEndLocation, FVector ActualEnemyLocation) {

    if (!AIController || !AIController->GetPawn())
    {
        return TArray<FVector>();
    }

    AActor* FlankBlockingNavVolume = GetOrSpawnFlankBlockingVolume();
    
    FVector PathStartLocation = AIController->GetPawn()->GetActorLocation();
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(AIController->GetWorld());
    
    PlaceAndScaleBigNavModifierInCenter(PathStartLocation, ActualEnemyLocation);

    NavSystem->Build();
    
    UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(AIController->GetWorld(), 
        PathStartLocation, PathEndLocation, AIController->GetPawn(), UNavQueryFilter_AvoidFlanking::StaticClass());

    // Restore the volume's original location, rotation, and scale (place it somewhere outside game world)
    FlankBlockingNavVolume->SetActorLocation(FVector(-100000.0f, -100000.0f, -100000.0f));
    FlankBlockingNavVolume->SetActorRotation(FQuat::Identity);
    FlankBlockingNavVolume->SetActorScale3D(FVector(-100000.0f, -100000.0f, -100000.0f));

    NavSystem->Build();

    if (!IsValid(NavPath) || !NavPath->IsValid()) {
        return TArray<FVector>();
    }

    // just in case: project points to nav mesh
    TArray<FVector> FoundPathPoints;
    for (const FVector& Point : NavPath->PathPoints)
    {
        FNavLocation NavLoc;
        if (NavSystem->ProjectPointToNavigation(Point, NavLoc))
        {
            FoundPathPoints.Add(NavLoc.Location);
        }
    }

    return FoundPathPoints;
}

void UFlankingComponent::PlaceAndScaleBigNavModifierInCenter(FVector NavModifierSourceLocation, FVector NavModifierTargetLocation)
{
    AActor* SpawnedModifier = GetOrSpawnFlankBlockingVolume();
    
    // Calculate the direction vector from source to target
    FVector Direction = (NavModifierTargetLocation - NavModifierSourceLocation).GetSafeNormal();
    
    // Calculate the total distance between source and target
    float TotalDistance = FVector::Distance(NavModifierSourceLocation, NavModifierTargetLocation);
    
    // Default width of the volume
    float VolumeWidth = 300.0f;
    
    // Margin from both source and target (100 units each)
    float Margin = 200.0f;
    
    // Calculate the length of the volume (total distance minus margins on both sides)
    float VolumeLength = FMath::Max(TotalDistance - (2 * Margin), 0.0f);
    
    // If the distance is too small, don't create a volume
    if (VolumeLength <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Source and target are too close to create a nav modifier volume"));
        return;
    }
    
    // Calculate the center point between source and target (accounting for margins)
    FVector CenterPoint = NavModifierSourceLocation + Direction * (Margin + VolumeLength / 2.0f);
    
    // Calculate the rotation to align the box with the direction vector
    FRotator Rotation = Direction.Rotation();
    Rotation.Pitch = 0.0f;
    Rotation.Roll = 0.0f;

    // Spawn the nav modifier actor at the center point with the calculated rotation
    UWorld* World = GEngine->GetWorldFromContextObjectChecked((const UObject*)GEngine->GameViewport);
    FActorSpawnParameters SpawnParams;
    
    SpawnedModifier->SetActorLocationAndRotation(CenterPoint, Rotation);
    
    // Set the scale of the box to match our desired dimensions
    // The default box is 100x100x100, so we scale accordingly
    FVector Scale(VolumeLength / 100.0f, VolumeWidth / 100.0f, 1.0f);

    Scale.Z *= 300;

    SpawnedModifier->SetActorScale3D(Scale);
    
    // Debug visualization
    if (bDebugDrawFlankAvoidanceBox)
    {
        DrawDebugBox(
            World, 
            CenterPoint, 
            FVector(VolumeLength / 2.0f, VolumeWidth / 2.0f, 50.0f), 
            Rotation.Quaternion(), 
            FColor::Red, 
            false, 
            5.0f, 
            0, 
            2.0f
        );
    }

}

void UFlankingComponent::DrawDebugPath(TArray<FVector> Path, float Duration)
{
    if (Path.Num() < 2 || !AIController || !AIController->GetWorld())
    {
        return;
    }

    UWorld* World = AIController->GetWorld();
    
    // Draw a sphere at each point in the path
    const FColor PointColor = FColor::Green;
    const float PointSize = 10.0f;
    
    // Draw lines connecting the points
    const FColor LineColor = FColor::Yellow;
    const float LineThickness = 2.0f;
    
    // Draw the first point with a different color to indicate the start
    DrawDebugSphere(World, Path[0], PointSize, 8, FColor::Blue, false, Duration, 0, 1.0f);
    
    // Draw the last point with a different color to indicate the end
    DrawDebugSphere(World, Path[Path.Num() - 1], PointSize, 8, FColor::Red, false, Duration, 0, 1.0f);
    
    // Draw all intermediate points and lines
    for (int32 i = 1; i < Path.Num(); i++)
    {
        // Draw sphere at current point
        DrawDebugSphere(World, Path[i], PointSize, 8, PointColor, false, Duration, 0, 1.0f);
        
        // Draw line from previous point to current point
        DrawDebugLine(World, Path[i-1], Path[i], LineColor, false, Duration, 0, LineThickness);
    }
    
    // Draw direction arrows along the path
    for (int32 i = 1; i < Path.Num(); i++)
    {
        FVector Direction = (Path[i] - Path[i-1]).GetSafeNormal();
        FVector ArrowLocation = Path[i-1] + Direction * FVector::Dist(Path[i-1], Path[i]) * 0.5f;
        
        // Draw an arrow in the middle of each line segment
        DrawDebugDirectionalArrow(World, ArrowLocation, ArrowLocation + Direction * 50.0f, 
                                 20.0f, LineColor, false, Duration, 0, 3.0f);
    }
}

// void AGoapShooterAIController::TestFlank(APawn* TargetToFlank)
// {
//     if (FlankActionForTesting) {
//         FlankActionForTesting->AbortAction();
//     }
//     FlankActionForTesting = NewObject<UFlankExecutableAction>();
//     FlankActionForTesting->Initialize(this, nullptr);
//
//     // Get the target's rotation to calculate the right vector
//     //FRotator TargetRotation = TargetToFlank->GetActorRotation();
//     //FVector TargetRightVector = TargetRotation.Vector().RotateAngleAxis(90.0f, FVector::UpVector);
//     
//     // Position 200 units to the right of the target based on its forward direction
//     //CurrentBestFlankTargetLocation = TargetToFlank->GetActorLocation() + (TargetRightVector * 200.0f);
//
//     FlankActionForTesting->TickAction(0.0f);
// }
//
// void AGoapShooterAIController::TestNavRebuild()
// {
//     UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
//     NavSystem->Build();
// }
