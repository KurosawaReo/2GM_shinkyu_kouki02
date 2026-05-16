/*
   - WidgetCrosshair -
   クロスヘアUI
*/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "WidgetCrosshair.generated.h"

UCLASS()
class GUNACTION_API UWidgetCrosshair : public UUserWidget
{
	GENERATED_BODY()
	
//▼ ===== 変数 ===== ▼.
protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* CrosshairImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty")
	class UTexture2D* CrosshairTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty")
	FLinearColor DefaultColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty")
	FVector2D CrosshairSize = FVector2D(64.0f, 64.0f);

//▼ ===== 関数 ===== ▼.
private:
	// ショット効果用タイマー.
	FTimerHandle ShotEffectTimerHandle;
	void ResetCrosshairColor();

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetCrosshairColor(FColor NewColor);
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetCrosshairOpacity(float Opacity);

	void OnShotEffect();
};
