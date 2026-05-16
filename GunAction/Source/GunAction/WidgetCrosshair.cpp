/*
   - WidgetCrosshair -
   クロスヘアUI
*/
#include "WidgetCrosshair.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UWidgetCrosshair::NativeConstruct()
{
	Super::NativeConstruct();

	if (CrosshairSize == FVector2D::ZeroVector)
	{
		CrosshairSize = FVector2D(64.0f, 64.0f);
	}

	// クロスヘア画像を初期化.
	if (CrosshairImage && CrosshairTexture)
	{
		CrosshairImage->SetBrushFromTexture(CrosshairTexture);
		CrosshairImage->SetColorAndOpacity(DefaultColor);
		CrosshairImage->SetDesiredSizeOverride(CrosshairSize);
	}
}

void UWidgetCrosshair::SetCrosshairColor(FColor NewColor)
{
	if (CrosshairImage)
	{
		CrosshairImage->SetColorAndOpacity(NewColor);
	}
}

void UWidgetCrosshair::SetCrosshairOpacity(float Opacity)
{
	if (CrosshairImage)
	{
		FLinearColor Color = CrosshairImage->GetColorAndOpacity();
		Color.A = Opacity;
		CrosshairImage->SetColorAndOpacity(Color);
	}
}
void UWidgetCrosshair::ResetCrosshairColor()
{
	if (CrosshairImage)
	{
		CrosshairImage->SetColorAndOpacity(DefaultColor);
	}
}

/// <summary>
/// 射撃時に呼び出す.
/// </summary>
void UWidgetCrosshair::OnShotEffect()
{
	if (CrosshairImage == nullptr || GetWorld() == nullptr)
	{
		return;
	}

	//ショット時に赤色に変更.
	CrosshairImage->SetColorAndOpacity(FColor::Red);

	// 0.1秒後に元の色に戻す
	GetWorld()->GetTimerManager().SetTimer(
		ShotEffectTimerHandle,
		this,
		&UWidgetCrosshair::ResetCrosshairColor,
		0.1f,
		false
	);
}