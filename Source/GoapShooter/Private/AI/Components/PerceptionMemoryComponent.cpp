#include "Components/PerceptionMemoryComponent.h"
#include "GoapShooterAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
#include "Characters/GoapShooterCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "GoapShooterGameState.h"
#include "Curves/CurveFloat.h"
#include "Perception/AISenseConfig_Hearing.h"

UPerceptionMemoryComponent::UPerceptionMemoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPerceptionMemoryComponent::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AGoapShooterAIController>(GetOwner());

	AIController->GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this,
		&UPerceptionMemoryComponent::TargetActorsPerceptionUpdated);

	GetWorld()->GetGameState<AGoapShooterGameState>()->OnPlayerKillDelegate.AddDynamic(this,
		&UPerceptionMemoryComponent::OnPlayerOrBotKilled);
	
	bIsInitialized = true;
}

void UPerceptionMemoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	// clean up bindings
	if (GetWorld())
	{
		AGoapShooterGameState* GameState = GetWorld()->GetGameState<AGoapShooterGameState>();
		if (IsValid(GameState))
		{
			GameState->OnPlayerKillDelegate.RemoveDynamic(this, &UPerceptionMemoryComponent::OnPlayerOrBotKilled);
		}
	}
}

void UPerceptionMemoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(AIController) || !IsValid(AIController->GetPawn())) return;
	
	UpdateMostInterestingActor();
	
	DrawPerceptionVisualization();

	DrawStrongHearingRange();
}

AGoapShooterCharacter* UPerceptionMemoryComponent::GetMostInterestingPerceivedEnemy()
{
	// if (bDebugMode)
	// {
	// 	// search for all actors of class
	// 	TArray<AActor*> AllActors;
	// 	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoapShooterCharacter::StaticClass(), AllActors);
 //        
	// 	for (AActor* Actor : AllActors)
	// 	{
	// 		AGoapShooterCharacter* Char = Cast<AGoapShooterCharacter>(Actor);
	// 		if (Char && GetPawn() != Char)
	// 		{
	// 			return Char;
	// 		}
	// 	}
	// 	UE_LOG(LogTemp, Error, TEXT("AGoapShooterAIController: In debug mode no enemy found"));
	// }
	
	AGoapShooterCharacter* MostInterestingActorTemp = nullptr;
	if (MostInterestingActor) 
	{
		MostInterestingActorTemp = Cast<AGoapShooterCharacter>(MostInterestingActor);
	}
	return MostInterestingActorTemp;
}

FPerceptionData UPerceptionMemoryComponent::GetMostInterestingPerception()
{
	if (!MostInterestingActor)
	{
		return FPerceptionData();
	}
	return PerceptionMemory[MostInterestingActor];
}

bool UPerceptionMemoryComponent::CanSeeEnemy()
{
	return HasStrongEnemyPerception() && HasLineOfSightToActor(MostInterestingActor); // TODO lineofsight info can be cached in perceptionmemory
}

FVector UPerceptionMemoryComponent::GetEstimatedEnemyLocation()
{
	return MostInterestingActor ? MostInterestingActor->GetActorLocation() : LastSeenEnemyLocation; // TODO this should be retrieved from perceptionmemory
}

void UPerceptionMemoryComponent::OnPlayerOrBotKilled(APlayerState* Killer, APlayerState* Victim)
{
	if (!IsValid(AIController) || !IsValid(AIController->GetPawn()))
	{
		return;
	}
	
	if (!Victim)
	{
		return;
	}
	AGoapShooterCharacter* MostInterestingChar = Cast<AGoapShooterCharacter>(MostInterestingActor);
	if (MostInterestingChar && MostInterestingChar->GetPlayerState() && MostInterestingChar->GetPlayerState() == Victim)
	{
		bLastEnemyInSightKilled = true;
	}
}

APawn* UPerceptionMemoryComponent::GetPawn()
{
	if (!AIController)
	{
		return nullptr;
	}
	return AIController->GetPawn();
}

