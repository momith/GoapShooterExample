#include "Controllers/GoapShooterAIController.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"
#include "Perception/AIPerceptionComponent.h"
#include "Characters/GoapShooterCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Kismet/KismetMathLibrary.h"
#include "AI/GOAP/Interfaces/GoapShooterPlanningComponent.h"
#include "AI/Execution/FlankExecutableAction.h"
#include "Components/FlankingComponent.h"
#include "Components/CoverComponent.h"
#include "Components/PerceptionMemoryComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Controllers/GoapMovementPlannerConfiguration.h"
#include "Controllers/GoapCombatPlannerConfiguration.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"

AGoapShooterAIController::AGoapShooterAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
    bWantsPlayerState = true; // bots should have a PlayerState too

    // create GOAP planning components
    MovementPlanningComponent = CreateDefaultSubobject<UGoapShooterPlanningComponent>(TEXT("MovementPlanningComponent"));
    MovementPlanningComponent->Controller = this;
    MovementPlanningComponent->ConfigClassToUse = UGoapMovementPlannerConfiguration::StaticClass();
    MovementPlanningComponent->bActivated = true; // can be changed for testing

    CombatPlanningComponent = CreateDefaultSubobject<UGoapShooterPlanningComponent>(TEXT("CombatPlanningComponent"));
    CombatPlanningComponent->Controller = this;
    CombatPlanningComponent->ConfigClassToUse = UGoapCombatPlannerConfiguration::StaticClass();
    CombatPlanningComponent->bActivated = true; // can be changed for testing
    
    // create perception component
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    SetPerceptionComponent(*PerceptionComponent);

    // configure sight sense
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1000.0f; 
    //SightConfig->SightRadius = 5.0f; // to be reverted, just for testing
    SightConfig->LoseSightRadius = 1200.0f;
    SightConfig->PeripheralVisionAngleDegrees = FieldOfViewAngle;
    SightConfig->SetMaxAge(0.1f); // keep it low to avoid memory leak
    SightConfig->AutoSuccessRangeFromLastSeenLocation = -1.0f;   
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
   
    // configure hearing sense
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 10000.0f;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->SetMaxAge(0.1f); // keep it low to avoid memory leak

    // configure damage sense
    DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(0.1f); // keep it low to avoid memory leak

    GetPerceptionComponent()->ConfigureSense(*SightConfig);
    GetPerceptionComponent()->ConfigureSense(*HearingConfig);
    GetPerceptionComponent()->ConfigureSense(*DamageConfig);
    
    GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());

    // create flanking component
    FlankingComponent = CreateDefaultSubobject<UFlankingComponent>(TEXT("FlankingComponent"));

    // create cover component
    CoverComponent = CreateDefaultSubobject<UCoverComponent>(TEXT("CoverComponent"));

    // create perception memory component
    PerceptionMemoryComponent = CreateDefaultSubobject<UPerceptionMemoryComponent>(TEXT("PerceptionMemoryComponent"));
}

TMap<FString, FGoapValue> AGoapShooterAIController::CalculateWorldState_Implementation()
{
    if (!FlankingComponent->IsInitialized() || !CoverComponent->IsInitialized() || !PerceptionMemoryComponent->IsInitialized())
    {
        return TMap<FString, FGoapValue>();
    }
    
    TMap<FString, FGoapValue> WorldState;
    
    // Add AI position
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::AIPosition), 
        FGoapValue(GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector));

    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::BestCoverLocation), 
        FGoapValue(CoverComponent->GetBestCoverLocation()));
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsCoverNearby), 
        FGoapValue(CoverComponent->IsCoverNearby()));
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsInCover), 
        FGoapValue(CoverComponent->IsInCover()));
    
    // Add enemy visibility state
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy), 
        FGoapValue(PerceptionMemoryComponent->CanSeeEnemy()));
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::WorldTimeLastEnemySeen), 
        FGoapValue(PerceptionMemoryComponent->GetLastSeenEnemyWorldTime()));
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::HasLastSeenEnemyLocation), 
        FGoapValue(!PerceptionMemoryComponent->GetLastSeenEnemyLocation().IsZero()));
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::AllEnemiesInSightAreKilled), 
        FGoapValue(PerceptionMemoryComponent->AreAllEnemiesInSightKilled()));
    
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsInDanger), 
        FGoapValue(PerceptionMemoryComponent->CanSeeEnemy()));
    //WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::FightInsteadOfFlight), 
    //    FGoapValue()); // TODO random from time to time fight mode activate where survival is not so important anymore?
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsAimedAt), 
        FGoapValue(PerceptionMemoryComponent->IsAimedAt()));

    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsFlankTargetLocationAvailable), 
        FGoapValue(FlankingComponent->IsFlankTargetLocationAvailable()));

    // Add the most interesting perceptions from perception memory
    if (PerceptionMemoryComponent->GetMostInterestingPerceivedEnemy()) 
    {
        WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsFocusedOnStrongPerceivedStimulus), 
            FGoapValue(PerceptionMemoryComponent->HasHalfStrongEnemyPerception()));

        WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsMinThresholdFulfilledForEnemyPerception), 
            FGoapValue(PerceptionMemoryComponent->IsMinThresholdFulfilledForEnemyPerception()));

        WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::IsLookingAtStimulus), 
            FGoapValue(PerceptionMemoryComponent->IsLookingAtStimulus()));
        WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::HasPerceivedStimulus), 
            FGoapValue(PerceptionMemoryComponent->HasPerceivedStimulus()));
        WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::HasInvestigatedStimulus), 
            FGoapValue(PerceptionMemoryComponent->HasInvestigatedStimulus()));
        WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::PerceivedStimulusType), 
            FGoapValue(PerceptionMemoryComponent->GetPerceivedStimulusType()));
        WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::PerceivedStimulusLocation), 
            FGoapValue(PerceptionMemoryComponent->GetPerceivedStimulusLocation()));
    }
    
    return WorldState;
}

