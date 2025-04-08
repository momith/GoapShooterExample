#include "Characters/CrosshairHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

void ACrosshairHUD::DrawHUD()
{
	Super::DrawHUD();
	DrawCrosshair();
}

void ACrosshairHUD::DrawCrosshair()
{
	if (!Canvas)
	{
		return;
	}

	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
	const float CrosshairSize = 8.0f;
	const float LineThickness = 2.0f;
	const FLinearColor CrosshairColor = FLinearColor::White;

	// Horizontal line
	DrawLine(
		Center.X - CrosshairSize, Center.Y,
		Center.X + CrosshairSize, Center.Y,
		CrosshairColor,
		LineThickness
	);

	// Vertical line
	DrawLine(
		Center.X, Center.Y - CrosshairSize,
		Center.X, Center.Y + CrosshairSize,
		CrosshairColor,
		LineThickness
	);
}