void UPerceptionMemoryComponent::TargetActorsPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	if (!IsValid(AIController) || !IsValid(AIController->GetPawn())) return;
	
    //UE_LOG(LogTemp, Error, TEXT("TargetActorsPerceptionUpdated called"));

    if (TargetActor == GetPawn() || !TargetActor->ActorHasTag("ToBePerceivedByAI"))
    {
        return;
    }

    //UE_LOG(LogTemp, Error, TEXT("TargetActorsPerceptionUpdated: %s"), *TargetActor->GetName());

    if (Stimulus.WasSuccessfullySensed()) {

        //UE_LOG(LogTemp, Error, TEXT("TargetActorsPerceptionUpdated: Sensed %s"), *TargetActor->GetName());
        AddOrUpdatePerceptionData(TargetActor, Stimulus);
        
    } else {

        //UE_LOG(LogTemp, Error, TEXT("TargetActorsPerceptionUpdated: Not sensed %s"), *TargetActor->GetName());
        StartPerceptionDataExpiration(TargetActor, Stimulus);
    }
}

void UPerceptionMemoryComponent::UpdateMostInterestingActor()
{
	CleanUpPerceptionMemory(); // cleaning up from time to time (helps ChooseMostInterestingPerceptionData(), but also mitigates risk of memory leaks)
	
    bool bThisTickStrongPerceptionOfMostInterestingActor = MostInterestingActor 
        && PerceptionMemory.Contains(MostInterestingActor) // check because could be deleted by expiration
        && HasStrongEnemyPerception() && HasLineOfSightToActor(MostInterestingActor);

    if (bLastTickStrongPerceptionOfMostInterestingActor == true && bThisTickStrongPerceptionOfMostInterestingActor == false) 
    {
        // lost sight of the most interesting enemy
        LastSeenEnemyLocation = MostInterestingActor->GetActorLocation();
        LastSeenEnemyWorldTime = GetWorld()->GetTimeSeconds();
        AIPositionWhenLastSawEnemy = GetPawn()->GetActorLocation();

        //OnSignificantWorldStateChange();
    }
    else if (bLastTickStrongPerceptionOfMostInterestingActor == false && bThisTickStrongPerceptionOfMostInterestingActor == true)
    {
        // gained sight of the most interesting enemy
        LastSeenEnemyLocation = FVector::ZeroVector;
        LastSeenEnemyWorldTime = -FLT_MAX;
        AIPositionWhenLastSawEnemy = FVector::ZeroVector;
        bLastEnemyInSightKilled = false;

        //OnSignificantWorldStateChange();
    }

    AActor* LastTickMostInterestingActor = MostInterestingActor;

    ChooseMostInterestingPerceptionData();

    if (MostInterestingActor != LastTickMostInterestingActor)
    {
        // the most interesting actor has changed
        LastSeenEnemyLocation = FVector::ZeroVector;
        LastSeenEnemyWorldTime = -FLT_MAX;
        AIPositionWhenLastSawEnemy = FVector::ZeroVector;
        bLastEnemyInSightKilled = false;

        UE_LOG(LogTemp, Error, TEXT("MostInterestingActor has changed => OnSignificantWorldStateChange !!!"));
        AIController->OnSignificantWorldStateChange();
    }

    bLastTickStrongPerceptionOfMostInterestingActor = MostInterestingActor 
        && PerceptionMemory.Contains(MostInterestingActor) // check because could be deleted by expiration
        && HasStrongEnemyPerception() && HasLineOfSightToActor(MostInterestingActor);
}

