#include "Controllers/GoapShooterAIController.h"
#include "AI/GOAP/WorldStates/GoapWorldStateKeys.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/GoapShooterCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Kismet/KismetMathLibrary.h"

AGoapShooterAIController::AGoapShooterAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{   
    // configure perception component
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1000.0f;
    SightConfig->LoseSightRadius = 1200.0f;
    SightConfig->PeripheralVisionAngleDegrees = FieldOfViewAngle;
    SightConfig->SetMaxAge(0.0f); // 0.0f means never expires

    SightConfig->AutoSuccessRangeFromLastSeenLocation = -1.0f;
    
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
  
    PerceptionComponent->ConfigureSense(*SightConfig);
    
    SetPerceptionComponent(*PerceptionComponent);

    GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AGoapShooterAIController::TargetActorsPerceptionUpdated);
}

TMap<FString, FGoapValue> AGoapShooterAIController::CalculateWorldState_Implementation()
{
    TMap<FString, FGoapValue> WorldState;

    // Add AI position
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::AIPosition), 
        FGoapValue(GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector));

    // Add enemy visibility state
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::CanSeeEnemy), 
        FGoapValue(CanSeeEnemy()));
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::WorldTimeLastEnemySeen), 
        FGoapValue(LastSeenEnemyWorldTime));
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::HasLastSeenEnemyLocation), 
        FGoapValue(LastSeenEnemyLocation != FVector::ZeroVector));
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::AllEnemiesInSightAreKilled), 
        FGoapValue(bLastEnemyInSightKilled && PerceptionMemory.Num() == 0));
    
    // Add the most interesting perceptions from perception memory
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::HasPerceivedStimulus), 
        FGoapValue(MostInterestingPerception.PerceptionType != EPerceptionType::None));
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::HasInvestigatedStimulus), 
        FGoapValue(MostInterestingPerception.bHasBeenInvestigated));
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::PerceivedStimulusType), 
        FGoapValue(FPerceptionTypeUtils::ToString(MostInterestingPerception.PerceptionType)));
    WorldState.Add(FGoapWorldStateKeyUtils::ToString(EGoapWorldStateKey::PerceivedStimulusLocation), 
        FGoapValue(MostInterestingPerception.LastPerceivedLocation));

    return WorldState;
}

void AGoapShooterAIController::TargetActorsPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
    if (TargetActor == GetPawn() || !TargetActor->ActorHasTag("ToBePerceivedByAI"))
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed()) {

        AddOrUpdatePerceptionData(TargetActor, Stimulus);
        
    } else {

        RemovePerceptionData(TargetActor, Stimulus);
    }

    CleanUpPerceptionMemory(); // cleaning up from time to time to mitigate risk of memory leaks

    ChooseMostInterestingPerceptionData();
}

void AGoapShooterAIController::ChooseMostInterestingPerceptionData()
{
    // reset first
    MostInterestingPerception = FPerceptionData();
    
    if (PerceptionMemory.Num() == 0)
    {
        return;
    }
    
    // priority order: Damage > Visual > Audio
    EPerceptionType HighestPriorityType = EPerceptionType::None;
    float HighestStrength = -1.0f;
    float MostRecentTime = -1.0f;
    AActor* MostInterestingActor = nullptr;
    
    for (const auto& Pair : PerceptionMemory)
    {
        AActor* Actor = Pair.Key;
        const FPerceptionData& Perception = Pair.Value;
        
        if (Perception.bHasBeenInvestigated)
        {
            continue;
        }
        
        bool bIsMoreInteresting = false;
        
        // prioritize according to perception type
        if (Perception.PerceptionType == EPerceptionType::Damage && 
            HighestPriorityType != EPerceptionType::Damage)
        {
            bIsMoreInteresting = true;
        }
        else if (Perception.PerceptionType == EPerceptionType::Visual && 
                 HighestPriorityType != EPerceptionType::Damage)
        {
            bIsMoreInteresting = true;
        }
        else if (Perception.PerceptionType == EPerceptionType::Audio && 
                 HighestPriorityType == EPerceptionType::None)
        {
            bIsMoreInteresting = true;
        }
        else if (Perception.PerceptionType == HighestPriorityType && 
                 Perception.Strength > HighestStrength)
        {
            bIsMoreInteresting = true;
        }
        else if (Perception.PerceptionType == HighestPriorityType && 
                 Perception.Strength == HighestStrength && 
                 Perception.LastPerceivedTime > MostRecentTime)
        {
            bIsMoreInteresting = true;
        }
        
        if (bIsMoreInteresting)
        {
            HighestPriorityType = Perception.PerceptionType;
            HighestStrength = Perception.Strength;
            MostRecentTime = Perception.LastPerceivedTime;
            MostInterestingActor = Actor;
        }
    }
    
    // set the most interesting perception
    if (MostInterestingActor)
    {
        MostInterestingPerception = PerceptionMemory[MostInterestingActor];
        
        // UE_LOG(LogTemp, Log, TEXT("AI %s chose %s as most interesting perception at location %s"),
        //     *GetName(),
        //     *FPerceptionTypeUtils::ToString(MostInterestingPerception.PerceptionType),
        //     *MostInterestingPerception.LastPerceivedLocation.ToString());
    }
}

