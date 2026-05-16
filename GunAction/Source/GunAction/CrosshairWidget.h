/*
   - CrosshairWidget -
   クロスヘアUI
*/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "CrosshairWidget.generated.h"

UCLASS()
class GUNACTION_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
	
//▼ ===== 変数 ===== ▼.
protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* CrosshairImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty")
	class UTexture2D* CrosshairTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty")
	FColor DefaultColor = FColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty")
	FVector2D CrosshairSize = FVector2D(64.0f, 64.0f);

//▼ ===== 関数 ===== ▼.
private:
	// ショット効果用タイマー.
	FTimerHandle ShotEffectTimerHandle;
	void ResetCrosshairColor();

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
};