void UPerceptionMemoryComponent::ChooseMostInterestingPerceptionData()
{
    // if nothing in memory, there is nothing to choose
    if (PerceptionMemory.Num() == 0)
    {
        //MostInterestingPerception = FPerceptionData();
        MostInterestingActor = nullptr;
        return;
    }

    // if current most interesting is not in our memory anymore (deleted by CleanUpPerceptionMemory()), we can already reset it before continuing
    if (!PerceptionMemory.Contains(MostInterestingActor))
    {
        //MostInterestingPerception = FPerceptionData();
        MostInterestingActor = nullptr;
    }

    // find new most interesting
    EPerceptionType HighestPriorityType = EPerceptionType::None;
    float HighestStrength = -1.0f;
    //float MostRecentTime = -1.0f;
    AActor* MostInterestingActorTemp = nullptr;
    
    for (auto& Pair : PerceptionMemory)
    {
        AActor* Actor = Pair.Key;
        FPerceptionData& Perception = Pair.Value;
        
        //if (Perception.bHasBeenInvestigated)
        //{
        //    continue;
        //}
        
        bool bIsMoreInteresting = false;
        
        if (Perception.GetStrength() > HighestStrength) 
        {
            if (Perception.PerceptionType == EPerceptionType::Audio
                && (HighestPriorityType == EPerceptionType::Visual || HighestPriorityType == EPerceptionType::Damage))
            {
                // dont do anything in this case (visual/damage always more important than audio)
            }
            else
            {
                bIsMoreInteresting = true;
            }
        }
        
        if (bIsMoreInteresting)
        {
            HighestPriorityType = Perception.PerceptionType;
            HighestStrength = Perception.GetStrength();
            //MostRecentTime = Perception.LastPerceivedTime;
            MostInterestingActorTemp = Actor;
        }
    }
    
    // dont change most interesting, if a current one exists and has strength of at least 1 and the new one is not super super strong (e.g. due to damage)
    if (MostInterestingActor && PerceptionMemory.Contains(MostInterestingActor)
        && PerceptionMemory[MostInterestingActor].GetStrength() >= 1.0f
        //&& PerceptionMemory[MostInterestingActor].GetStrength() < HighestStrength
        && MostInterestingActorTemp != MostInterestingActor
        && HighestStrength - PerceptionMemory[MostInterestingActor].GetStrength() < 0.33f)
    {
        return;
    }

    // set the most interesting perception if it has changed
    if (MostInterestingActorTemp && MostInterestingActorTemp != MostInterestingActor)
    {

        // dont change most interesting, if the current action is related to the current most interesting and the new perception is not really strong
        if (AIController->IsAnyCurrentExecutableActionRelatedToMostInterestingActor() && HighestStrength < 0.75f)
        {
            return;
        }

        // update most interesting
        //MostInterestingPerception = PerceptionMemory[MostInterestingActorTemp];
        MostInterestingActor = MostInterestingActorTemp;
    }
}

bool UPerceptionMemoryComponent::HasLineOfSightToActor(AActor* TargetActor)
{
    if (!GetPawn() || !TargetActor)
    {
        return false;
    }

    // Get start and end locations for the line trace
    FVector StartLocation = GetPawn()->GetActorLocation();
    // Add eye height offset for more realistic line of sight
    StartLocation.Z += Cast<APawn>(GetPawn())->BaseEyeHeight;
    
    // Get the target location - aim for the center of the actor
    FVector TargetLocation = TargetActor->GetActorLocation();
    
    // Set up trace parameters
    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(GetPawn()); // Ignore the AI pawn
    CollisionParams.AddIgnoredActor(TargetActor); // Ignore the target actor
    CollisionParams.bTraceComplex = true; // Use complex collision for more accurate results
    
    // Perform the line trace
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        TargetLocation,
        ECC_Visibility, // Use visibility channel for line of sight
        CollisionParams
    );
    
    // If we didn't hit anything or we hit the target actor, we have line of sight
    bool bHasLineOfSight = !bHit || HitResult.GetActor() == TargetActor;
    
    // Optional: Visualize the line trace in debug mode
    // if (bDebugMode)
    // {
    //     FColor LineColor = bHasLineOfSight ? FColor::Green : FColor::Red;
    //     DrawDebugLine(GetWorld(), StartLocation, bHit ? HitResult.ImpactPoint : TargetLocation, 
    //         LineColor, false, 0.1f, 0, 1.0f);
        
    //     if (bHit && HitResult.GetActor() != TargetActor)
    //     {
    //         // Draw a sphere at the impact point
    //         DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.0f, 8, LineColor, false, 0.1f);
    //     }
    // }
    
    return bHasLineOfSight;
}

