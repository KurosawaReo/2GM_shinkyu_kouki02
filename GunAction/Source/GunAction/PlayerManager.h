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

class UCrosshairWidget;
class USpringArmComponent;
class UCameraComponent;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|UI")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|UI")
	UCrosshairWidget* CrosshairWidget;

	//カメラコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|Camera")
	class UCameraComponent* FollowCamera;

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

#pragma region "カメラ"
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FVector  GetCameraVector(FString dir) const;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	FVector  GetCameraLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	FRotator GetCameraRotation() const;

	//カメラ前方の目標地点を計算する関数.
//	UFUNCTION(BlueprintCallable, Category = "Bullet")
//	FVector GetTargetPos(float Distance) const;
#pragma endregion

#pragma region "UI"
	void InitializeUI();
#pragma endregion

#pragma region "射撃"
	void ShotBullet() override; //override
#pragma endregion
};