void AGoapShooterAIController::BeginPlay()
{
    Super::BeginPlay();
}

void AGoapShooterAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!GetPawn())
    {
        return;
    }

    UpdateCurrentEnemy();
    
    RotatePawnTowardsEnemy(DeltaTime);
    
    DrawPerceptionVisualization();
}

void AGoapShooterAIController::RotatePawnTowardsEnemy(float DeltaTime)
{
    //return; // can be disabled to test actions like WaitForPeekAction

    if (CurrentEnemy && GetPawn())
    {
        FVector EnemyLocation = CurrentEnemy->GetActorLocation();
        FVector AILocation = GetPawn()->GetActorLocation();
        
        FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(AILocation, EnemyLocation);
        
        APawn* ControlledPawn = GetPawn();
        FRotator CurrentRotation = ControlledPawn->GetActorRotation();
        FRotator TargetRotation = FRotator(CurrentRotation.Pitch, LookAtRotation.Yaw, CurrentRotation.Roll);
        
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f);
        
        ControlledPawn->SetActorRotation(NewRotation);
    }
}

bool AGoapShooterAIController::CanSeeEnemy() const
{
    return CurrentEnemy != nullptr;
}

AGoapShooterCharacter* AGoapShooterAIController::GetCurrentEnemy() const
{
    return Cast<AGoapShooterCharacter>(CurrentEnemy);
}

void AGoapShooterAIController::OnCurrentEnemyKilled()
{
    bLastEnemyInSightKilled = true;
}

void AGoapShooterAIController::UpdateCurrentEnemy()
{
    if (!CurrentEnemy)
    {
        PickNewEnemyFromMemory();
        if (CurrentEnemy)
        {
            LastSeenEnemyLocation = FVector::ZeroVector;
            LastSeenEnemyWorldTime = -FLT_MAX;
            bLastEnemyInSightKilled = false;
        }
    }
    else
    {
        AActor* CurrentEnemyTemp = CurrentEnemy;
        if (!IsCurrentEnemyStillInClearSight())
        {
            PickNewEnemyFromMemory();
            if (CurrentEnemy)
            {
                LastSeenEnemyLocation = FVector::ZeroVector;
                LastSeenEnemyWorldTime = -FLT_MAX;
                bLastEnemyInSightKilled = false;
            } else
            {
                LastSeenEnemyLocation = CurrentEnemyTemp->GetActorLocation();
                LastSeenEnemyWorldTime = GetWorld()->GetTimeSeconds();
            }
        }
    }
}

bool AGoapShooterAIController::IsCurrentEnemyStillInClearSight()
{
    bool bCurrentEnemyValid = false;
    if (PerceptionMemory.Contains(CurrentEnemy))
    {
        const FPerceptionData& CurrentEnemyPerception = PerceptionMemory[CurrentEnemy];
        if (CurrentEnemyPerception.PerceptionType == EPerceptionType::Visual)
        {
            bCurrentEnemyValid = IsActorInFieldOfView(CurrentEnemy);
        }
    }
    return bCurrentEnemyValid;
}

void AGoapShooterAIController::PickNewEnemyFromMemory()
{
    CurrentEnemy = nullptr;
    
    AActor* BestVisualPerceptionActor = nullptr;
    
    for (const auto& Pair : PerceptionMemory)
    {
        const FPerceptionData& PerceptionData = Pair.Value;
        
        if (IsActorInFieldOfView(PerceptionData.PerceivedActor))
        {
            BestVisualPerceptionActor = PerceptionData.PerceivedActor;
        }
    }
    
    if (BestVisualPerceptionActor != nullptr)
    {
        CurrentEnemy = BestVisualPerceptionActor;

        const FPerceptionData& SelectedPerception = PerceptionMemory[BestVisualPerceptionActor];
        
        bLastEnemyInSightKilled = false;
        
        //UE_LOG(LogTemp, Log, TEXT("AI %s selected visual actor %s as enemy"),
        //    *GetName(),
        //    *CurrentEnemy->GetName());
    }
}

bool AGoapShooterAIController::IsActorInFieldOfView(AActor* TargetActor)
{
    if (!GetPawn() || !TargetActor)
    {
        return false;
    }
    
    FVector PawnLocation = GetPawn()->GetActorLocation();
    FVector TargetLocation = TargetActor->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - PawnLocation).GetSafeNormal();
    
    FVector PawnForward = GetPawn()->GetActorForwardVector();
    
    float DotProduct = FVector::DotProduct(PawnForward, DirectionToTarget);
    float AngleRadians = FMath::Acos(DotProduct);
    float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);
    
    return AngleDegrees <= (FieldOfViewAngle * 0.5f);
}

