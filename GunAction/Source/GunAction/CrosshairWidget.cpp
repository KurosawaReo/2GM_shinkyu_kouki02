// Fill out your copyright notice in the Description page of Project Settings.


#include "CrosshairWidget.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// デフォルト値を設定.
	if (DefaultColor == FColor::Black)
	{
		DefaultColor = FColor::White;
	}

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

void UCrosshairWidget::SetCrosshairColor(FColor NewColor)
{
	if (CrosshairImage)
	{
		CrosshairImage->SetColorAndOpacity(NewColor);
	}
}

void UCrosshairWidget::OnShotEffect()
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
		&UCrosshairWidget::ResetCrosshairColor,
		0.1f,
		false
	);
}

void UCrosshairWidget::SetCrosshairOpacity(float Opacity)
{
	if (CrosshairImage)
	{
		FLinearColor Color = CrosshairImage->GetColorAndOpacity();
		Color.A = Opacity;
		CrosshairImage->SetColorAndOpacity(Color);
	}
}
void UCrosshairWidget::ResetCrosshairColor()
{
	if (CrosshairImage)
	{
		CrosshairImage->SetColorAndOpacity(DefaultColor);
	}
}


