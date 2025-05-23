#include "Core/GoapShooterGameMode.h"
#include "Characters/GoapShooterAICharacter.h"
#include "Controllers/GoapShooterAIController.h"
#include "GameFramework/PlayerStart.h"
#include "GoapShooterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/CrosshairHUD.h"

AGoapShooterGameMode::AGoapShooterGameMode()
{
    AIControllerClass = AGoapShooterAIController::StaticClass();
    HUDClass = ACrosshairHUD::StaticClass();
    GameStateClass = AGoapShooterGameState::StaticClass();
}

void AGoapShooterGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Spawn initial bots
    SpawnAIBots(NumBotsToSpawn);
    
    // Set up timer for bot respawning if enabled
    if (bAutoRespawnBots)
    {
        GetWorldTimerManager().SetTimer(
            BotRespawnTimerHandle,
            this,
            &AGoapShooterGameMode::CheckAndRespawnBots,
            BotRespawnCheckInterval,
            true // Loop the timer
        );
    }
}

AGoapShooterAICharacter* AGoapShooterGameMode::SpawnAIBot()
{
    if (!AICharacterClass || !AIControllerClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("AGoapShooterGameMode: AI Character or Controller class not set!"));
        return nullptr;
    }

    // Find a spawn location
    AActor* StartSpot = FindRandomPlayerStart();
    if (!StartSpot)
    {
        UE_LOG(LogTemp, Warning, TEXT("AGoapShooterGameMode: Could not find spawn location for AI!"));
        return nullptr;
    }

    // Set spawn parameters
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Spawn the AI character
    AGoapShooterAICharacter* AICharacter = GetWorld()->SpawnActor<AGoapShooterAICharacter>(
        AICharacterClass,
        StartSpot->GetActorLocation(),
        StartSpot->GetActorRotation(),
        SpawnParams
    );

    if (AICharacter)
    {
        // Spawn and set the AI controller
        AGoapShooterAIController* AIController = GetWorld()->SpawnActor<AGoapShooterAIController>(
            AIControllerClass,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (AIController)
        {
            AIController->Possess(AICharacter);
        }
    }

    return AICharacter;
}

void AGoapShooterGameMode::SpawnAIBots(int32 NumBots)
{
    for (int32 i = 0; i < NumBots; ++i)
    {
        SpawnAIBot();
    }
}

AActor* AGoapShooterGameMode::FindRandomPlayerStart()
{
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

    if (PlayerStarts.Num() > 0)
    {
        // Pick a random player start
        int32 RandomIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
        return PlayerStarts[RandomIndex];
    }

    return nullptr;
}

void AGoapShooterGameMode::CheckAndRespawnBots()
{
    if (!bAutoRespawnBots)
    {
        return;
    }
    
    // Get current number of bots
    int32 CurrentBotCount = GetCurrentBotCount();
    
    // Calculate how many bots we need to spawn
    int32 BotsToSpawn = NumBotsToSpawn - CurrentBotCount;
    
    if (BotsToSpawn > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Respawning %d bots. Current count: %d, Target: %d"), 
            BotsToSpawn, CurrentBotCount, NumBotsToSpawn);
        
        // Spawn the required number of bots
        SpawnAIBots(BotsToSpawn);
    }
}

int32 AGoapShooterGameMode::GetCurrentBotCount() const
{
    int32 Count = 0;
    
    // Find all AI characters in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoapShooterAICharacter::StaticClass(), FoundActors);
    
    // Count only alive bots
    for (AActor* Actor : FoundActors)
    {
        AGoapShooterAICharacter* AIChar = Cast<AGoapShooterAICharacter>(Actor);
        if (AIChar && !AIChar->IsDead())
        {
            Count++;
        }
    }
    
    return Count;
}