bool AGoapShooterAIController::IsAnyCurrentExecutableActionRelatedToMostInterestingActor()
{
    bool AnyCurrentExecutableActionIsRelatedToMostInterestingActor = false;
    AActor* TheMostInterestingActor = PerceptionMemoryComponent->GetMostInterestingPerceivedEnemy();
    UExecutableAction* MovementExecutable = MovementPlanningComponent->CurrentExecutableAction;
    if (MovementExecutable && TheMostInterestingActor)
    {
        if (MovementExecutable->GetIsRelatedToMostInterestingActor()
            && MovementExecutable->GetMostInterestingActorWhenActionStarted() == TheMostInterestingActor)
        {
            AnyCurrentExecutableActionIsRelatedToMostInterestingActor = true;
        }
    }
    UExecutableAction* HandsExecutable = CombatPlanningComponent->CurrentExecutableAction;
    if (HandsExecutable && TheMostInterestingActor)
    {
        if (HandsExecutable->GetIsRelatedToMostInterestingActor()
            && HandsExecutable->GetMostInterestingActorWhenActionStarted() == TheMostInterestingActor)
        {
            AnyCurrentExecutableActionIsRelatedToMostInterestingActor = true;
        }
    }
    return AnyCurrentExecutableActionIsRelatedToMostInterestingActor;
}

void AGoapShooterAIController::BeginPlay()
{
    Super::BeginPlay();
}

void AGoapShooterAIController::OnSignificantWorldStateChange()
{
    MovementPlanningComponent->OnSignificantWorldStateChange();
    CombatPlanningComponent->OnSignificantWorldStateChange();
}

void AGoapShooterAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!GetPawn())
    {
        return;
    }
    
    RotatePawnTowardsEnemy(DeltaTime);
}

void AGoapShooterAIController::RotatePawnTowardsEnemy(float DeltaTime)
{
    //return; // can be disabled to test actions like WaitForPeekAction

    if (PerceptionMemoryComponent->HasHalfStrongEnemyPerception()) // MostInterestingActor
    {
        RotatePawnTowardsLocation(PerceptionMemoryComponent->GetMostInterestingPerceivedEnemy()->GetActorLocation(), DeltaTime);
    } 
    else if (!PerceptionMemoryComponent->GetLastSeenEnemyLocation().IsZero() 
        && (PerceptionMemoryComponent->GetLastSeenEnemyWorldTime() + RandSecondsRotateTowardsLastSeenLocation > GetWorld()->GetTimeSeconds())) 
    {
        RotatePawnTowardsLocation(PerceptionMemoryComponent->GetLastSeenEnemyLocation(), DeltaTime);
    }
    else 
    {
        // from time to time we want to recalculate the time to stay locked on last seen location
        RandSecondsRotateTowardsLastSeenLocation = FMath::FRandRange(
            MinSecondsRotateTowardsLastSeenLocation, MaxSecondsRotateTowardsLastSeenLocation);
    }
}

void AGoapShooterAIController::RotatePawnTowardsLocation(FVector Location, float DeltaTime)
{
    FVector AILocation = GetPawn()->GetActorLocation();
    
    FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(AILocation, Location);

    FRotator CurrentRotation = GetPawn()->GetActorRotation();
    FRotator TargetRotation(CurrentRotation.Pitch, LookAtRotation.Yaw, CurrentRotation.Roll);

    // >> Do not Interp! thats not working because we dont have a fixed AI location and fixed target rotation

    GetPawn()->SetActorRotation(TargetRotation);
}

