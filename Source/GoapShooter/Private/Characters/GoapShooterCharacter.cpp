#include "Characters/GoapShooterCharacter.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GoapShooterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/DamageType.h"
#include "Perception/AIPerceptionSystem.h"
#include "Characters/SimpleFootStepsComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"

AGoapShooterCharacter::AGoapShooterCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	// set up stimulus source component => AI can perceive this character
    Tags.Add(FName("ToBePerceivedByAI"));
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimulusSourceComponent"));
	StimuliSourceComponent->RegisterForSense(TSubclassOf<UAISense_Sight>());
    StimuliSourceComponent->RegisterForSense(TSubclassOf<UAISense_Hearing>());
    StimuliSourceComponent->RegisterForSense(TSubclassOf<UAISense_Damage>());
	StimuliSourceComponent->RegisterWithPerceptionSystem();

    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

    // Create a mesh component for first person view
    Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
    Mesh1P->SetupAttachment(GetMesh());
    Mesh1P->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    Mesh1P->SetRelativeLocation(FVector(0.0f, 0.0f, 160.0f));
    Mesh1P->bCastDynamicShadow = false;
    Mesh1P->CastShadow = false;

    // Default character movement
    GetCharacterMovement()->MaxWalkSpeed = 350.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    // Footsteps component
    FootStepsComponent = CreateDefaultSubobject<USimpleFootStepsComponent>(TEXT("FootStepsComponent"));
}

void AGoapShooterCharacter::BeginPlay()
{
    // Call the base class  
    Super::BeginPlay();
    
    // Initialize health to max health at the start
    Health = MaxHealth;

    // Initialize FootStepsComponent, for some reason not working in constructor
    FootStepsComponent->DefaultFootstepSound = FootstepSound;
}

void AGoapShooterCharacter::PlayShootSound()
{
    if (ShootSound)
    {
        UGameplayStatics::SpawnSoundAtLocation(this, ShootSound, GetActorLocation());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ShootSound is null"));
    }
    ReportShootNoise(); 
}

void AGoapShooterCharacter::ReportDamage(AActor* DamageInstigator)
{
    UAISense_Damage::ReportDamageEvent(
        GetWorld(),
        this, // Damaged actor
        DamageInstigator, // Damage Instigator
        1.0f, // Damage amount
        GetActorLocation(), // Damage location
        GetActorLocation(), // Hit location
        FName("Shot") // Tag for the damage
    );
}

void AGoapShooterCharacter::ReportShootNoise()
{
    UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),
        GetActorLocation(), // Noise location 
        1.0f, // Loudness
        this, // Noise Instigator (this character)
        0.0f, // Max range (0 means default hearing range)
        FName("Shot") // Tag for the noise
    );
}

