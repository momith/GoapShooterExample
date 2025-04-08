#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GoapShooterGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerKillDelegate, APlayerState*, KillerPlayerState, APlayerState*, VictimPlayerState);

/**
 * Basic GameState with notifier mechanism for kills.
 */
UCLASS()
class GOAPSHOOTER_API AGoapShooterGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="WildWest")
	FPlayerKillDelegate OnPlayerKillDelegate;
};