bool UPerceptionMemoryComponent::IsControlledPawnInFieldOfViewOfActor(AActor* Actor)
{
    if (!GetPawn() || !Actor)
    {
        return false;
    }
    
    FVector PawnLocation = Actor->GetActorLocation();
    FVector TargetLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - PawnLocation).GetSafeNormal();
    
    FVector PawnForward = Actor->GetActorForwardVector();
    
    float DotProduct = FVector::DotProduct(PawnForward, DirectionToTarget);
    float AngleRadians = FMath::Acos(DotProduct);
    float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);
    
    return AngleDegrees <= (AIController->FieldOfViewAngle * 0.5f);
}

bool UPerceptionMemoryComponent::IsActorInFieldOfView(AActor* TargetActor)
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
    
    return AngleDegrees <= (AIController->FieldOfViewAngle * 0.5f);
}

void UPerceptionMemoryComponent::OnStimulusInvestigated(const FPerceptionData& Stimulus)
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

FInterpCurveFloat UPerceptionMemoryComponent::GetCurveForHearingAttenuation(FName NoiseTag)
{
    FInterpCurveFloat Curve;
    if (NoiseTag == FName("Shot"))
    {
        Curve.AddPoint(0.0f, 1.0f);
        Curve.AddPoint(AIController->GetHearingConfig()->HearingRange, 1.0f);
    }
    else if (NoiseTag == FName("Footstep"))
    {
        Curve.AddPoint(0.0f, 1.0f);
        Curve.AddPoint(StrongHearingRange, 1.0f);
        Curve.AddPoint(StrongHearingRange * 2, 0.0f);
        Curve.AddPoint(AIController->GetHearingConfig()->HearingRange, 0.0f);
    }
    else
    {
        Curve.AddPoint(0.0f, 1.0f);
        Curve.AddPoint(StrongHearingRange, 1.0f);
        Curve.AddPoint(StrongHearingRange * 2, 0.0f);
        Curve.AddPoint(AIController->GetHearingConfig()->HearingRange, 0.0f);
    }
    return Curve;
}

