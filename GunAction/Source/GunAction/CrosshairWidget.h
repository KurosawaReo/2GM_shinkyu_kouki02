// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "CrosshairWidget.generated.h"

/**
 * 
 */
UCLASS()
class GUNACTION_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
	

public :
public:
	virtual void NativeConstruct() override;

	// クロスヘアの色を変更する関数.
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetCrosshairColor(FColor NewColor);

	// ショット時の視覚効果.
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void OnShotEffect();

	// クロスヘアの透明度を設定.
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetCrosshairOpacity(float Opacity);

protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* CrosshairImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	class UTexture2D* CrosshairTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	FColor DefaultColor = FColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	FVector2D CrosshairSize = FVector2D(64.0f, 64.0f);

private:
	// ショット効果用タイマー.
	FTimerHandle ShotEffectTimerHandle;
	void ResetCrosshairColor();
};
