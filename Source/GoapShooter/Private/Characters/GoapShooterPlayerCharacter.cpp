#include "Characters/GoapShooterPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GoapShooterGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"


AGoapShooterPlayerCharacter::AGoapShooterPlayerCharacter()
{  
    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
    FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
    FirstPersonCameraComponent->bUsePawnControlRotation = true;

    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
}

void AGoapShooterPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AGoapShooterPlayerCharacter::Shoot()
{
    float DamageAmount = 20.0f;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	int32 ViewportX, ViewportY;
	PC->GetViewportSize(ViewportX, ViewportY);

	const FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);

	FVector WorldLocation;
	FVector WorldDirection;

	if (PC->DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, WorldLocation, WorldDirection))
	{
		FVector TraceStart = WorldLocation;
		FVector TraceEnd = TraceStart + WorldDirection * 10000.0f; // 10,000 units ahead

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, Params))
		{
			// Apply damage
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor->ActorHasTag(FName("ToBePerceivedByAI")))
			{
				//UGameplayStatics::ApplyDamage(HitActor, DamageAmount, PC, this, UDamageType::StaticClass());

                UGameplayStatics::ApplyPointDamage(
                    HitActor,                          // Target actor
                    DamageAmount,                     // Damage amount
                    WorldDirection,                  // Hit direction
                    Hit,                       // Hit info
                    GetController(),                 // Instigator controller
                    this,                            // Damage causer
                    UDamageType::StaticClass()       // Damage type
                );

                // Draw debug line and impact point
                DrawDebugLine(GetWorld(), TraceStart, Hit.ImpactPoint, FColor::Green, false, 2.0f, 0, 1.0f);
                DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 8.0f, 12, FColor::Yellow, false, 2.0f);
			}
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Nothing hit!"));

                // If nothing hit, draw full line
                DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
            }
		}
		else
		{
			// If nothing hit, draw full line
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
		}
	}

    PlayShootSound();
}

void AGoapShooterPlayerCharacter::Die(AActor* DamageCauser)
{
    AGoapShooterGameMode* GameMode = Cast<AGoapShooterGameMode>(GetWorld()->GetAuthGameMode());
    if (GameMode && GameMode->bActivateGodModeForPlayer) {
        return;
    }
    Super::Die(DamageCauser);
}

void AGoapShooterPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Add Input Mapping Context for player characters
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void AGoapShooterPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jumping
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

        // Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGoapShooterPlayerCharacter::Move);

        // Looking
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGoapShooterPlayerCharacter::Look);

        // Shooting
        EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &AGoapShooterPlayerCharacter::Shoot);
    }
}

void AGoapShooterPlayerCharacter::Move(const FInputActionValue& Value)
{
    // input is a Vector2D
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // get forward vector
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        // get right vector 
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // add movement 
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AGoapShooterPlayerCharacter::Look(const FInputActionValue& Value)
{
    // input is a Vector2D
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // add yaw and pitch input to controller
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}