void UPerceptionMemoryComponent::AddOrUpdatePerceptionData(AActor* PerceivedActor, FAIStimulus Stimulus)
{
    if (DeterminePerceptionType(Stimulus) == EPerceptionType::Damage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Damage stimulus received !!!"));
    }

    // dont override existing visual perception by audio perception
    if (PerceptionMemory.Contains(PerceivedActor) 
        && PerceptionMemory[PerceivedActor].PerceptionType == EPerceptionType::Visual 
        && DeterminePerceptionType(Stimulus) == EPerceptionType::Audio)
    {
        // ...could be improved here: a weak visual perception could be strengthened by a strong audio perception...
        return;
    }
    // discard too weak audio stimuli
    if (DeterminePerceptionType(Stimulus) == EPerceptionType::Audio && Stimulus.Strength < 0.33f)
    {
        return;
    }

    // in case of audio we need to adjust the strength according to distance
    float Strength = Stimulus.Strength;
    if (DeterminePerceptionType(Stimulus) == EPerceptionType::Audio)
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PerceivedActor->GetActorLocation());
        FInterpCurveFloat Curve = GetCurveForHearingAttenuation(Stimulus.Tag);
        Strength = Strength * Curve.Eval(Distance, 1.0f);

        // if the current strength is lower than the existing strength, dont weaken the existing strength!!
        if (PerceptionMemory.Contains(PerceivedActor) && PerceptionMemory[PerceivedActor].GetStrength() > Strength)
        {
            Strength = PerceptionMemory[PerceivedActor].GetStrength();
        }
    }
    // in case of damage we will always take 1.5 strength
    else if (DeterminePerceptionType(Stimulus) == EPerceptionType::Damage)
    {
        Strength = 1.5f;
    }
    else if (DeterminePerceptionType(Stimulus) == EPerceptionType::Visual)
    {
        if (PerceptionMemory.Contains(PerceivedActor))
        {
            Strength = FMath::Max(PerceptionMemory[PerceivedActor].GetStrength(), Stimulus.Strength);
        }
    }

    //bool bIsSignificantWorldStateChange = false;
    //if (MostInterestingActor == PerceivedActor && PerceptionMemory.Contains(PerceivedActor) 
    //       && Strength + PerceptionMemory[PerceivedActor].GetStrength() >= 1.0f) // just a heuristic indicator
    //{
    //    bIsSignificantWorldStateChange = true;
    //}

    // in ADD or UPDATE case:
    FPerceptionData& PerceptionData = PerceptionMemory.FindOrAdd(PerceivedActor);
    PerceptionData.PerceivedActor = PerceivedActor;
    PerceptionData.LastPerceivedLocation = PerceivedActor->GetActorLocation();
    PerceptionData.LastPerceivedTime = FLT_MAX; // FLT_MAX indicates that the actor is actively perceived
    PerceptionData.PerceptionType = DeterminePerceptionType(Stimulus);
    PerceptionData.Strength = Strength;
    PerceptionData.SecondsUntilRemovalOfPerceptionData = SecondsUntilRemovalOfPerceptionData; // helps for the GetStrength() calculation
    // only in ADD case:
    if (!PerceptionMemory.Contains(PerceivedActor))
    {
        PerceptionData.bHasBeenInvestigated = false;
    }

    //if (bIsSignificantWorldStateChange) {
    //   OnSignificantWorldStateChange();
    //}
}

void UPerceptionMemoryComponent::StartPerceptionDataExpiration(AActor* UnPerceivedActor, FAIStimulus Stimulus)
{
    if (!PerceptionMemory.Contains(UnPerceivedActor))
    {
        return;
    }
    EPerceptionType PerceptionType = DeterminePerceptionType(Stimulus);
    FPerceptionData& StoredPerception = PerceptionMemory[UnPerceivedActor];
    if (StoredPerception.PerceptionType == PerceptionType)
    {
        //PerceptionMemory.Remove(UnPerceivedActor);
        StoredPerception.LastPerceivedTime = GetWorld()->GetTimeSeconds();
    }
}

void UPerceptionMemoryComponent::CleanUpPerceptionMemory()
{
	// remove entries with invalid actors
	for (auto It = PerceptionMemory.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Key))
		{
			It.RemoveCurrent();
		}
	}
	// remove entries with expired stimuli (strength)
	for (auto It = PerceptionMemory.CreateIterator(); It; ++It)
	{
		if (It->Value.GetStrength() <= 0.0f)
		{
			It.RemoveCurrent();
		}
	}
}

EPerceptionType UPerceptionMemoryComponent::DeterminePerceptionType(const FAIStimulus& Stimulus)
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

