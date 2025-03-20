#include "Controllers/GoapShooterPlayerController.h"
#include "Controllers/GoapShooterAIControllerBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AGoapShooterPlayerController::AGoapShooterPlayerController()
{
    GoapDebugWidget = nullptr;
}

void AGoapShooterPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    // Set up enhanced input
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 1);
        }
    }
    
    // Create the debug widget if a class is specified
    if (GoapDebugWidgetClass)
    {
        GoapDebugWidget = CreateWidget<UGoapDebugWidget>(this, GoapDebugWidgetClass);
    }
}

void AGoapShooterPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    // Set up Enhanced Input component
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        // Bind the O key action
        if (OKeyAction)
        {
            EnhancedInputComponent->BindAction(OKeyAction, ETriggerEvent::Triggered, this, &AGoapShooterPlayerController::ToggleGoapDebugWidget);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to set up Enhanced Input component. Please ensure Enhanced Input is properly set up."));
    }
}

void AGoapShooterPlayerController::OnOKeyPressed()
{
    UE_LOG(LogTemp, Warning, TEXT("O key was pressed!"));
}

void AGoapShooterPlayerController::ToggleGoapDebugWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("Toggle GOAP debug widget!"));
    if (GoapDebugWidget)
    {
        if (GoapDebugWidget->IsInViewport())
        {
            HideGoapDebugWidget();
        }
        else
        {
            ShowGoapDebugWidget();
        }
    }
    else if (GoapDebugWidgetClass)
    {
        // Create the widget if it doesn't exist
        GoapDebugWidget = CreateWidget<UGoapDebugWidget>(this, GoapDebugWidgetClass);
        ShowGoapDebugWidget();
        UE_LOG(LogTemp, Log, TEXT("Created GOAP debug widget"));
    }
}

void AGoapShooterPlayerController::ShowGoapDebugWidget()
{
    if (!GoapDebugWidget)
    {
        return;
    }
    
    // Find the closest AI controller to debug
    AGoapShooterAIControllerBase* AIController = FindClosestAIController();
    if (AIController && AIController->DebugHelper)
    {
    // Set the debug helper and current action/goal
    GoapDebugWidget->SetDebugHelper(AIController->DebugHelper);

    
    // Add the widget to the viewport
    GoapDebugWidget->AddToViewport();
    UE_LOG(LogTemp, Log, TEXT("Showing GOAP debug widget"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No AI controller found to debug"));
    }
}

void AGoapShooterPlayerController::HideGoapDebugWidget()
{
    if (GoapDebugWidget && GoapDebugWidget->IsInViewport())
    {
        GoapDebugWidget->RemoveFromParent();
    }
}

AGoapShooterAIControllerBase* AGoapShooterPlayerController::FindClosestAIController()
{
    // Get all AI controllers
    TArray<AActor*> AIControllers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoapShooterAIControllerBase::StaticClass(), AIControllers);
    
    if (AIControllers.Num() == 0)
    {
        return nullptr;
    }
    
    // If there's only one AI controller, return it
    if (AIControllers.Num() == 1)
    {
        return Cast<AGoapShooterAIControllerBase>(AIControllers[0]);
    }
    
    // Find the closest AI controller to the player
    APawn* PlayerPawn = GetPawn();
    if (!PlayerPawn)
    {
        // If no player pawn, return the first AI controller
        return Cast<AGoapShooterAIControllerBase>(AIControllers[0]);
    }
    
    // Get player location
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Find the closest AI controller
    float ClosestDistanceSq = MAX_FLT;
    AGoapShooterAIControllerBase* ClosestController = nullptr;
    
    for (AActor* Actor : AIControllers)
    {
        AGoapShooterAIControllerBase* AIController = Cast<AGoapShooterAIControllerBase>(Actor);
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
    
    return ClosestController;
}
