#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CrosshairHUD.generated.h"

UCLASS()
class GOAPSHOOTER_API ACrosshairHUD : public AHUD
{
    GENERATED_BODY()

protected:
	virtual void DrawHUD() override;

private:
	void DrawCrosshair();
};
