#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimpleFootStepsComponent.generated.h"

/*
 * ...Copied/adjusted from main project...
 * 
 * This is an "abstract" class which should be extended by Blueprint
 * (because physical materials and surfaces are supposed to be configured in engine editor).
 *
 * The audio Cue is then selected in Blueprint based on the surface type of the ground.
 * The audio Cue can react on bool parameters which will be set:
 * 
 * - "IsCrouching"
 * - "IsSprinting"
 * - "IsStartingToMove"
 * - "IsEndingToMove"
 * - "IsStartingToJump"
 * - "IsLanding"
 */
UCLASS(ClassGroup=(Custom), meta = (BlueprintSpawnableComponent))
class GOAPSHOOTER_API USimpleFootStepsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USimpleFootStepsComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRunSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CrouchVolumeMultiplier = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WalkVolumeMultiplier = 0.75f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RunVolumeMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceThreshold;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* DefaultFootstepSound;
	UPROPERTY()
	float DistanceAfterToPlayStartSound;
	UPROPERTY()
	float DistanceTraveledSinceLastFootstep;
	UPROPERTY()
	float PreviousSpeed;
	UPROPERTY()
	FVector PreviousPosition;
	UPROPERTY()
	bool bPreviousIsGrounded;
	UPROPERTY()
	ACharacter* PlayerChar;
	UPROPERTY()
	bool bWasStartSoundPlayed;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	USoundBase* SelectFootStepSound();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	void PlayFootStepSound(FVector CurrentPosition, bool bIsCrouching, bool bIsSprinting, bool bIsStartingToMove,
		bool bIsEndingToMove, bool bIsStartingToJump, bool bIsLanding);

	void ReportFootstepNoise(float Volume);

};
