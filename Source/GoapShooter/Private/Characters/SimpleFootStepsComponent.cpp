#include "Characters/SimpleFootStepsComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AISenseConfig_Hearing.h"

USimpleFootStepsComponent::USimpleFootStepsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	DistanceThreshold = 150.0f; // Set your desired distance threshold
	DistanceAfterToPlayStartSound = 20.0f; // Set your desired distance threshold
	DefaultFootstepSound = nullptr; // Set your footstep sound asset
	DistanceTraveledSinceLastFootstep = 0.0f;
	PreviousSpeed = 0.0f;
	bPreviousIsGrounded = true;
}

void USimpleFootStepsComponent::ReportFootstepNoise(float Volume)
{
	//UE_LOG(LogTemp, Error, TEXT("ReportFootstepNoise called with volume: %f, character: %s"), Volume, *GetOwner()->GetName());
    UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),
        GetOwner()->GetActorLocation(), // Noise location 
        Volume, // Loudness
        GetOwner(), // Noise Instigator (this character)
        0.0f, // Max range (0 means default hearing range)
        FName("Footstep") // Tag for the noise
    );
}

void USimpleFootStepsComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerChar = Cast<ACharacter>(GetOwner());
	
	PreviousPosition = PlayerChar->GetActorLocation();
}

void USimpleFootStepsComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!DefaultFootstepSound)
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultFootstepSound is not set in USimpleFootStepsComponent."));
		return;
	}
	
	DistanceTraveledSinceLastFootstep += FVector::Dist(PreviousPosition, GetOwner()->GetActorLocation());
	
	UCharacterMovementComponent* MovementComponent = PlayerChar->GetCharacterMovement();
	float CurrentSpeed = MovementComponent->Velocity.Size();
	bool bIsSprinting = CurrentSpeed >= MaxRunSpeed;
	bool bIsCrouching = MovementComponent->IsCrouching();
	FVector CurrentPosition = GetOwner()->GetActorLocation();
	bool bCurrentIsGrounded = MovementComponent->IsFalling() == false;

	// check if we just started to move and if yes, play according sound
	if (!bWasStartSoundPlayed && DistanceTraveledSinceLastFootstep >= DistanceAfterToPlayStartSound && CurrentSpeed > 0.0f)
	{
		PlayFootStepSound(CurrentPosition, bIsCrouching, bIsSprinting, true, false, false, false);
		bWasStartSoundPlayed = true;
	}
	// check if we just ended to move and if yes, play according sound
	else if (PreviousSpeed > 0.0f && CurrentSpeed == 0.0f)
	{
		PlayFootStepSound(CurrentPosition, bIsCrouching, bIsSprinting, false, true, false, false);
		DistanceTraveledSinceLastFootstep = 0.0f;
		bWasStartSoundPlayed = false;
	}
	// check if we just landed and if yes, play footstep sound
	else if (bPreviousIsGrounded == false && MovementComponent->IsFalling() == false)
	{
		PlayFootStepSound(CurrentPosition, bIsCrouching, bIsSprinting, false, false, false, true);
		DistanceTraveledSinceLastFootstep = 0.0f;
	}
	// check if we just started to jump and if yes, play according sound
	else if (bPreviousIsGrounded == true && MovementComponent->IsFalling() == true)
	{
		PlayFootStepSound(CurrentPosition, bIsCrouching, bIsSprinting, false, false, true, false);
		DistanceTraveledSinceLastFootstep = 0.0f;
	}
	// check that we moved enough distance to play footstep sound again
	else if (DistanceTraveledSinceLastFootstep >= DistanceThreshold && CurrentSpeed > 0.0f && bCurrentIsGrounded)
	{
		PlayFootStepSound(CurrentPosition, bIsCrouching, bIsSprinting, false, false, false, false);
		DistanceTraveledSinceLastFootstep = 0.0f;
	}

	PreviousPosition = CurrentPosition;
	PreviousSpeed = CurrentSpeed;
	bPreviousIsGrounded = bCurrentIsGrounded;
}

void USimpleFootStepsComponent::PlayFootStepSound(FVector CurrentPosition, bool bIsCrouching, bool bIsSprinting,
													bool bIsStartingToMove, bool bIsEndingToMove,
													bool bIsStartingToJump, bool bIsLanding)
{
	USoundBase* FootStepSound = SelectFootStepSound();
	if (FootStepSound == nullptr)
	{
		FootStepSound = DefaultFootstepSound;
	}
	if (FootStepSound != nullptr)
	{
		float VolumeMultiplier = 1.0f;
		if (bIsCrouching)
		{
			VolumeMultiplier = CrouchVolumeMultiplier;
		}
		else if (bIsSprinting)
		{
			VolumeMultiplier = RunVolumeMultiplier;
		}
		else
		{
			VolumeMultiplier = WalkVolumeMultiplier;
		}
		UAudioComponent* AudioComponent;
		if (PlayerChar->IsLocallyControlled()) {
			VolumeMultiplier = VolumeMultiplier - 0.2;
			AudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), FootStepSound, VolumeMultiplier);
		}
		else {
			AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
				GetWorld(), FootStepSound, CurrentPosition, FRotator::ZeroRotator, VolumeMultiplier);
		}
		ReportFootstepNoise(VolumeMultiplier);
	}
}

USoundBase* USimpleFootStepsComponent::SelectFootStepSound_Implementation()
{
	return DefaultFootstepSound;
}