bool AGoapShooterCharacter::ShootAt(AGoapShooterCharacter* Target, float MaxSway)
{
    // Check if we have a rifle and valid target
    if (!Target || IsDead())
    {
        return false;
    }
    
    // Get eye location (where the shot will originate from)
    FVector EyeLocation;
    FRotator EyeRotation;
    GetActorEyesViewPoint(EyeLocation, EyeRotation);
    
    // Get direction to target (perfect aim)
    FVector TargetLocation = Target->GetActorLocation();
    
    // Aim at character center mass instead of actor origin
    UCapsuleComponent* TargetCapsule = Cast<UCapsuleComponent>(Target->GetComponentByClass(UCapsuleComponent::StaticClass()));
    if (TargetCapsule)
    {
        // Adjust target location to aim at middle of capsule
        TargetLocation.Z += TargetCapsule->GetScaledCapsuleHalfHeight() * 0.7f;
    }
    
    FVector DirectionToTarget = (TargetLocation - EyeLocation).GetSafeNormal();
    
    // Check if target is moving and adjust sway accordingly
    float MovementMultiplier = 1.0f;
    FVector TargetVelocity = Target->GetVelocity();
    if (!TargetVelocity.IsNearlyZero())
    {
        // Calculate target speed
        float TargetSpeed = TargetVelocity.Size();
        
        // Get character movement component to determine max speed
        UCharacterMovementComponent* MovementComp = Target->GetCharacterMovement();
        float MaxSpeed = MovementComp ? MovementComp->MaxWalkSpeed : 600.0f;
        
        // Normalize speed (0.0 to 1.0 range)
        float NormalizedSpeed = FMath::Clamp(TargetSpeed / MaxSpeed, 0.0f, 1.0f);
        
        // Increase sway based on movement speed (up to 2x for full speed)
        MovementMultiplier = 1.0f + NormalizedSpeed;
        
        // Additional sway for lateral movement (strafing is harder to hit)
        FVector LateralVelocity = TargetVelocity - (DirectionToTarget * FVector::DotProduct(TargetVelocity, DirectionToTarget));
        float LateralSpeed = LateralVelocity.Size();
        float NormalizedLateralSpeed = FMath::Clamp(LateralSpeed / MaxSpeed, 0.0f, 1.0f);
        
        // Add up to 0.5x more sway for full lateral movement
        MovementMultiplier += NormalizedLateralSpeed * 0.5f;
    }
    
    // Apply random sway to simulate imperfect aim, adjusted by movement
    float AdjustedMaxSway = MaxSway * MovementMultiplier;
    float HorizontalSway = FMath::RandRange(-AdjustedMaxSway, AdjustedMaxSway) * 0.5f;
    float VerticalSway = FMath::RandRange(-AdjustedMaxSway, AdjustedMaxSway) * 0.5f;
    
    // Create rotation from direction and apply sway
    FRotator DirectionRotation = DirectionToTarget.Rotation();
    DirectionRotation.Yaw += HorizontalSway;
    DirectionRotation.Pitch += VerticalSway;
    
    // Calculate new direction with sway
    FVector ShootDirection = DirectionRotation.Vector();
    
    // Set up trace parameters
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.bTraceComplex = true;
    
    // Perform line trace to see if we hit something
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        EyeLocation,
        EyeLocation + ShootDirection * MaxShootDistance,
        ECC_Pawn, // Use Pawn collision channel instead of Visibility
        QueryParams
    );
    
    // Determine hit color for debug visualization
    FColor TraceColor;
    bool bHitTarget = false;
    
    bool bIsDeadlyShot = false;
    
    if (bHit)
    {
        if (HitResult.GetActor() == Target)
        {
            // We hit the intended target - green trace
            TraceColor = FColor::Green;
            bHitTarget = true;
            
            // Apply damage using UGameplayStatics::ApplyPointDamage instead of direct TakeDamage
            UGameplayStatics::ApplyPointDamage(
                Target,                          // Target actor
                RifleDamage,                     // Damage amount
                ShootDirection,                  // Hit direction
                HitResult,                       // Hit info
                GetController(),                 // Instigator controller
                this,                            // Damage causer
                UDamageType::StaticClass()       // Damage type
            );

            // Check if the target is dead
            bIsDeadlyShot = Target->IsDead();
        }
        else
        {
            // We hit something else - yellow trace
            TraceColor = FColor::Yellow;

            // this is not really right, because we want to also apply damage to characters (collaterial damage)
        }
    }
    else
    {
        // We didn't hit anything - red trace
        TraceColor = FColor::Red;
    }
    
    PlayShootSound();

    // Draw debug line for visualization - replaced with blinking shot effect
    const int32 NumBlinks = 3; // Number of blinks for the shot effect
    const float BlinkDuration = 0.05f; // Duration of each blink in seconds
    const float BlinkSpacing = 0.03f; // Time between blinks
    
    FVector StartPoint = EyeLocation;
    FVector EndPoint = bHit ? HitResult.ImpactPoint : EyeLocation + ShootDirection * MaxShootDistance;
    
    // Create a sequence of delayed line draws to create a blinking effect
    for (int32 i = 0; i < NumBlinks; i++)
    {
        // Calculate delay for this blink
        float Delay = i * (BlinkDuration + BlinkSpacing);
        
        // Schedule the line to appear
        FTimerHandle LineAppearHandle;
        FTimerDelegate LineAppearDelegate;
        LineAppearDelegate.BindLambda([this, StartPoint, EndPoint, TraceColor]() {
            DrawDebugLine(
                GetWorld(),
                StartPoint,
                EndPoint,
                TraceColor,
                false,
                0.0f, // Will be cleared by the disappear timer
                0,
                3.0f // Thicker line for better visibility
            );
        });
        GetWorldTimerManager().SetTimer(LineAppearHandle, LineAppearDelegate, Delay, false);
        
        // Schedule the line to disappear
        FTimerHandle LineDisappearHandle;
        FTimerDelegate LineDisappearDelegate;
        LineDisappearDelegate.BindLambda([this]() {
            FlushPersistentDebugLines(GetWorld());
        });
        GetWorldTimerManager().SetTimer(LineDisappearHandle, LineDisappearDelegate, Delay + BlinkDuration, false);
    }
    
    // If we hit, draw a debug point at impact location
    if (bHit)
    {
        DrawDebugSphere(
            GetWorld(),
            HitResult.ImpactPoint,
            10.0f,
            8,
            TraceColor,
            false,
            DebugDrawTime,
            0,
            1.0f
        );
    }
    
    return bHitTarget && bIsDeadlyShot;
}

float AGoapShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Call parent implementation first
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    // Apply damage to health
    Health = FMath::Max(Health - ActualDamage, 0.0f);
    
    //UE_LOG(LogTemp, Log, TEXT("%s took %.1f damage from %s. Health: %.1f"), 
    //    *GetName(), 
    //    ActualDamage, 
    //    DamageCauser ? *DamageCauser->GetName() : TEXT("Unknown"),
    //    Health);

    ReportDamage(DamageCauser);

    // Check if character is dead
    if (IsDead())
    {
        Die(DamageCauser);
    }
    
    return ActualDamage;
}

void AGoapShooterCharacter::Die(AActor* DamageCauser)
{
    UE_LOG(LogTemp, Log, TEXT("%s has died"), *GetName());

    // Tell GameState about death/kill
    AGoapShooterCharacter* KillerChar = Cast<AGoapShooterCharacter>(DamageCauser);
    APlayerState* KillerPlayerState = nullptr;
    if (KillerChar)
    {
        KillerPlayerState = KillerChar->GetPlayerState();
    }
    APlayerState* VictimPlayerState = GetPlayerState();
    if (VictimPlayerState)
    {
        GetWorld()->GetGameState<AGoapShooterGameState>()->
            OnPlayerKillDelegate.Broadcast(KillerPlayerState, VictimPlayerState);
    }
    
    // Disable collision to prevent further interaction
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Make the character invisible to AI perception
    SetActorHiddenInGame(true);
    
    // Remove the tag that makes this actor perceivable by AI
    //Tags.Remove(FName("ToBePerceivedByAI"));
    UAIPerceptionSystem::GetCurrent(GetWorld())->UnregisterSource(*this);

    // Disable tick to save performance
    SetActorTickEnabled(false);
    
    // Schedule the actor to be destroyed after a delay
    // This allows any effects to complete before removal
    
    //Destroy();
    SetLifeSpan(1.0f);
}
