#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GoapShooterGameMode.generated.h"

/**
 * Basic GameMode with respawn mechanism for the bots.
 */
UCLASS(minimalapi)
class AGoapShooterGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AGoapShooterGameMode();
    
    virtual void BeginPlay() override;
    
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    int32 NumBotsToSpawn = 2;
    
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    bool bActivateGodModeForPlayer = true;

    /** Time between bot respawn checks (in seconds) */
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float BotRespawnCheckInterval = 8.0f;

    /** Whether to automatically respawn bots when they die */
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    bool bAutoRespawnBots = true;
    
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    TSubclassOf<class AGoapShooterAICharacter> AICharacterClass;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    TSubclassOf<class AGoapShooterAIController> AIControllerClass;

protected:

    /** Spawn an AI bot at a player start location */
    UFUNCTION(BlueprintCallable, Category = "AI")
    class AGoapShooterAICharacter* SpawnAIBot();

    /** Spawn multiple AI bots */
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SpawnAIBots(int32 NumBots);
    
    /** Find a random player start */
    AActor* FindRandomPlayerStart();

    /** Timer handle for bot respawn checking */
    FTimerHandle BotRespawnTimerHandle;

    /** Check if we need to respawn bots and do so if needed */
    UFUNCTION()
    void CheckAndRespawnBots();

    /** Get the current number of AI bots in the level */
    UFUNCTION()
    int32 GetCurrentBotCount() const;
};
