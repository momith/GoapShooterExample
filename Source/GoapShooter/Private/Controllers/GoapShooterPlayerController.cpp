#include "Controllers/GoapShooterPlayerController.h"
#include "Controllers/GoapShooterAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AI/GOAP/Interfaces/GoapShooterPlanningComponent.h"
#include "AI/GOAP/Debug/PerceptionDebugWidget.h"

AGoapShooterPlayerController::AGoapShooterPlayerController()
{
    //
}

void AGoapShooterPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 1);
        }
    }
    
    if (!GoapDebugWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create GOAP debug widget. Please ensure the widget class is set up."));
    }
}

void AGoapShooterPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        if (MovementDebugKeyAction && CombatDebugKeyAction && PerceptionDebugKeyAction)
        {
            EnhancedInputComponent->BindAction(MovementDebugKeyAction, ETriggerEvent::Triggered, this, &AGoapShooterPlayerController::ToggleMovementGoapDebugWidget);
            EnhancedInputComponent->BindAction(CombatDebugKeyAction, ETriggerEvent::Triggered, this, &AGoapShooterPlayerController::ToggleCombatGoapDebugWidget);
            EnhancedInputComponent->BindAction(PerceptionDebugKeyAction, ETriggerEvent::Triggered, this, &AGoapShooterPlayerController::TogglePerceptionDebugWidget);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to bind GOAP debug actions. Please ensure the actions are properly set up."));
        }
    }
}

void AGoapShooterPlayerController::TogglePerceptionDebugWidget()
{
    AGoapShooterAIController* AIController = FindClosestAIController();
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("No AI controller found to debug"));
        return;
    }

    if (!PerceptionDebugWidget) {
        PerceptionDebugWidget = CreateWidget<UPerceptionDebugWidget>(this, PerceptionDebugWidgetClass);
        PerceptionDebugWidget->SetAIController(AIController);
    }
    
    if (PerceptionDebugWidget->IsInViewport())
    {
        HideWidget(PerceptionDebugWidget);
    }
    else
    {
        ShowWidget(PerceptionDebugWidget);
    }
}

void AGoapShooterPlayerController::ToggleMovementGoapDebugWidget()
{
    ToggleGoapDebugWidget(true);
}

void AGoapShooterPlayerController::ToggleCombatGoapDebugWidget()
{
    ToggleGoapDebugWidget(false);
}

void AGoapShooterPlayerController::ToggleGoapDebugWidget(bool bDebugMovementGoap)
{
    UGoapDebugWidget* Widget = nullptr;
    UGoapDebugWidget** WidgetPtr = nullptr;
    
    if (bDebugMovementGoap)
    {
        Widget = MovementGoapDebugWidget;
        WidgetPtr = &MovementGoapDebugWidget;
    }
    else
    {
        Widget = CombatGoapDebugWidget;
        WidgetPtr = &CombatGoapDebugWidget;
    }
    
    if (Widget)
    {
        if (Widget->IsInViewport())
        {
            HideWidget(Widget);
        }
        else
        {
            ShowWidget(Widget);
        }
    }
    else if (GoapDebugWidgetClass)
    {
        // Create the widget if it doesn't exist yet
        *WidgetPtr = CreateWidget<UGoapDebugWidget>(this, GoapDebugWidgetClass);
        Widget = *WidgetPtr;

        AGoapShooterAIController* AIController = FindClosestAIController();
        if (!AIController)
        {
            UE_LOG(LogTemp, Warning, TEXT("No AI controller found to debug"));
            return;
        }

        if (bDebugMovementGoap)
        {
            Widget->SetDebugHelper(AIController->GetMovementPlanningComponent()->GoapDebugHelper);
        }
        else
        {
            Widget->SetDebugHelper(AIController->GetCombatPlanningComponent()->GoapDebugHelper);
        }

        ShowWidget(Widget);
    }
}

void AGoapShooterPlayerController::ShowWidget(UUserWidget* Widget)
{
    if (!Widget)
    {
        return;
    }
    
    Widget->AddToViewport();
}

void AGoapShooterPlayerController::HideWidget(UUserWidget* Widget)
{
    if (Widget && Widget->IsInViewport())
    {
        Widget->RemoveFromParent();
    }
}

AGoapShooterAIController* AGoapShooterPlayerController::FindClosestAIController()
{
    if (ClosestAIController) 
    {
        return ClosestAIController;
    }

    // Get all AI controllers
    TArray<AActor*> AIControllers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoapShooterAIController::StaticClass(), AIControllers);
    
    if (AIControllers.Num() == 0)
    {
        return nullptr;
    }
    
    // If there's only one AI controller, return it
    if (AIControllers.Num() == 1)
    {
        return Cast<AGoapShooterAIController>(AIControllers[0]);
    }
    
    // Find the closest AI controller to the player
    APawn* PlayerPawn = GetPawn();
    if (!PlayerPawn)
    {
        // If no player pawn, return the first AI controller
        return Cast<AGoapShooterAIController>(AIControllers[0]);
    }
    
    // Get player location
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Find the closest AI controller
    float ClosestDistanceSq = MAX_FLT;
    AGoapShooterAIController* ClosestController = nullptr;
    
    for (AActor* Actor : AIControllers)
    {
        AGoapShooterAIController* AIController = Cast<AGoapShooterAIController>(Actor);
        if (!AIController)
        {
            continue;
        }
        
        // Get the AI pawn
        APawn* AIPawn = AIController->GetPawn();
        if (!AIPawn)
        {
            continue;
        }
        
        // Calculate distance
        float DistanceSq = FVector::DistSquared(PlayerLocation, AIPawn->GetActorLocation());
        if (DistanceSq < ClosestDistanceSq)
        {
            ClosestDistanceSq = DistanceSq;
            ClosestController = AIController;
        }
    }
    
    ClosestAIController = ClosestController;

    return ClosestAIController;
}
