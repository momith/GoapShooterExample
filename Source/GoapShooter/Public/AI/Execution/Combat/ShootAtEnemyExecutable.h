#pragma once

#include "CoreMinimal.h"
#include "AI/Execution/ExecutableAction.h"
#include "ShootAtEnemyExecutable.generated.h"

/**
 * Executable action that makes the AI shoot at an enemy
 * Handles shooting logic and success/failure determination
 */
UCLASS()
class GOAPSHOOTER_API UShootAtEnemyExecutable : public UExecutableAction
{
	GENERATED_BODY()
	
public:
	UShootAtEnemyExecutable();
	
	// ExecutableAction interface
	virtual bool StartExecution() override;
	virtual void TickAction(float DeltaTime) override;
	virtual bool IsActionComplete() const override;
	virtual void AbortAction() override;
	// End of ExecutableAction interface
	
	/** Maximum sway angle in degrees when shooting */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float MaxShootSway;
	
	/** Time to wait before shooting */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float ShootInterval;
	
private:
	/** Perform the actual shooting */
	UFUNCTION()
	void ShootNow();
	
	/** Timer handle for shooting */
	FTimerHandle ShootTimerHandle;
	
	/** Current enemy to shoot at */
	UPROPERTY()
	class AGoapShooterCharacter* CurrentEnemy;
	
	/** Whether the action has started */
	bool bHasStarted;
	
	/** Whether the last shot killed the enemy */
	bool bEnemyKilled;
};