void AGoapShooterAIController::OnStimulusInvestigated(const FPerceptionData& Stimulus)
{
    // Mark stimulus as investigated
    if (PerceptionMemory.Contains(Stimulus.PerceivedActor))
    {
        FPerceptionData& PerceptionData = PerceptionMemory[Stimulus.PerceivedActor];
        if (Stimulus.LastPerceivedLocation.Equals(PerceptionData.LastPerceivedLocation, 1.0f))
        {
            PerceptionData.bHasBeenInvestigated = true;
        }
    }
}

void AGoapShooterAIController::AddOrUpdatePerceptionData(AActor* PerceivedActor, FAIStimulus Stimulus)
{
    // dont override existing visual perception (visual is the most important)
    if (PerceptionMemory.Contains(PerceivedActor) 
        && PerceptionMemory[PerceivedActor].PerceptionType == EPerceptionType::Visual 
        && DeterminePerceptionType(Stimulus) != EPerceptionType::Visual)
    {
        return;
    }

    FPerceptionData& PerceptionData = PerceptionMemory.FindOrAdd(PerceivedActor);
    PerceptionData.PerceivedActor = PerceivedActor;
    PerceptionData.LastPerceivedLocation = PerceivedActor->GetActorLocation();
    PerceptionData.LastPerceivedTime = GetWorld()->GetTimeSeconds();
    PerceptionData.PerceptionType = DeterminePerceptionType(Stimulus);
    PerceptionData.Strength = Stimulus.Strength;
    PerceptionData.bHasBeenInvestigated = false;
}

void AGoapShooterAIController::RemovePerceptionData(AActor* UnPerceivedActor, FAIStimulus Stimulus)
{
    if (PerceptionMemory.Contains(UnPerceivedActor))
    {
        EPerceptionType PerceptionType = DeterminePerceptionType(Stimulus);
        FPerceptionData& StoredPerception = PerceptionMemory[UnPerceivedActor];
        if (StoredPerception.PerceptionType == PerceptionType)
        {
            PerceptionMemory.Remove(UnPerceivedActor);
        }
    }
}

void AGoapShooterAIController::CleanUpPerceptionMemory()
{
    // remove entries with invalid actors
    for (auto It = PerceptionMemory.CreateIterator(); It; ++It)
    {
        if (!IsValid(It->Key))
        {
            It.RemoveCurrent();
        }
    }

    // remove entries with expired stimuli
    for (auto It = PerceptionMemory.CreateIterator(); It; ++It)
    {
        if (It->Value.LastPerceivedTime + PerceptionDataLifetimeInSeconds < GetWorld()->GetTimeSeconds())
        {
            It.RemoveCurrent();
        }
    }
}

EPerceptionType AGoapShooterAIController::DeterminePerceptionType(const FAIStimulus& Stimulus) const
{
    // Determine the type of perception based on the stimulus type
    if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
    {
        return EPerceptionType::Visual;
    }
    else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
    {
        return EPerceptionType::Audio;
    }
    else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
    {
        return EPerceptionType::Damage;
    }
    
    return EPerceptionType::None;
}

void AGoapShooterAIController::DrawPerceptionVisualization()
{
    if (!bVisualizePerception || !GetPawn() || !SightConfig)
    {
        return;
    }

    // Get the pawn's location and forward vector
    FVector PawnLocation = GetPawn()->GetActorLocation();
    FVector PawnForward = GetPawn()->GetActorForwardVector();
    
    const float SightRadius = SightConfig->SightRadius;
    const float PeripheralVisionAngle = SightConfig->PeripheralVisionAngleDegrees;
    
    // Determine the color based on whether an enemy is detected
    // IMPORTANT: Using our own check for visualization purposes ONLY
    bool bIsActorPerceivedForVisualization = PerceptionMemory.Num() > 0;
    
    FColor ConeColor = bIsActorPerceivedForVisualization ? FColor::Green : FColor::Red;
    
    // Draw the sight cone
    DrawDebugCone(
        GetWorld(),                // World
        PawnLocation,              // Origin
        PawnForward,               // Direction
        SightRadius,               // Length
        FMath::DegreesToRadians(PeripheralVisionAngle * 0.5f), // Angle in radians (half the total angle)
        0.0f,                     // Inner angle in radians
        12,                        // Number of sides
        ConeColor,                 // Color
        false,                     // Persistent lines
        0.0f                       // Lifetime (0 = one frame)
    );
    
    // If there's a current enemy, draw a line to them
    if (CurrentEnemy)
    {
        DrawDebugLine(
            GetWorld(),
            PawnLocation,
            CurrentEnemy->GetActorLocation(),
            bIsActorPerceivedForVisualization ? FColor::Green : FColor::Yellow,
            false,
            0.0f,
            0,
            2.0f
        );
    }
}
