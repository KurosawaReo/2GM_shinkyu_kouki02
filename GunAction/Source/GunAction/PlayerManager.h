/*
   - PlayerManager -
   共通のCharacterクラスから派生したプレイヤークラス.

   [クラス構成]
   CharacterBase
   └PlayerManager ←ここ
   └EnemyManager
*/
#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h" //親クラス.
#include "PlayerManager.generated.h"

/// <summary>
/// プレイヤークラス.
/// </summary>
UCLASS()
class GUNACTION_API APlayerManager : public ACharacterBase
{
	GENERATED_BODY()

//▼ ===== 変数 ===== ▼.
public:
	//クロスヘア.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Base|UI")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Base|UI")
	UCrosshairWidget* CrosshairWidget;

//▼ ===== 関数 ===== ▼.
protected:
	//コンストラクタ.
	APlayerManager();

#pragma region "ライフサイクル"
	//召喚した瞬間.
	void BeginPlay() override;
	//常に実行.
	void Tick(float DeltaTime) override;
#pragma endregion

#pragma region "入力処理"
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Input(UInputComponent* PlayerInputComponent);
#pragma endregion

#pragma region "移動"
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void StartSprint();
	void StopSprint();
#pragma endregion

#pragma region "UI"
	void InitializeUI();
#pragma endregion

#pragma region "射撃"
	void ShotBullet() override; //override
#pragma endregion
};