bool UPerceptionMemoryComponent::IsAimedAt()
{
	// Check if any of the perceived enemies are looking at us
	for (const TPair<AActor*, FPerceptionData>& Pair : PerceptionMemory)
	{
		AActor* Enemy = Pair.Key;
		if (!Enemy || Pair.Value.PerceptionType == EPerceptionType::Audio || Pair.Value.PerceptionType == EPerceptionType::None)
		{
			continue;
		}

		// Get the forward vector of the enemy (the direction they're facing)
		FVector EnemyForward = Enemy->GetActorForwardVector();
        
		// Get the direction from the enemy to our pawn
		FVector DirectionToUs = (GetPawn()->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
        
		// Calculate the dot product - values closer to 1 mean the enemy is looking more directly at us
		float DotProduct = FVector::DotProduct(EnemyForward, DirectionToUs);
        
		// If the dot product is above our threshold (e.g., 0.7 means roughly within a 45-degree cone),
		// then the enemy is considered to be looking at us
		const float LookingAtThreshold = 0.7f; // Adjust this value as needed
		if (DotProduct > LookingAtThreshold)
		{
			return true;
		}
	}
	return false;
}

bool UPerceptionMemoryComponent::IsLookingAtStimulus()
{
	return IsActorInFieldOfView(MostInterestingActor);
}

bool UPerceptionMemoryComponent::IsMinThresholdFulfilledForEnemyPerception()
{
	return MostInterestingActor 
		&& PerceptionMemory[MostInterestingActor].GetStrength() >= 0.25f;
}

bool UPerceptionMemoryComponent::HasHalfStrongEnemyPerception()
{
	return MostInterestingActor 
		&& PerceptionMemory[MostInterestingActor].GetStrength() >= 0.5f
		&& PerceptionMemory[MostInterestingActor].PerceptionType == EPerceptionType::Visual;
}

bool UPerceptionMemoryComponent::HasStrongEnemyPerception()
{
	return MostInterestingActor 
		&& PerceptionMemory[MostInterestingActor].GetStrength() >= 1.0f
		&& PerceptionMemory[MostInterestingActor].PerceptionType == EPerceptionType::Visual;
}

bool UPerceptionMemoryComponent::AreAllEnemiesInSightKilled()
{
	return bLastEnemyInSightKilled && PerceptionMemory.Num() == 0;
}

bool UPerceptionMemoryComponent::HasPerceivedStimulus()
{
	return PerceptionMemory[MostInterestingActor].PerceptionType != EPerceptionType::None;
}

bool UPerceptionMemoryComponent::HasInvestigatedStimulus()
{
	return PerceptionMemory[MostInterestingActor].bHasBeenInvestigated;
}

FString UPerceptionMemoryComponent::GetPerceivedStimulusType()
{
	return FPerceptionTypeUtils::ToString(PerceptionMemory[MostInterestingActor].PerceptionType);
}

FVector UPerceptionMemoryComponent::GetPerceivedStimulusLocation()
{
	return PerceptionMemory[MostInterestingActor].LastPerceivedLocation;
}

void UPerceptionMemoryComponent::DrawStrongHearingRange()
{
	if (!bDebugDrawPerception)
	{
		return;
	}
	DrawDebugCircle(
		GetWorld(),
		GetPawn()->GetActorLocation(),
		StrongHearingRange,
		32,
		FColor::Yellow,
		false,
		-1.0f,
		0,
		1.0f,
		FVector(1, 0, 0), // YAxis
		FVector(0, 1, 0)  // ZAxis -> up direction
	);
}

void UPerceptionMemoryComponent::DrawPerceptionVisualization()
{
	if (!bDebugDrawPerception || !AIController->GetPawn() || !AIController->GetSightConfig())
	{
		return;
	}

	// Get the pawn's location and forward vector
	FVector PawnLocation = AIController->GetPawn()->GetActorLocation();
	FVector PawnForward = AIController->GetPawn()->GetActorForwardVector();
    
	const float SightRadius = AIController->GetSightConfig()->SightRadius;
	const float PeripheralVisionAngle = AIController->GetSightConfig()->PeripheralVisionAngleDegrees;
    
	// Determine the color based on whether an enemy is detected
	// IMPORTANT: Using our own check for visualization purposes ONLY
	bool bIsActorPerceivedForVisualization = HasStrongEnemyPerception(); //PerceptionMemory.Num() > 0;
    
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
	if (bIsActorPerceivedForVisualization)
	{
		DrawDebugLine(
			GetWorld(),
			PawnLocation,
			MostInterestingActor->GetActorLocation(),
			FColor::Yellow,
			false,
			0.0f,
			0,
			2.0f
		);
	}
}
